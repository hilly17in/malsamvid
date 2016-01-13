# include "passthrough.h"

unsigned int uConnections = 0;

NTSTATUS
MAFilterConnect(
      PFLT_PORT ClientPort,
      PVOID ServerPortCookie,
      PVOID ConnectionContext,
      ULONG SizeOfContext,
      PVOID *ConnectionPortCookie)
{
	int *Context ;
	int connone = 1;
	int conntwo = 2;

	if (ConnectionContext == NULL)
		return STATUS_SUCCESS;

	PAGED_CODE() ;

	UNREFERENCED_PARAMETER(ServerPortCookie) ;
	UNREFERENCED_PARAMETER(SizeOfContext) ;
	UNREFERENCED_PARAMETER(ConnectionPortCookie) ;

	Context = (int *)ConnectionContext;

	if(*Context == 1)
	{
		ConnectionPortCookie = (PVOID) &connone ;
		gClientPort = ClientPort ;
	}
	else if (*Context == 2)
	{
		ConnectionPortCookie = (PVOID) &conntwo ;
		gRegPort = ClientPort ;
	}
	

	return STATUS_SUCCESS ;
} // PECFilterConnect



VOID
MAFilterDisconnect(
      PVOID ConnectionCookie)
{
	int *Cookie;

    PAGED_CODE();


	if(ConnectionCookie == NULL)
		return ;

	Cookie = (int *)ConnectionCookie ;
    
    //
    //  Close our handle
    //
	if(*Cookie == 1)
	   FltCloseClientPort( gFilterHandle, &gClientPort );
	else if(*Cookie == 2)
		FltCloseClientPort( gFilterHandle, &gRegPort);
} // PECFilterDisconnect


NTSTATUS
MAFilterMessage(
	  PVOID PortCookie,
      PVOID InputBuffer,
      ULONG InputBufferLength,
      PVOID OutputBuffer,
      ULONG OutputBufferLength,
      PULONG ReturnOutputBufferLength)
{
    PAGED_CODE();

	UNREFERENCED_PARAMETER(PortCookie) ;
    UNREFERENCED_PARAMETER(InputBuffer) ;
    UNREFERENCED_PARAMETER(InputBufferLength) ;
    UNREFERENCED_PARAMETER(OutputBuffer) ;
    UNREFERENCED_PARAMETER(OutputBufferLength) ;
    UNREFERENCED_PARAMETER(ReturnOutputBufferLength) ;
    
	return STATUS_SUCCESS ;
} // PECFilterMessage


NTSTATUS
SendMessageToUserMode(
 MAMessageType			Type, 
 PNotifyProcessData		pData, 
 USHORT					uOperationIndex, 
 unsigned long			code,
 PUNICODE_STRING		pFilePath
 ) 
{
	Buffer_ForUserMode	buffer;
	NTSTATUS			status ;
	LARGE_INTEGER		timeout;

	timeout.QuadPart		= -(LONGLONG) 1;
	

	switch(Type)
	{
	case NotifyProcess:

		buffer.Type = Type ;
		RtlCopyMemory(&buffer.NPData, pData, sizeof(NotifyProcessData)) ;
		
		break ;

	case NotifyBehavior:
	
		buffer.Type = Type ;
		buffer.NBData.Pid = gPid ;
		buffer.NBData.uOperationIndex = uOperationIndex ;

		//zero the contents of memory to remove all the garbage
		RtlZeroMemory( buffer.NBData.FilePath, MAX_IMAGE_PATH * sizeof(WCHAR) );

		if( pFilePath != NULL )
			RtlCopyMemory( buffer.NBData.FilePath, pFilePath->Buffer, pFilePath->Length );
	
		break ;

	} // switch


	status = FltSendMessage(gFilterHandle, 
						&gClientPort, 
					    &buffer, sizeof(buffer), 
						NULL, 0, &timeout) ;

	if(status != STATUS_SUCCESS)
		DbgPrint("fltSendMessage gClientPort error %x \n", status);
	
	return status ;
} // SendMessageToUserMode


VOID
SendRegEventToUserMode
(
 PREGISTRY_EVENT	pRegEvent
 )
{
	NTSTATUS			status; 
	LARGE_INTEGER		timeout;
	timeout.QuadPart	= -(LONGLONG) 1;

	status = FltSendMessage(gFilterHandle,
					&gRegPort,
					pRegEvent, sizeof(REGISTRY_EVENT),
					NULL, 0, &timeout);

	if(status != STATUS_SUCCESS)
		DbgPrint("fltSendMessage gRegPort error %x \n", status);

	return ;
}