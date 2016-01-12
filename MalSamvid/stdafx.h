// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// disable the warning: 4290. We will revisit.
#pragma warning( disable : 4290 )

#include "targetver.h"

#include <Windows.h>
#include <process.h> 
#include <stdio.h>
#include <tchar.h>


#include <iostream>
#include <string>
#include <vector>

#include <iomanip>
#include <cstdlib>
#include <fstream>
using namespace std;

#include <libconfig.h++>
using namespace libconfig;

// TODO: reference additional headers your program requires here
#include "base.h"
#include "ErrorCode.h"
#include "CmdLineProcessor.h"
#include "Log.h"
#include "SettingReader.h"
#include "listdir.h"
#include "AnalysisManager.h"
#include "vix.h"
#include "cvix.h"
#include "ErrorHandler.h"

#include <VirtualBox.h>
#include "VBoxManager.h"
#include "VBoxMachine.h"

// Dont know whether i should keep it here.
#define SAFE_RELEASE(x) \
    if (x) { \
        x->Release(); \
        x = NULL; \
    }


#define _VERBOSE