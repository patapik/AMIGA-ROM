/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Exec functions declared as extern
    Lang: english
*/

extern void AROS_SLIB_ENTRY(Supervisor,Exec,5)();
extern void AROS_SLIB_ENTRY(ExitIntr,Exec,6)();
extern void AROS_SLIB_ENTRY(Schedule,Exec,7)();
extern void AROS_SLIB_ENTRY(Reschedule,Exec,8)();
extern void AROS_SLIB_ENTRY(Switch,Exec,9)();
extern void AROS_SLIB_ENTRY(Dispatch,Exec,10)();
extern void AROS_SLIB_ENTRY(Exception,Exec,11)();
extern void AROS_SLIB_ENTRY(InitCode,Exec,12)();
extern void AROS_SLIB_ENTRY(InitStruct,Exec,13)();
extern void AROS_SLIB_ENTRY(MakeLibrary,Exec,14)();
extern void AROS_SLIB_ENTRY(MakeFunctions,Exec,15)();
extern void AROS_SLIB_ENTRY(FindResident,Exec,16)();
extern void AROS_SLIB_ENTRY(InitResident,Exec,17)();
extern void AROS_SLIB_ENTRY(Alert,Exec,18)();
extern void AROS_SLIB_ENTRY(Debug,Exec,19)();
extern void AROS_SLIB_ENTRY(Disable,Exec,20)();
extern void AROS_SLIB_ENTRY(Enable,Exec,21)();
extern void AROS_SLIB_ENTRY(Forbid,Exec,22)();
extern void AROS_SLIB_ENTRY(Permit,Exec,23)();
extern void AROS_SLIB_ENTRY(SetSR,Exec,24)();
extern void AROS_SLIB_ENTRY(SuperState,Exec,25)();
extern void AROS_SLIB_ENTRY(UserState,Exec,26)();
extern void AROS_SLIB_ENTRY(SetIntVector,Exec,27)();
extern void AROS_SLIB_ENTRY(AddIntServer,Exec,28)();
extern void AROS_SLIB_ENTRY(RemIntServer,Exec,29)();
extern void AROS_SLIB_ENTRY(Cause,Exec,30)();
extern void AROS_SLIB_ENTRY(Allocate,Exec,31)();
extern void AROS_SLIB_ENTRY(Deallocate,Exec,32)();
extern void AROS_SLIB_ENTRY(AllocMem,Exec,33)();
extern void AROS_SLIB_ENTRY(AllocAbs,Exec,34)();
extern void AROS_SLIB_ENTRY(FreeMem,Exec,35)();
extern void AROS_SLIB_ENTRY(AvailMem,Exec,36)();
extern void AROS_SLIB_ENTRY(AllocEntry,Exec,37)();
extern void AROS_SLIB_ENTRY(FreeEntry,Exec,38)();
extern void AROS_SLIB_ENTRY(Insert,Exec,39)();
extern void AROS_SLIB_ENTRY(AddHead,Exec,40)();
extern void AROS_SLIB_ENTRY(AddTail,Exec,41)();
extern void AROS_SLIB_ENTRY(Remove,Exec,42)();
extern void AROS_SLIB_ENTRY(RemHead,Exec,43)();
extern void AROS_SLIB_ENTRY(RemTail,Exec,44)();
extern void AROS_SLIB_ENTRY(Enqueue,Exec,45)();
extern void AROS_SLIB_ENTRY(FindName,Exec,46)();
extern void AROS_SLIB_ENTRY(AddTask,Exec,47)();
extern void AROS_SLIB_ENTRY(RemTask,Exec,48)();
extern void AROS_SLIB_ENTRY(FindTask,Exec,49)();
extern void AROS_SLIB_ENTRY(SetTaskPri,Exec,50)();
extern void AROS_SLIB_ENTRY(SetSignal,Exec,51)();
extern void AROS_SLIB_ENTRY(SetExcept,Exec,52)();
extern void AROS_SLIB_ENTRY(Wait,Exec,53)();
extern void AROS_SLIB_ENTRY(Signal,Exec,54)();
extern void AROS_SLIB_ENTRY(AllocSignal,Exec,55)();
extern void AROS_SLIB_ENTRY(FreeSignal,Exec,56)();
extern void AROS_SLIB_ENTRY(AllocTrap,Exec,57)();
extern void AROS_SLIB_ENTRY(FreeTrap,Exec,58)();
extern void AROS_SLIB_ENTRY(AddPort,Exec,59)();
extern void AROS_SLIB_ENTRY(RemPort,Exec,60)();
extern void AROS_SLIB_ENTRY(PutMsg,Exec,61)();
extern void AROS_SLIB_ENTRY(GetMsg,Exec,62)();
extern void AROS_SLIB_ENTRY(ReplyMsg,Exec,63)();
extern void AROS_SLIB_ENTRY(WaitPort,Exec,64)();
extern void AROS_SLIB_ENTRY(FindPort,Exec,65)();
extern void AROS_SLIB_ENTRY(AddLibrary,Exec,66)();
extern void AROS_SLIB_ENTRY(RemLibrary,Exec,67)();
extern void AROS_SLIB_ENTRY(OldOpenLibrary,Exec,68)();
extern void AROS_SLIB_ENTRY(CloseLibrary,Exec,69)();
extern void AROS_SLIB_ENTRY(SetFunction,Exec,70)();
extern void AROS_SLIB_ENTRY(SumLibrary,Exec,71)();
extern void AROS_SLIB_ENTRY(AddDevice,Exec,72)();
extern void AROS_SLIB_ENTRY(RemDevice,Exec,73)();
extern void AROS_SLIB_ENTRY(OpenDevice,Exec,74)();
extern void AROS_SLIB_ENTRY(CloseDevice,Exec,75)();
extern void AROS_SLIB_ENTRY(DoIO,Exec,76)();
extern void AROS_SLIB_ENTRY(SendIO,Exec,77)();
extern void AROS_SLIB_ENTRY(CheckIO,Exec,78)();
extern void AROS_SLIB_ENTRY(WaitIO,Exec,79)();
extern void AROS_SLIB_ENTRY(AbortIO,Exec,80)();
extern void AROS_SLIB_ENTRY(AddResource,Exec,81)();
extern void AROS_SLIB_ENTRY(RemResource,Exec,82)();
extern void AROS_SLIB_ENTRY(OpenResource,Exec,83)();
extern void AROS_SLIB_ENTRY(RawIOInit,Exec,84)();
extern void AROS_SLIB_ENTRY(RawMayGetChar,Exec,85)();
extern void AROS_SLIB_ENTRY(RawPutChar,Exec,86)();
extern void AROS_SLIB_ENTRY(RawDoFmt,Exec,87)();
extern void AROS_SLIB_ENTRY(GetCC,Exec,88)();
extern void AROS_SLIB_ENTRY(TypeOfMem,Exec,89)();
extern void AROS_SLIB_ENTRY(Procure,Exec,90)();
extern void AROS_SLIB_ENTRY(Vacate,Exec,91)();
extern void AROS_SLIB_ENTRY(OpenLibrary,Exec,92)();
extern void AROS_SLIB_ENTRY(InitSemaphore,Exec,93)();
extern void AROS_SLIB_ENTRY(_ObtainSemaphore,Exec,94)();
extern void AROS_SLIB_ENTRY(_ReleaseSemaphore,Exec,95)();
extern void AROS_SLIB_ENTRY(AttemptSemaphore,Exec,96)();
extern void AROS_SLIB_ENTRY(ObtainSemaphoreList,Exec,97)();
extern void AROS_SLIB_ENTRY(ReleaseSemaphoreList,Exec,98)();
extern void AROS_SLIB_ENTRY(FindSemaphore,Exec,99)();
extern void AROS_SLIB_ENTRY(AddSemaphore,Exec,100)();
extern void AROS_SLIB_ENTRY(RemSemaphore,Exec,101)();
extern void AROS_SLIB_ENTRY(SumKickData,Exec,102)();
extern void AROS_SLIB_ENTRY(AddMemList,Exec,103)();
extern void AROS_SLIB_ENTRY(CopyMem,Exec,104)();
extern void AROS_SLIB_ENTRY(CopyMemQuick,Exec,105)();
extern void AROS_SLIB_ENTRY(CacheClearU,Exec,106)();
extern void AROS_SLIB_ENTRY(CacheClearE,Exec,107)();
extern void AROS_SLIB_ENTRY(CacheControl,Exec,108)();
extern void AROS_SLIB_ENTRY(CreateIORequest,Exec,109)();
extern void AROS_SLIB_ENTRY(DeleteIORequest,Exec,110)();
extern void AROS_SLIB_ENTRY(CreateMsgPort,Exec,111)();
extern void AROS_SLIB_ENTRY(DeleteMsgPort,Exec,112)();
extern void AROS_SLIB_ENTRY(_ObtainSemaphoreShared,Exec,113)();
extern void AROS_SLIB_ENTRY(AllocVec,Exec,114)();
extern void AROS_SLIB_ENTRY(FreeVec,Exec,115)();
extern void AROS_SLIB_ENTRY(CreatePool,Exec,116)();
extern void AROS_SLIB_ENTRY(DeletePool,Exec,117)();
extern void AROS_SLIB_ENTRY(AllocPooled,Exec,118)();
extern void AROS_SLIB_ENTRY(FreePooled,Exec,119)();
extern void AROS_SLIB_ENTRY(AttemptSemaphoreShared,Exec,120)();
extern void AROS_SLIB_ENTRY(ColdReboot,Exec,121)();
extern void AROS_SLIB_ENTRY(StackSwap,Exec,122)();
extern void AROS_SLIB_ENTRY(ChildFree,Exec,123)();
extern void AROS_SLIB_ENTRY(ChildOrphan,Exec,124)();
extern void AROS_SLIB_ENTRY(ChildStatus,Exec,125)();
extern void AROS_SLIB_ENTRY(ChildWait,Exec,126)();
extern void AROS_SLIB_ENTRY(CachePreDMA,Exec,127)();
extern void AROS_SLIB_ENTRY(CachePostDMA,Exec,128)();
extern void AROS_SLIB_ENTRY(AddMemHandler,Exec,129)();
extern void AROS_SLIB_ENTRY(RemMemHandler,Exec,130)();
extern void AROS_SLIB_ENTRY(ObtainQuickVector,Exec,131)();
extern void AROS_SLIB_ENTRY(TaggedOpenLibrary,Exec,135)();
extern void AROS_SLIB_ENTRY(AllocVecPooled,Exec,149)();
extern void AROS_SLIB_ENTRY(FreeVecPooled,Exec,150)();