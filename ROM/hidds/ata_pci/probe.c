/*
    Copyright ? 2004-2019, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Hardware detection routine
    Lang: English
*/

#include <aros/debug.h>
#include <proto/exec.h>

/* We want all other bases obtained from our base */
#define __NOLIBBASE__

#include <proto/bootloader.h>
#include <proto/oop.h>

#include <aros/asmcall.h>
#include <aros/bootloader.h>
#include <aros/symbolsets.h>
#include <asm/io.h>
#include <exec/lists.h>
#include <exec/rawfmt.h>
#include <hardware/ahci.h>
#include <hidd/bus.h>
#include <hidd/ata.h>
#include <hidd/storage.h>
#include <hidd/pci.h>
#include <oop/oop.h>

#include <string.h>

#include "bus_class.h"
#include "interface_pio.h"
#include "interface_dma.h"
#include "pci.h"

#define DSATA(x)

/*
 * Currently we support legacy ISA ports only on x86.
 * This can change only if someone ports AROS to PowerPC
 * retro-machine like PReP.
 */
#ifdef __i386__
#define SUPPORT_LEGACY
#endif
#ifdef __x86_64__
#define SUPPORT_LEGACY
#endif

#define NAME_BUFFER 128

#define RANGESIZE0 8
#define RANGESIZE1 4
#define DMASIZE    16

CONST_STRPTR ataPCIName = "ata_pci.hidd";
CONST_STRPTR ataPCIControllerName = "PCI Dual Channel IDE Controller";
#ifdef SUPPORT_LEGACY
CONST_STRPTR ataISAControllerName = "ISA IDE Controller";
#endif

/* static list of io/irqs that we can handle */
struct ata__legacybus 
{
    UWORD       lb_Port;
    UWORD       lb_Alt;
    UBYTE       lb_IRQ;
    UBYTE       lb_ControllerID;
    UBYTE       lb_Bus;
    const char *lb_Name;
};

static const struct ata__legacybus LegacyBuses[] = 
{
    {0x1f0, 0x3f4, 14, 0, 0, "ISA IDE0 primary channel"  },
    {0x170, 0x374, 15, 0, 1, "ISA IDE0 secondary channel"},
    {0x168, 0x36c, 10, 1, 0, "ISA IDE1 primary channel"  },
    {0x1e8, 0x3ec, 11, 1, 1, "ISA IDE1 secondary channel"},
    {    0,     0,  0, 0, 0, NULL                        }
};

#ifdef DO_SATA_HANDOFF

/* SATA handoff code needs timer */

static struct IORequest *OpenTimer(void)
{
    struct MsgPort *p = CreateMsgPort();

    if (NULL != p)
    {
        struct IORequest *io = CreateIORequest(p, sizeof(struct timerequest));

        if (NULL != io)
        {
            if (0 == OpenDevice("timer.device", UNIT_MICROHZ, io, 0))   
            {
                return io;
            }
            DeleteIORequest(io);
        }
        DeleteMsgPort(p);
    }

    return NULL;
}

static void CloseTimer(struct IORequest *tmr)
{
    if (NULL != tmr)
    {
        struct MsgPort *p = tmr->io_Message.mn_ReplyPort;

        CloseDevice(tmr);
        DeleteIORequest(tmr);
        DeleteMsgPort(p);
    }
}

static void WaitTO(struct IORequest* tmr, ULONG secs, ULONG micro)
{
    tmr->io_Command = TR_ADDREQUEST;
    ((struct timerequest*)tmr)->tr_time.tv_secs = secs;
    ((struct timerequest*)tmr)->tr_time.tv_micro = micro;

    DoIO(tmr);
}

#endif

/*
 * PCI BUS ENUMERATOR
 *   collect ALL ata/ide capable devices (including SATA and other) and
 *   spawn concurrent tasks.
 *
 * This function is growing too large. It will shorten drasticly once this whole mess gets converted into c++
 */

static
AROS_UFH3(void, ata_PCIEnumerator_h,
    AROS_UFHA(struct Hook *, hook,   A0),
    AROS_UFHA(OOP_Object *,  Device, A2),
    AROS_UFHA(APTR,          message,A1))
{
    AROS_USERFUNC_INIT

    struct atapciBase *base = hook->h_Data;
    OOP_Object *Driver;
    struct PCIDeviceRef *devRef;
    IPTR DMABase, DMASize, INTLine;
    IPTR IOBase, IOAlt, IOSize, AltSize, SubClass, Interface;
    int x;
    CONST_STRPTR owner;
    OOP_Object *ata_PCI = NULL;
    struct TagItem ata_tags[] =
    {
        {aHidd_Name             , (IPTR)ataPCIName              },
        {aHidd_HardwareName     , (IPTR)ataPCIControllerName	},
        {aHidd_Producer         , 0				},
#define ATA_TAG_VEND 2
        {aHidd_Product          , 0				},
#define ATA_TAG_PROD 3
        {TAG_DONE               , 0				}
    };

    /*
     * obtain more or less useful data
     */
    OOP_GetAttr(Device, aHidd_PCIDevice_Driver   , (IPTR *)&Driver);
    OOP_GetAttr(Device, aHidd_PCIDevice_VendorID , &ata_tags[ATA_TAG_VEND].ti_Data);
    OOP_GetAttr(Device, aHidd_PCIDevice_ProductID, &ata_tags[ATA_TAG_PROD].ti_Data);
    OOP_GetAttr(Device, aHidd_PCIDevice_SubClass , &SubClass);
    OOP_GetAttr(Device, aHidd_PCIDevice_Base4    , &DMABase);
    OOP_GetAttr(Device, aHidd_PCIDevice_Size4    , &DMASize);
    OOP_GetAttr(Device, aHidd_PCIDevice_Interface, &Interface);

    D(bug("[ATA:PCI] ata_PCIEnumerator_h: Found IDE device %04x:%04x\n", ata_tags[ATA_TAG_VEND].ti_Data, ata_tags[ATA_TAG_PROD].ti_Data));

    /* First check subclass */
    if ((SubClass == PCI_SUBCLASS_SCSI) || (SubClass == PCI_SUBCLASS_SAS))
    {
        D(bug("[ATA:PCI] Unsupported subclass %d\n", SubClass));
        return;
    }

    owner = HIDD_PCIDevice_Obtain(Device, base->lib.lib_Node.ln_Name);
    if (owner)
    {
        D(bug("[ATA:PCI] Already owned by %s\n", owner));
        return;
    }

    devRef = AllocMem(sizeof(struct PCIDeviceRef), MEMF_ANY);
    if (!devRef)
    {
        D(bug("[ATA:PCI] Failed to allocate reference structure\n"));
        return;
    }

    devRef->ref_Device = Device;
    devRef->ref_Count  = 0;

    /*
     * SATA controllers may need a special treatment before becoming usable.
     * The machine's firmware (EFI on Mac) may operate them in native AHCI mode
     * and not set up legacy mode by itself.
     * In this case we have to do it ourselves.
     * This code is based on incomplete ahci.device source code by DissyOfCRN.
     * CHECKME: In order to work on PPC it uses explicit little-endian I/O,
     * assuming AHCI register file is always little-endian. Is it correct?
     */
    if (SubClass == PCI_SUBCLASS_SATA)
    {
        APTR hba_phys = NULL;
        IPTR hba_size = 0;
        volatile struct ahci_hwhba *hwhba;
        ULONG ghc, cap;

        OOP_GetAttr(Device, aHidd_PCIDevice_Base5, (IPTR *)&hba_phys);
        OOP_GetAttr(Device, aHidd_PCIDevice_Size5, &hba_size);

        DSATA(bug("[ATA:PCI] Device %04x:%04x is a SATA device, HBA 0x%p, size 0x%p\n", ata_tags[ATA_TAG_VEND].ti_Data, ata_tags[ATA_TAG_PROD].ti_Data, hba_phys, hba_size));

        hwhba = HIDD_PCIDriver_MapPCI(Driver, hba_phys, hba_size);
        DSATA(bug("[ATA:PCI] Mapped at 0x%p\n", hwhba));

        if (!hwhba)
        {
            DSATA(bug("[ATA:PCI] Mapping failed, device will be ignored\n"));
            DeviceFree(devRef, base);
            return;
        }

        cap = mmio_inl_le(&hwhba->cap);
        ghc = mmio_inl_le(&hwhba->ghc);
        DSATA(bug("[ATA:PCI] Capabilities: 0x%08X, host control: 0x%08X\n", cap, ghc));

        /*
         * Some hardware may report GHC_AE to be zero, together with CAP_SAM set (indicating
         * that the device doesn't support legacy IDE registers). Seems to be spec violation
         * (the AHCI specification says that in this cases GHC_AE is read-only bit which is
         * hardwired to 1).
         * Attempting to drive such a hardware causes ata.device to freeze.
         * This effect has been observed on Marvel 9172 controller (and some other HW,
         * according to user reports, but nobody has ever provided a debug log).
         */
        if (cap & CAP_SAM)
        {
            DSATA(bug("[ATA:PCI] Legacy mode is not supported, device will be ignored\n"));

            HIDD_PCIDriver_UnmapPCI(Driver, hba_phys, hba_size);
            DeviceFree(devRef, base);
            return;
        }

        if (ghc & GHC_AE)
        {
            DSATA(bug("[ATA:PCI] AHCI enabled\n"));

            /*
             * This is ATA driver, not SATA driver, so i'd like to keep SATA-specific code
             * at a minimum. None of tests revealed a real need for BIOS handoff, no BIOS
             * was discovered to use controllers in SMI mode.
             * However, if on some machine we have problems, we can try
             * to #define this.
             */
#ifdef DO_SATA_HANDOFF
            ULONG version = mmio_inl_le(&hwhba->vs);
            ULONG cap2    = mmio_inl_le(&hwhba->cap2);

            DSATA(bug("[ATA:PCI] Version: 0x%08X, Cap2: 0x%08X\n", version, cap2));

            if ((version >= AHCI_VERSION_1_20) && (cap2 && CAP2_BOH))
            {
                ULONG bohc;

                DSATA(bug("[ATA:PCI] HBA supports BIOS/OS handoff\n"));

                bohc = mmio_inl_le(&hwhba->bohc);
                if (bohc && BOHC_BOS)
                {
                    struct IORequest *timereq;

                    DSATA(bug("[ATA:PCI] Device owned by BIOS, performing handoff\n"));

                    /*
                     * We need timer.device in order to perform delays.
                     * TODO: in ata_InitBus() it will be opened and closed again.
                     * This is not optimal, it could be opened and closed just once.
                     */
                   timereq = OpenTimer(base);
                   if (!timereq)
                   {
                        DSATA(bug("[ATA:PCI] Failed to open timer, can't perform handoff. Device will be ignored\n"));

                        HIDD_PCIDriver_UnmapPCI(Driver, hba_phys, hba_size);
                        DeviceFree(devRef, base);
                        return;
                   }

                    mmio_outl_le(bohc | BOHC_OOS, &hwhba->bohc);
                    /* Spin on BOHC_BOS bit FIXME: Possible dead lock. No maximum time given on AHCI1.3 specs... */
                    while (mmio_inl_le(&hwhba->bohc) & BOHC_BOS);

                    WaitTO(timereq, 0, 25000);
                    /* If after 25ms BOHC_BB bit is still set give bios a minimum of 2 seconds more time to run */

                    if (mmio_inl_le(&hwhba->bohc) & BOHC_BB)
                    {
                        DSATA(bug("[ATA:PCI] Delayed handoff, waiting...\n"));
                        ata_WaitTO(timereq, 2, 0);
                    }

                    DSATA(bug("[ATA:PCI] Handoff done\n"));
                    CloseTimer(timereq);
                }
            }
#endif
            /* This resets GHC_AE bit, disabling AHCI */
            mmio_outl_le(0, &hwhba->ghc);
        }

        HIDD_PCIDriver_UnmapPCI(Driver, (APTR)hwhba, hba_size);
    }

    ata_PCI = HW_AddDriver(base->storageRoot, base->ataClass, ata_tags);
    if (ata_PCI)
    {
        D(bug("[ATA:PCI] ata_PCIEnumerator_h: ATA HW Object @ 0x%p\n", ata_PCI));
        /*
         * we can have up to two buses assigned to this device
         */
        for (x = 0; devRef != NULL && x < MAX_DEVICEBUSES; x++)
        {
            BYTE basePri = ATABUSNODEPRI_PROBED;

            /*
             * obtain I/O bases and interrupt line
             */
            if ((Interface & (1 << (x << 1))) || SubClass != PCI_SUBCLASS_IDE)
            {
                switch (x)
                {
                case 0:
                    OOP_GetAttr(Device, aHidd_PCIDevice_Base0, &IOBase);
                    OOP_GetAttr(Device, aHidd_PCIDevice_Size0, &IOSize);
                    OOP_GetAttr(Device, aHidd_PCIDevice_Base1, &IOAlt);
                    OOP_GetAttr(Device, aHidd_PCIDevice_Size1, &AltSize);
                    break;

                case 1:
                    OOP_GetAttr(Device, aHidd_PCIDevice_Base2, &IOBase);
                    OOP_GetAttr(Device, aHidd_PCIDevice_Size2, &IOSize);
                    OOP_GetAttr(Device, aHidd_PCIDevice_Base3, &IOAlt);
                    OOP_GetAttr(Device, aHidd_PCIDevice_Size3, &AltSize);
                    break;
                }
                OOP_GetAttr(Device, aHidd_PCIDevice_INTLine, &INTLine);
            }
            else if (LegacyBuses[base->legacycount].lb_ControllerID == 0)
            {
                IPTR isa_io_base;

                OOP_GetAttr(Driver, aHidd_PCIDriver_IOBase, &isa_io_base);
                D(bug("[ATA:PCI] Device using Legacy-Bus IOPorts @ 0x%p\n", isa_io_base));

                IOBase   = LegacyBuses[base->legacycount].lb_Port + isa_io_base;
                IOAlt    = LegacyBuses[base->legacycount].lb_Alt  + isa_io_base;
                INTLine  = LegacyBuses[base->legacycount].lb_IRQ;
                basePri  = ATABUSNODEPRI_PROBEDLEGACY;
                IOSize   = RANGESIZE0;
                AltSize  = RANGESIZE1;

                base->legacycount++;
            }
            else
            {
                D(bug("[ATA:PCI] Legacy buses exhausted\n"));
                IOBase = 0;
            }

            if (IOBase != 0 && IOSize == RANGESIZE0 && AltSize == RANGESIZE1 &&
                (DMASize >= DMASIZE || DMABase == 0 || SubClass == PCI_SUBCLASS_IDE))
            {
                struct ata_ProbedBus *probedbus;
                STRPTR str[2];
                int len;

                D(bug("[ATA:PCI] ata_PCIEnumerator_h: Adding Bus %d - IRQ %d, IO: %x:%x, DMA: %x\n",
                      x, INTLine, IOBase, IOAlt, DMABase));

                OOP_GetAttr(Device, aHidd_PCIDevice_SubClassDesc, (IPTR *)&str[0]);
                str[1] = x ? "secondary" : "primary";
                len = 14 + strlen(str[0]) + strlen(str[1]);

                probedbus = AllocVec(sizeof(struct ata_ProbedBus) + len, MEMF_ANY);
                if (probedbus)
                {
                    IPTR dmaBase = DMABase ? DMABase + (x << 3) : 0;
                    STRPTR name = (char *)probedbus + sizeof(struct ata_ProbedBus);

                    RawDoFmt("PCI %s %s channel", (RAWARG)str, RAWFMTFUNC_STRING, name);

                    probedbus->atapb_Parent       = ata_PCI;
                    probedbus->atapb_Node.ln_Name = name;
                    probedbus->atapb_Node.ln_Type = basePri;
                    probedbus->atapb_Node.ln_Pri  = basePri - (base->ata__buscount++);
                    probedbus->atapb_Device       = devRef;
                    probedbus->atapb_Vendor       = ata_tags[ATA_TAG_VEND].ti_Data;
                    probedbus->atapb_Product      = ata_tags[ATA_TAG_PROD].ti_Data;
                    probedbus->atapb_BusNo        = x;
                    probedbus->atapb_IOBase       = IOBase;
                    probedbus->atapb_IOAlt        = IOAlt;
                    probedbus->atapb_INTLine      = INTLine;
                    probedbus->atapb_DMABase      = dmaBase;

                    devRef->ref_Count++;
                    Enqueue((struct List *)&base->probedbuses, &probedbus->atapb_Node);

                    OOP_SetAttrsTags(Device, aHidd_PCIDevice_isIO, TRUE,
                                             aHidd_PCIDevice_isMaster, DMABase != 0,
                                             TAG_DONE);
                }
            }

            if (!devRef->ref_Count)
            {
                DeviceFree(devRef, base);
                devRef = NULL;
            }
        }
    }
    AROS_USERFUNC_EXIT
}

static CONST_STRPTR pciInterfaceIDs[] =
{
    IID_Hidd_PCI,
    IID_Hidd_PCIDevice,
    IID_Hidd_PCIDriver,
    NULL
};

/* We need no attributes for IID_Hidd_PCI, only methods */
#define ATTR_OFFSET 1

static const struct TagItem Requirements[] =
{
    {tHidd_PCI_Class, PCI_CLASS_MASSSTORAGE},
    {TAG_DONE,        0x00                 }
};

/*
 * The manner in which this code is written can look a little bit overcomplicated.
 * However it's just experimental attempt to write hardware scan routine which
 * can be run more than once, and is hotplug-aware.
 * In future this may assist implementation of "Rescan devices" functionality
 * in SysExplorer (for example). This will provide a possibility to load and unload
 * device drivers on the fly.
 * For now it's just experiment... Don't pay much attention please. :)
 */
static int ata_bus_Detect(struct atapciBase *base)
{
    APTR BootLoaderBase;
    struct ata_ProbedBus *probedbus;
    BOOL scanpci    = TRUE;
#ifdef SUPPORT_LEGACY
    BOOL scanlegacy = TRUE;
    OOP_Object *ata_ISA = NULL;
    int ata_ISA_Ports = 0;
#endif

    D(bug("[ATA:PCI] %s()\n", __PRETTY_FUNCTION__));

    /* Prepare lists for probed/found ide buses */
    NEWLIST(&base->probedbuses);
    base->ata__buscount = 0;
    base->legacycount   = 0;

    /* Obtain command line parameters */
    BootLoaderBase = OpenResource("bootloader.resource");
    D(bug("[ATA:PCI] BootloaderBase = %p\n", BootLoaderBase));
    if (BootLoaderBase != NULL)
    {
        struct List *list;
        struct Node *node;

        list = (struct List *)GetBootInfo(BL_Args);
        if (list)
        {
            ForeachNode(list, node)
            {
                if (strncmp(node->ln_Name, "ATA=", 4) == 0)
                {
                    const char *cmdline = &node->ln_Name[4];

                    if (strstr(cmdline, "nopci"))
                    {
                        D(bug("[ATA:PCI] Disabling PCI device scan\n"));
                        scanpci = FALSE;
                    }
#ifdef SUPPORT_LEGACY
                    if (strstr(cmdline, "nolegacy"))
                    {
                        D(bug("[ATA:PCI] Disabling Legacy ports\n"));
                        scanlegacy = FALSE;
                    }
#endif
                    if (strstr(cmdline, "disable"))
                    {
                        D(bug("[ATA:PCI] Disabling all ATA devices\n"));
#ifdef SUPPORT_LEGACY
                        scanlegacy = FALSE;
#endif
                        scanpci = FALSE;

                    }
                }
            }
        }
    }

    D(bug("[ATA:PCI] ata_bus_Detect: Enumerating devices\n"));

    if (scanpci)
    {
	BOOL doPCIScan = TRUE;
        /*
         * Attempt to get PCI subsytem object.
         * If this fails, PCI isn't there. But it's not fatal for us.
         */
        OOP_Object *pci = OOP_NewObject(NULL, CLID_Hidd_PCI, NULL);

        if (pci)
        {
            struct Hook FindHook =
            {
                .h_Entry = (IPTR (*)())ata_PCIEnumerator_h,
                .h_Data  = base
            };

           /*
             * Obtain PCI attribute and method bases only once.
             */
            if (!base->PCIDeviceAttrBase)
            {
                if (OOP_ObtainAttrBasesArray(&base->PCIDeviceAttrBase, &pciInterfaceIDs[ATTR_OFFSET]))
		{
		    doPCIScan = FALSE;
		}
                if (OOP_ObtainMethodBasesArray(&base->PCIMethodBase, pciInterfaceIDs))
		{
		    doPCIScan = FALSE;
		}
            }

	    if (doPCIScan)
	    {
		D(bug("[ATA:PCI] ata_bus_Detect: Checking for supported PCI devices ..\n"));

 		HIDD_PCI_EnumDevices(pci, &FindHook, Requirements);
	    }
        }
    }

#ifdef SUPPORT_LEGACY
    if (scanlegacy)
    {
        struct TagItem ata_tags[] =
        {
            {aHidd_Name         , (IPTR)ataPCIName              },
            {aHidd_HardwareName , (IPTR)ataISAControllerName    },
            {TAG_DONE           , 0                             }
        };
        UBYTE n = base->legacycount;

        D(bug("[ATA:PCI] ata_bus_Detect: Detecting Legacy-Buses\n"));

        ata_ISA = HW_AddDriver(base->storageRoot, base->ataClass, ata_tags);
        if (ata_ISA)
        {
            D(bug("[ATA:PCI] ata_bus_Detect: Adding Remaining Legacy-Buses\n"));

            while (LegacyBuses[n].lb_Port)
            {
                probedbus = AllocVec(sizeof(struct ata_ProbedBus), MEMF_ANY);
                if (probedbus)
                {
                    probedbus->atapb_Parent = ata_ISA;
                    probedbus->atapb_Node.ln_Name = (STRPTR)LegacyBuses[n].lb_Name;
                    probedbus->atapb_Node.ln_Type = ATABUSNODEPRI_LEGACY;
                    probedbus->atapb_Node.ln_Pri  = ATABUSNODEPRI_LEGACY - (base->ata__buscount++);
                    probedbus->atapb_Device       = NULL;
                    probedbus->atapb_Vendor       = 0;
                    probedbus->atapb_Product      = 0;
                    probedbus->atapb_BusNo        = LegacyBuses[n].lb_Bus;
                    probedbus->atapb_IOBase       = LegacyBuses[n].lb_Port;
                    probedbus->atapb_IOAlt        = LegacyBuses[n].lb_Alt;
                    probedbus->atapb_INTLine      = LegacyBuses[n].lb_IRQ;
                    probedbus->atapb_DMABase      = 0;

                    D(bug("[ATA:PCI] ata_bus_Detect: Adding Legacy Bus - IO: %x:%x\n",
                          probedbus->atapb_IOBase, probedbus->atapb_IOAlt));

                    Enqueue((struct List *)&base->probedbuses, &probedbus->atapb_Node);
                }
                n++;
            }
        }
    }
#endif    

    D(bug("[ATA:PCI] ata_bus_Detect: Registering Probed Buses..\n"));

    while ((probedbus = (struct ata_ProbedBus *)RemHead((struct List *)&base->probedbuses)) != NULL)
    {
        struct TagItem attrs[] =
        {
            {aHidd_Name                 , (IPTR)ataPCIName                      },
            {aHidd_HardwareName         , (IPTR)probedbus->atapb_Node.ln_Name   },
            {aHidd_Producer             , probedbus->atapb_Vendor               },
            {aHidd_Product              , probedbus->atapb_Product              },
            {aHidd_DriverData           , (IPTR)probedbus                       },
            {aHidd_ATABus_PIODataSize   , sizeof(struct pio_data)               },
            {aHidd_ATABus_BusVectors    , (IPTR)bus_FuncTable                   },
            {aHidd_ATABus_PIOVectors    , (IPTR)pio_FuncTable                   },
            {aHidd_ATABus_DMADataSize   , sizeof(struct dma_data)               },
            {aHidd_ATABus_DMAVectors    , (IPTR)dma_FuncTable                   },
            /*
             * Legacy ISA controllers have no other way to detect their
             * presence. Do not confuse the user with phantom devices.
             */
            {aHidd_Bus_KeepEmpty     , probedbus->atapb_Node.ln_Type == ATABUSNODEPRI_LEGACY
                                        ? FALSE : TRUE                          },
            {TAG_DONE                   , 0                                     }
        };
        OOP_Object *bus;

        /*
         * We use this field as ownership indicator.
         * The trick is that HW_AddDriver() fails if either object creation fails
         * or subsystem-side setup fails. In the latter case our object will be
         * disposed.
         * We need to know whether OOP_DisposeObject() or we should deallocate
         * this structure on failure.
         */
        probedbus->atapb_Node.ln_Succ = NULL;

        D(bug("[ATA:PCI] ata_bus_Detect: Attaching Instance of 0x%p to Controller @ 0x%p\n", base->busClass, probedbus->atapb_Parent));

        bus = HIDD_StorageController_AddBus(probedbus->atapb_Parent, base->busClass, attrs);
        if (!bus)
        {
            D(bug("[ATA:PCI] Failed to create object for device %04X:%04X - IRQ %d, IO: %x:%x, DMA: %x\n",
                  probedbus->atapb_Vendor, probedbus->atapb_Product, probedbus->atapb_INTLine,
                  probedbus->atapb_IOBase, probedbus->atapb_IOAlt, probedbus->atapb_DMABase));

            /*
             * Free the structure only upon object creation failure!
             * In case of success it becomes owned by the driver object!
             */
            if (!probedbus->atapb_Node.ln_Succ)
            {
                DeviceUnref(probedbus->atapb_Device, base);
                FreeVec(probedbus);
            }
        }
#ifdef SUPPORT_LEGACY
	else if ((ata_ISA) && (probedbus->atapb_Parent == ata_ISA))
		ata_ISA_Ports++;
#endif
    }

#ifdef SUPPORT_LEGACY
    if ((ata_ISA) && (ata_ISA_Ports == 0))
    {
	    D(bug("[ATA:PCI] ata_bus_Detect: Disposing Unused ISA controller object\n");)
	    HW_RemoveDriver(base->storageRoot, ata_ISA);
    }
#endif

    D(bug("[ATA:PCI] ata_bus_Detect: Finished..\n");)

    return TRUE;
}

ADD2INITLIB(ata_bus_Detect, 30)
