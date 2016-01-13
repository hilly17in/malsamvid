# pragma once

NTSTATUS
MAFilterConnect(
      PFLT_PORT ClientPort,
      PVOID ServerPortCookie,
      PVOID ConnectionContext,
      ULONG SizeOfContext,
      PVOID *ConnectionPortCookie
      );


VOID
MAFilterDisconnect(
      PVOID ConnectionCookie
      );


NTSTATUS
MAFilterMessage(
	  PVOID PortCookie,
      PVOID InputBuffer,
      ULONG InputBufferLength,
      PVOID OutputBuffer,
      ULONG OutputBufferLength,
      PULONG ReturnOutputBufferLength) ;

NTSTATUS SendMessageToUserMode(
 MAMessageType			Type, 
 PNotifyProcessData		pData, 
 USHORT					uBitIndex, 
 unsigned long			code,
 PUNICODE_STRING		pBuffer
 ) ;

VOID
SendRegEventToUserMode
(
 PREGISTRY_EVENT	pRegEvent
 ) ;
