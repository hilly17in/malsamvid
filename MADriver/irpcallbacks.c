# include "passthrough.h"

//
// This function intercepts the IRP_MJ_CREATE before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreCreate (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
	)
/*++

Routine Description:

    This routine is the main pre-operation dispatch routine for this
    miniFilter. Since this is just a simple passThrough miniFilter it
    does not do anything with the callbackData but rather return
    FLT_PREOP_SUCCESS_WITH_CALLBACK thereby passing it down to the next
    miniFilter in the chain.

    This is non-pageable because it could be called on the paging path

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The context for the completion routine for this
        operation.

Return Value:

    The return value is the status of the operation.

--*/
{
	NTSTATUS status;
	PFLT_FILE_NAME_INFORMATION FileNameInformation ;

	// we will try to open file with same name
	// so we need some variables

	ACCESS_MASK			DesiredAccess;
	ULONG				FileAttributes;
	ULONG				ShareAccess;
	ULONG				CreateDisposition;
	ULONG				CreateOptions, checkopt;
	UNICODE_STRING		checkFileName ;
	HANDLE				filehandle;
	IO_STATUS_BLOCK		ioStatusBlock;
	OBJECT_ATTRIBUTES	oa ;
	unsigned long		Pid;

    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
	
	if(!gPid)
		return FLT_PREOP_SUCCESS_NO_CALLBACK;


	Pid = (unsigned long)PsGetCurrentProcessId();
	if( !IsPidPresentinList(Pid) )
		return FLT_PREOP_SUCCESS_NO_CALLBACK;

	

	checkopt = (Data->Iopb->Parameters.Create.Options) & 0x00FFFFFF; 
	if(checkopt & FILE_DIRECTORY_FILE)
	{
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}


	//
    //  See if this is an operation we would like the operation status
    //  for.  If so request it.
    //
    //  NOTE: most filters do NOT need to do this.  You only need to make
    //        this call if, for example, you need to know if the oplock was
    //        actually granted.
    //

	// get the file name information
	status = FltGetFileNameInformation(Data, 
				  FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
				  &FileNameInformation) ;

	if (!NT_SUCCESS(status))
		goto cleanup ;

	// parse the file name information to get the normalized image name
	status = FltParseFileNameInformation(FileNameInformation) ;

	if (!NT_SUCCESS(status))
	{
		FltReleaseFileNameInformation(FileNameInformation) ;
		goto cleanup ;
	}

	CreateDisposition = (Data->Iopb->Parameters.Create.Options >>  24) & 0xFF; 
	if ((CreateDisposition == FILE_SUPERSEDE)|| (CreateDisposition == FILE_OVERWRITE_IF))
	{
		// Report to user mode program. - attempt to create a file..
		DbgPrint("this is created definite %wZ \n", &FileNameInformation->Name);
		SendMessageToUserMode(NotifyBehavior, NULL, CreateNewFile, gPid, &FileNameInformation->Name);

		// Release the file name finally
		FltReleaseFileNameInformation(FileNameInformation) ;
		goto cleanup;
	}
		
	// prepare the params
	DesiredAccess = Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess; 
	FileAttributes = Data->Iopb->Parameters.Create.FileAttributes ;
	ShareAccess = Data->Iopb->Parameters.Create.ShareAccess;
	CreateOptions = checkopt ; //CreateOptions = Data->Iopb->Parameters.Create.SecurityContext->FullCreateOptions; 

	// Construct a check file name, to check if the file exists 
	checkFileName.MaximumLength = checkFileName.Length = FileNameInformation->Name.Length; 
	
	status = MAllocateUnicodeString(&checkFileName);
	if(status != STATUS_SUCCESS)
	{
		FltReleaseFileNameInformation(FileNameInformation) ;
		goto cleanup;
	}


	RtlCopyMemory(checkFileName.Buffer, FileNameInformation->Name.Buffer, FileNameInformation->Name.Length);
	
	checkFileName.Length = FileNameInformation->Name.Length;

	// init the object attributes with the file name, etc. 
	InitializeObjectAttributes(&oa, 
							   &checkFileName, 
							   OBJ_KERNEL_HANDLE, 
							   NULL, NULL); 
	
	//
	// check if it is a new file
	status = FltCreateFile( gFilterHandle, 
							FltObjects->Instance, 
							&filehandle, 
							DesiredAccess, 
							&oa, 
							&ioStatusBlock, 
							(PLARGE_INTEGER) NULL, 
							FileAttributes, 
							ShareAccess, 
							FILE_OPEN, //CreateDisposition, 
							CreateOptions, 
							NULL, 
							0L, 0L ); 


	if(status == STATUS_SUCCESS) // if file exists
	{
		FltClose(filehandle);

		if( (CreateDisposition == FILE_OPEN) || 
			(CreateDisposition == FILE_OPEN_IF)|| 
			(CreateDisposition == FILE_OVERWRITE))
		{
			// trying to open a existing file
			// report to user program
			SendMessageToUserMode(NotifyBehavior, NULL, OpenExistFile, gPid, &FileNameInformation->Name);
		}
	
	}
	else
	{
		if( (CreateDisposition == FILE_CREATE) || 
			(CreateDisposition == FILE_OPEN_IF))
		{
			// trying to create a new file
			// report to user program
			SendMessageToUserMode(NotifyBehavior, NULL, CreateNewFile, 0, &FileNameInformation->Name);
		}
	}
	

	// Release the file name finally
	FltReleaseFileNameInformation(FileNameInformation) ;


cleanup:

    if (PtDoRequestOperationStatus( Data )) {

        status = FltRequestOperationStatusCallback( Data,
                                                    PtOperationStatusCallback,
                                                    (PVOID)(++OperationStatusCtx) );
        if (!NT_SUCCESS(status)) {

            PT_DBG_PRINT( PTDBG_TRACE_OPERATION_STATUS,
                          ("PassThrough!PtPreOperationPassThrough: FltRequestOperationStatusCallback Failed, status=%08x\n",
                           status) );
        }
    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

//
// This function intercepts the IRP_MJ_WRITE before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreRead(
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    )
{
	NTSTATUS status;
	PFLT_FILE_NAME_INFORMATION FileNameInformation ;
	unsigned long Pid ;

    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

    /*PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("PassThrough!PtPreOperationPassThrough: Entered\n") );*/

	if(!gPid)
		return FLT_PREOP_SUCCESS_NO_CALLBACK;


	Pid = (unsigned long)PsGetCurrentProcessId();
	if( !IsPidPresentinList(Pid) )
		return FLT_PREOP_SUCCESS_NO_CALLBACK;


	// get the file name information
	status = FltGetFileNameInformation(Data, 
				  FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
				  &FileNameInformation) ;

	if (!NT_SUCCESS(status))
		goto cleanup ;

	// parse the file name information to get the normalized image name
	status = FltParseFileNameInformation(FileNameInformation) ;

	if (!NT_SUCCESS(status))
	{
		FltReleaseFileNameInformation(FileNameInformation) ;
		goto cleanup ;
	}


	SendMessageToUserMode(NotifyBehavior, NULL, ReadingFile, 0, &FileNameInformation->Name);

	FltReleaseFileNameInformation(FileNameInformation) ;


    //
    //  See if this is an operation we would like the operation status
    //  for.  If so request it.
    //
    //  NOTE: most filters do NOT need to do this.  You only need to make
    //        this call if, for example, you need to know if the oplock was
    //        actually granted.
    //

cleanup:
    if (PtDoRequestOperationStatus( Data )) {

        status = FltRequestOperationStatusCallback( Data,
                                                    PtOperationStatusCallback,
                                                    (PVOID)(++OperationStatusCtx) );
        if (!NT_SUCCESS(status)) {

            PT_DBG_PRINT( PTDBG_TRACE_OPERATION_STATUS,
                          ("PassThrough!PtPreOperationPassThrough: FltRequestOperationStatusCallback Failed, status=%08x\n",
                           status) );
        }
    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

//
// This function intercepts the IRP_MJ_WRITE before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreWrite (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    )
{
	NTSTATUS status;
	PFLT_FILE_NAME_INFORMATION FileNameInformation ;
	unsigned long		Pid;
	

    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

    //PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
    //              ("PassThrough!PtPreOperationPassThrough: Entered\n") );

	if(!gPid)
		return FLT_PREOP_SUCCESS_NO_CALLBACK;


	Pid = (unsigned long)PsGetCurrentProcessId();
	if( !IsPidPresentinList(Pid) )
		return FLT_PREOP_SUCCESS_NO_CALLBACK;


	// get the file name information
	status = FltGetFileNameInformation(Data, 
				  FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
				  &FileNameInformation) ;

	if (!NT_SUCCESS(status))
		goto cleanup ;

	// parse the file name information to get the normalized image name
	status = FltParseFileNameInformation(FileNameInformation) ;

	if (!NT_SUCCESS(status))
	{
		FltReleaseFileNameInformation(FileNameInformation) ;
		goto cleanup ;
	}


	SendMessageToUserMode(NotifyBehavior, NULL, WritingFile, 0, &FileNameInformation->Name);

	FltReleaseFileNameInformation(FileNameInformation) ;


cleanup:
 
	//
    //  See if this is an operation we would like the operation status
    //  for.  If so request it.
    //
    //  NOTE: most filters do NOT need to do this.  You only need to make
    //        this call if, for example, you need to know if the oplock was
    //        actually granted.
    //

    if (PtDoRequestOperationStatus( Data )) {

        status = FltRequestOperationStatusCallback( Data,
                                                    PtOperationStatusCallback,
                                                    (PVOID)(++OperationStatusCtx) );
        if (!NT_SUCCESS(status)) {

            PT_DBG_PRINT( PTDBG_TRACE_OPERATION_STATUS,
                          ("PassThrough!PtPreOperationPassThrough: FltRequestOperationStatusCallback Failed, status=%08x\n",
                           status) );
        }
    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

//
// This function intercepts the IRP_MJ_SET_INFORMATION before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreSetInformation (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
	)
{
	NTSTATUS status;
	PFLT_FILE_NAME_INFORMATION FileNameInformation ;
	UINT32 uFileInfoClass = 0 ;
	USHORT code =0;
	unsigned long		Pid;

	FILE_DISPOSITION_INFORMATION *file_disp_info ;


    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

    //PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
    //              ("PassThrough!PtPreOperationPassThrough: Entered\n") );


	if(!gPid)
		return FLT_PREOP_SUCCESS_NO_CALLBACK;


	Pid = (unsigned long)PsGetCurrentProcessId();
	if( !IsPidPresentinList(Pid) )
		return FLT_PREOP_SUCCESS_NO_CALLBACK;

	// we will find if operation is delete
	uFileInfoClass = Data->Iopb->Parameters.SetFileInformation.FileInformationClass ;
	switch(uFileInfoClass)
	{
	case FileDispositionInformation:

		file_disp_info = ( (FILE_DISPOSITION_INFORMATION *)Data->Iopb->Parameters.SetFileInformation.InfoBuffer );
		if(file_disp_info)
		{
			if(file_disp_info->DeleteFile)
			{
				code = DeletesFile;
				DbgPrint("Delete file is true %d:\n", Pid);
			}
			else
			{
				return FLT_PREOP_SUCCESS_NO_CALLBACK;
			}

		}
		else
		{
			return FLT_PREOP_SUCCESS_NO_CALLBACK;
		}

		break;

	case FileBasicInformation:

		code = SetTimeStamp;

		break;

	default:

		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	// get the file name information
	status = FltGetFileNameInformation(Data, 
				  FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
				  &FileNameInformation) ;

	if (!NT_SUCCESS(status))
		goto cleanup ;

	// parse the file name information to get the normalized image name
	status = FltParseFileNameInformation(FileNameInformation) ;

	if (!NT_SUCCESS(status))
	{
		FltReleaseFileNameInformation(FileNameInformation) ;
		goto cleanup ;
	}


	SendMessageToUserMode(NotifyBehavior, NULL, code, 0, &FileNameInformation->Name);

	FltReleaseFileNameInformation(FileNameInformation) ;

	Data->IoStatus.Status = STATUS_ACCESS_DENIED;
	Data->IoStatus.Information = 0;

    return FLT_PREOP_COMPLETE;


cleanup:

    //
    //  See if this is an operation we would like the operation status
    //  for.  If so request it.
    //
    //  NOTE: most filters do NOT need to do this.  You only need to make
    //        this call if, for example, you need to know if the oplock was
    //        actually granted.
    //

    if (PtDoRequestOperationStatus( Data )) {

        status = FltRequestOperationStatusCallback( Data,
                                                    PtOperationStatusCallback,
                                                    (PVOID)(++OperationStatusCtx) );
        if (!NT_SUCCESS(status)) {

            PT_DBG_PRINT( PTDBG_TRACE_OPERATION_STATUS,
                          ("PassThrough!PtPreOperationPassThrough: FltRequestOperationStatusCallback Failed, status=%08x\n",
                           status) );
        }
    }

	return FLT_PREOP_SUCCESS_NO_CALLBACK;

}

//
// This function intercepts the IRP_MJ_SET_SECURITY before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreSetSecurity (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    )
{
	NTSTATUS status;
	PFLT_FILE_NAME_INFORMATION FileNameInformation ;
	unsigned long		Pid;

    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

    //PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
    //              ("PassThrough!PtPreOperationPassThrough: Entered\n") );

	if(!gPid)
		return FLT_PREOP_SUCCESS_NO_CALLBACK;


	Pid = (unsigned long)PsGetCurrentProcessId();
	if( !IsPidPresentinList(Pid) )
		return FLT_PREOP_SUCCESS_NO_CALLBACK;

	// get the file name information
	status = FltGetFileNameInformation(Data, 
				  FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
				  &FileNameInformation) ;

	if (!NT_SUCCESS(status))
		goto cleanup ;

	// parse the file name information to get the normalized image name
	status = FltParseFileNameInformation(FileNameInformation) ;

	if (!NT_SUCCESS(status))
	{
		FltReleaseFileNameInformation(FileNameInformation) ;
		goto cleanup ;
	}


	SendMessageToUserMode(NotifyBehavior, NULL, ChangesPerm, 0, &FileNameInformation->Name);

	FltReleaseFileNameInformation(FileNameInformation) ;


cleanup:
    //
    //  See if this is an operation we would like the operation status
    //  for.  If so request it.
    //
    //  NOTE: most filters do NOT need to do this.  You only need to make
    //        this call if, for example, you need to know if the oplock was
    //        actually granted.
    //

    if (PtDoRequestOperationStatus( Data )) {

        status = FltRequestOperationStatusCallback( Data,
                                                    PtOperationStatusCallback,
                                                    (PVOID)(++OperationStatusCtx) );
        if (!NT_SUCCESS(status)) {

            PT_DBG_PRINT( PTDBG_TRACE_OPERATION_STATUS,
                          ("PassThrough!PtPreOperationPassThrough: FltRequestOperationStatusCallback Failed, status=%08x\n",
                           status) );
        }
    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

//
// This function intercepts the IRP_MJ_DEVICE_CONTROL before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreDeviceControl (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
	)
{
	NTSTATUS status;
	PFLT_FILE_NAME_INFORMATION FileNameInformation ;  
	unsigned long dwIoControlCode;
	unsigned long		Pid;

	UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

    //PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
    //              ("PassThrough!PtPreOperationPassThrough: Entered\n") );

	if(!gPid)
		return FLT_PREOP_SUCCESS_NO_CALLBACK;


	Pid = (unsigned long)PsGetCurrentProcessId();
	if( !IsPidPresentinList(Pid) )
		return FLT_PREOP_SUCCESS_NO_CALLBACK;

	// get the file name information
	status = FltGetFileNameInformation(Data, 
				  FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
				  &FileNameInformation) ;

	if (!NT_SUCCESS(status))
		goto cleanup ;

	// parse the file name information to get the normalized image name
	status = FltParseFileNameInformation(FileNameInformation) ;

	if (!NT_SUCCESS(status))
	{
		FltReleaseFileNameInformation(FileNameInformation) ;
		goto cleanup ;
	}

	dwIoControlCode = Data->Iopb->Parameters.DeviceIoControl.Common.IoControlCode ;

	SendMessageToUserMode(NotifyBehavior, NULL, SendsIoctl, dwIoControlCode, &FileNameInformation->Name);

	FltReleaseFileNameInformation(FileNameInformation) ;


cleanup:
    //
    //  See if this is an operation we would like the operation status
    //  for.  If so request it.
    //
    //  NOTE: most filters do NOT need to do this.  You only need to make
    //        this call if, for example, you need to know if the oplock was
    //        actually granted.
    //

    if (PtDoRequestOperationStatus( Data )) {

        status = FltRequestOperationStatusCallback( Data,
                                                    PtOperationStatusCallback,
                                                    (PVOID)(++OperationStatusCtx) );
        if (!NT_SUCCESS(status)) {

            PT_DBG_PRINT( PTDBG_TRACE_OPERATION_STATUS,
                          ("PassThrough!PtPreOperationPassThrough: FltRequestOperationStatusCallback Failed, status=%08x\n",
                           status) );
        }
    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

//
// This function intercepts the IRP_MJ_CREATE AFTER it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostCreate (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );

	


    //PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
    //              ("PassThrough!PtPostOperationPassThrough: Entered\n") );

    return FLT_POSTOP_FINISHED_PROCESSING;

}


//
// This function intercepts the IRP_MJ_WRITE after it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostRead (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );

    //PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
    //              ("PassThrough!PtPostOperationPassThrough: Entered\n") );

    return FLT_POSTOP_FINISHED_PROCESSING;

}

//
// This function intercepts the IRP_MJ_WRITE after it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostWrite (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );

    //PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
    //              ("PassThrough!PtPostOperationPassThrough: Entered\n") );

    return FLT_POSTOP_FINISHED_PROCESSING;

}

//
// This function intercepts the IRP_MJ_SET_INFORMATION after it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostSetInformation (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
	UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );

    //PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
    //              ("PassThrough!PtPostOperationPassThrough: Entered\n") );

    return FLT_POSTOP_FINISHED_PROCESSING;

}

//
// This function intercepts the IRP_MJ_SET_SECURITY after it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostSetSecurity (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );

    //PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
    //              ("PassThrough!PtPostOperationPassThrough: Entered\n") );

    return FLT_POSTOP_FINISHED_PROCESSING;

}

//
// This function intercepts the IRP_MJ_DEVICE_CONTROL after it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostDeviceControl (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );

    //PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
    //              ("PassThrough!PtPostOperationPassThrough: Entered\n") );

    return FLT_POSTOP_FINISHED_PROCESSING;

}
