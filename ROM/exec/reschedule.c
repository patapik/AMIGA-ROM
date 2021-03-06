/*
    Copyright ? 1995-2017, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Enforce task rescheduling
    Lang: english
*/

#define DEBUG 0

#include <exec/execbase.h>
#include <aros/libcall.h>
#include <aros/atomic.h>
#include <hardware/intbits.h>

#include "chipset.h"
#include "exec_intern.h"

/*****************************************************************************

    NAME */
#include <proto/exec.h>

        AROS_LH0(void, Reschedule,

/*  SYNOPSIS */

/*  LOCATION */
        struct ExecBase *, SysBase, 8, Exec)

/*  FUNCTION
        Give up the CPU time to other tasks (if there are any).

    INPUTS
        None

    RESULT
        None

    NOTES
        This function was private in AmigaOS(tm) up to v3.1. There's no guarantee
        that it will continue to exist in other systems.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

******************************************************************************/
{
    AROS_LIBFUNC_INIT

    BOOL switchpending = FLAG_SCHEDSWITCH_ISSET;

    FLAG_SCHEDSWITCH_SET;       /* Set scheduling attention */

    if (TDNESTCOUNT_GET < 0)                 /* If task switching enabled */
    {
        if (IDNESTCOUNT_GET < 0)             /* And interrupts enabled */
        {
            D(bug("[Reschedule] Calling scheduler, KernelBase 0x%p\n", KernelBase);)
            KrnSchedule();                      /* Call scheduler */
        }
        else if (!switchpending)
        {
            /*
             * Interrupts are disabled and there was no pending switch before us.
             * Tag the software interrupt to be executed immediately after Enable().
             */
            CUSTOM_CAUSE(INTF_SOFTINT);
        }
    }

    AROS_LIBFUNC_EXIT
} /* Reschedule */
