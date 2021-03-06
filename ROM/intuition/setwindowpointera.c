/*
    Copyright ? 1995-2020, The AROS Development Team. All rights reserved.
    Copyright ? 2001-2003, The MorphOS Development Team. All Rights Reserved.
    $Id$
*/

#include <proto/graphics.h>
#include <proto/utility.h>
#include "intuition_intern.h"
#include "monitorclass_private.h"
#include <intuition/pointerclass.h>

/*****************************************************************************

    NAME */
#include <proto/intuition.h>

        AROS_LH2(void, SetWindowPointerA,

/*  SYNOPSIS */
        AROS_LHA(struct Window * , window , A0),
        AROS_LHA(struct TagItem *, taglist, A1),

/*  LOCATION */
        struct IntuitionBase *, IntuitionBase, 136, Intuition)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct Library *UtilityBase = GetPrivIBase(IntuitionBase)->UtilityBase;
    DEBUG_SETPOINTER(dprintf("SetWindowPointer: window 0x%p\n", window));

    if (window)
    {
        ULONG 	lock;
        Object *pointer = (Object *)GetTagData(WA_Pointer, 0, taglist);
        BOOL	busy = (GetTagData(WA_BusyPointer, FALSE, taglist) != 0) ? TRUE : FALSE;

        DEBUG_SETPOINTER(dprintf("SetWindowPointer: %spointer 0x%p\n",
                                (busy) ? "busy" : "",
                                pointer));

        lock = LockIBase(0);

        DEBUG_SETPOINTER(dprintf("SetWindowPointer: old %spointer 0x%p\n",
                                (IW(window)->busy) ? "busy " : "",
                                IW(window)->pointer));

        if (IW(window)->pointer != pointer || IW(window)->busy != busy)
        {
            Object *oldpointer = NULL;

            if (IW(window)->free_pointer)
                oldpointer = IW(window)->pointer;

            IW(window)->pointer = pointer;
            IW(window)->busy = busy;
            IW(window)->free_pointer = FALSE;
            window->Pointer = NULL;

            if (window->Flags & WFLG_WINDOWACTIVE)
            {
                struct IntScreen *scr = GetPrivScreen(window->WScreen);

                DEBUG_SETPOINTER(dprintf("SetWindowPointer: screen @ 0x%p\n", scr));

                if (GetTagData(WA_PointerDelay, FALSE, taglist) &&
                        IntuitionBase->ActiveScreen == &scr->Screen)
                {
                    DEBUG_SETPOINTER(dprintf("SetWindowPointer: delay change\n"));
                    GetPrivIBase(IntuitionBase)->PointerDelay = 5;
                }
                else
                {
                    struct msSetPointerShape pmsg;
                    pmsg.MethodID = MM_SetPointerShape;
                    pmsg.pointer = NULL;

                    if (pointer == NULL)
                        pointer = GetPrivIBase(IntuitionBase)->DefaultPointer;
                    if (busy)
                        pointer = GetPrivIBase(IntuitionBase)->BusyPointer;

                    GetAttr(POINTERA_SharedPointer, pointer, (IPTR *)&pmsg.pointer);

                    DEBUG_POINTER(dprintf("SetWindowPointer: scr 0x%lx pointer 0x%lx sprite 0x%lx\n",
                                          scr, pointer, pmsg.pointer->sprite));

                    if (DoMethodA(scr->IMonitorNode, &pmsg))
                    {
                        ObtainSharedPointer(pmsg.pointer, IntuitionBase);
                        ReleaseSharedPointer(scr->Pointer, IntuitionBase);
                        scr->Pointer = pmsg.pointer;
                        if (window)
                        {
                            window->XOffset = pmsg.pointer->xoffset;
                            window->YOffset = pmsg.pointer->yoffset;
                        }
                    }
                    else
                    {
                        DEBUG_POINTER(dprintf("SetWindowPointer: can't set pointer.\n"));
                    }
                }
            }

            DisposeObject(oldpointer);
        }

        UnlockIBase(lock);
    }

    AROS_LIBFUNC_EXIT
} /* SetWindowPointerA */
