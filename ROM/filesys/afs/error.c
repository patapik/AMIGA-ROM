/*
    Copyright ? 1995-2015, The AROS Development Team. All rights reserved.
    $Id$
*/

/*
 * -date------ -name------------------- -description-----------------------------
 * 02-jan-2008 [Tomasz Wiszkowski]      added disk check option for broken disks
 * 04-jan-2008 [Tomasz Wiszkowski]      corrected tabulation
 */

#include <proto/intuition.h>
#include <aros/debug.h>
#include <exec/rawfmt.h>
#include <intuition/intuition.h>

#include "os.h"
#include "error.h"
#include "errstrings.h"
#include "baseredef.h"


/*
 * displays requester on screen or puts text to the debug buffer
 */
LONG showPtrArgsText(struct AFSBase *afsbase, const char *string, enum showReqType type, RAWARG args) 
{
	LONG answer = 0;
	char* options[] =
	{
		"Cancel",
		"Retry|Cancel",
		"Check disk|Cancel",
		"Continue|Cancel",
		"Continue",
		NULL
	};
	struct EasyStruct es={sizeof (struct EasyStruct),0,"AFFS",0,options[type]};
	struct IntuitionBase *IntuitionBase;

	IntuitionBase = (APTR)TaggedOpenLibrary(TAGGEDOPEN_INTUITION);
	if (IntuitionBase != NULL)
	{
	    es.es_TextFormat=string;

	    if (IntuitionBase->FirstScreen != NULL)
	    {
		answer = EasyRequestArgs(NULL,&es,NULL,args);
	    }
	    CloseLibrary((struct Library *)IntuitionBase);
	}
	else
	{
	    /* We use serial for error printing when gfx.hidd is not initialized */
	    RawDoFmt(string, args, RAWFMTFUNC_SERIAL, NULL);
	    RawPutChar('\n');
	}

	return answer;
}

LONG showError(struct AFSBase *afsbase, ULONG error, ...)
{
    LONG ret;

    if (error == ERR_ALREADY_PRINTED)
        ret = 0;
    else if (error >= ERR_UNKNOWN)
        ret = showPtrArgsText(afsbase, texts[ERR_UNKNOWN].text, texts[ERR_UNKNOWN].type, (RAWARG)&error);
    else {
        AROS_SLOWSTACKFORMAT_PRE_USING(error, texts[error].text);
        ret = showPtrArgsText(afsbase, texts[error].text, texts[error].type, AROS_SLOWSTACKFORMAT_ARG(error));
        AROS_SLOWSTACKFORMAT_POST(error);
    }

    return ret;
}


/* vim: set ts=3 noet fdm=marker fmr={,}: */
