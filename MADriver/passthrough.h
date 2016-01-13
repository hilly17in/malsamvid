# pragma once

//Only one of the following two should be enabled

# include <fltKernel.h>
# include <ntstrsafe.h>
# include <dontuse.h>
# include <suppress.h>
# include <ntdddisk.h>

# pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")


# include "umkm_com.h"
# include "comport.h"
# include "instance.h"
# include "MAFilter.h"
# include "dispatch.h"
# include "irpcallbacks.h"
# include "debug.h"
# include "memmgmt.h"
# include "processnotify.h"
# include "regcallback.h"
# include "globals.h"

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PtUnload)
#pragma alloc_text(PAGE, PtInstanceQueryTeardown)
#pragma alloc_text(PAGE, PtInstanceSetup)
#pragma alloc_text(PAGE, PtInstanceTeardownStart)
#pragma alloc_text(PAGE, PtInstanceTeardownComplete)
#endif
