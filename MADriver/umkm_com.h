# pragma once

#define MAX_IMAGE_PATH		260

typedef enum _Operation_Index
{
	CreateNewFile		= 1,
	OpenExistFile		= 2,
	ReadingFile			= 3,
	WritingFile			= 4,
	SendsIoctl			= 5,
	ChangesPerm			= 6,
	DeletesFile			= 7,
	SetTimeStamp		= 8,
	CreateRegKey		= 9,
	SetValueKey			= 10,
	DeleteKey			= 11,
	OpenRegKey			= 12,
	EnumRegKey			= 13,
	RegEnumValueKey		= 14,
	RegQueryKey			= 15,
	RegQueryValueKey	= 16
} OperationIndex;

typedef enum _MAMessage_Type_
{
	NotifyProcess ,
	NotifyBehavior
} MAMessageType, *PMAMessageTypes ;

typedef struct _NotifyProcessData_
{
	unsigned int Pid ;
	unsigned int ParentId ;
	BOOLEAN	Create ;
} NotifyProcessData, *PNotifyProcessData;

typedef struct _NotifyBehaviorData_
{
	unsigned int		Pid ;
	OperationIndex		uOperationIndex;
	WCHAR				FilePath[MAX_IMAGE_PATH];
} NotifyBehaviorData, *PNotifyBehaviorData;

typedef struct _Buffer_ForUserMode_
{
	MAMessageType	Type ;
	union {
		NotifyProcessData	NPData ;
		NotifyBehaviorData	NBData ;
	} ;
} Buffer_ForUserMode, *PBuffer_ForUserMode;

typedef struct _RecvdBuffer_
{
	FILTER_MESSAGE_HEADER	header ;
	Buffer_ForUserMode		buffer ;
	HANDLE					handle ;
} RecvdBuffer, *PRecvdBuffer ;

typedef struct _REGISTRY_EVENT
{
	OperationIndex	OpIndex;
	WCHAR			wValueName[255];
	WCHAR			wRegPath[1024];
} REGISTRY_EVENT, *PREGISTRY_EVENT;

typedef struct _RegBuffer_
{
	FILTER_MESSAGE_HEADER	header;
	REGISTRY_EVENT			RegEvent;
	HANDLE					handle;
} RegBuffer, *PRegBuffer;

//
//  Name of PEC Filter's Symbolic Link
//
#define MAFILTER_SYMLINK_NAME				  L"\\??\\MAFilterSymLink"

#define MALANALYZE_FILTER_DEVICE	0x8100

//
//  Name of PEC Filter's communication server port
//
#define MAFILTER_PORT_NAME                   L"\\MAFilterPort"

#define IOCTL_SEND_AND_INITIALIZE		CTL_CODE(MALANALYZE_FILTER_DEVICE, \
										0x802, \
										METHOD_BUFFERED, \
										0)

#define IOCTL_TERMINATE_PROCESSID		CTL_CODE(MALANALYZE_FILTER_DEVICE, \
										0x803, \
										METHOD_BUFFERED, \
										0)

#define IOCTL_SEND_SERVICE_PID			CTL_CODE(MALANALYZE_FILTER_DEVICE, \
										0x804, \
										METHOD_BUFFERED, \
										0)