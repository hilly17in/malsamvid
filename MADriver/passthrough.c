/*++

Module Name:

    passThrough.c

Abstract:

    This is the main module of the miniFilter driver.
    This filter hooks all IO operations for both pre and post operation
    callbacks.  The filter passes through the operations.

Environment:

    Kernel mode

--*/


# include "passthrough.h"


/*************************************************************************
    Prototypes
*************************************************************************/

NTSTATUS
DriverEntry (
    __in PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING RegistryPath
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#endif

/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    __in PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This is the initialization routine for this miniFilter driver.  This
    registers with FltMgr and initializes all global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.

    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Returns STATUS_SUCCESS.

--*/
{
    NTSTATUS	status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( RegistryPath );

	//store the driver object address in global pointer
	gpDriverObject = DriverObject;

	status = InitDriver(DriverObject) ;
	if (status != STATUS_SUCCESS)
	{
		goto complete ;
	}
	
complete:
   return status;
} // DriverEntry

NTSTATUS
PtUnload (
    __in FLT_FILTER_UNLOAD_FLAGS Flags
    )
/*++

Routine Description:

    This is the unload routine for this miniFilter driver. This is called
    when the minifilter is about to be unloaded. We can fail this unload
    request if this is not a mandatory unloaded indicated by the Flags
    parameter.

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    Returns the final status of this operation.

--*/
{
	NTSTATUS status;
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    /*PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("PassThrough!PtUnload: Entered\n") );*/

	status = CmUnRegisterCallback(RegContext.Cookie);

	status = PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, TRUE) ;

	UnRegisterMAFilter() ;

	CleanDriver() ;

	return STATUS_SUCCESS;
}


NTSTATUS InitDriver(PDRIVER_OBJECT DriverObject)
{
	NTSTATUS		status = STATUS_SUCCESS;

	UNICODE_STRING		usDeviceName ;
	UNICODE_STRING		usDosDeviceName ;
	PDEVICE_OBJECT		pDeviceObject ;
	BOOLEAN				IsDeviceObject = FALSE;
	BOOLEAN				IsSymLink = FALSE ; 
	BOOLEAN				IsProcessCreateNotify = FALSE;
	BOOLEAN				IsRegistryCallback = FALSE;
	//OB_CALLBACK_REGISTRATION	ObCallbackRegistration;
	//OB_OPERATION_REGISTRATION	ObOperation;
	//PVOID				RegistrationHandle ;


	try {
		//
		// Initialize the string for device name
		//
		RtlInitUnicodeString(&usDeviceName, MAFILTER_DEVICE_NAME) ;

		// create a device object to accept some ioctls
		status = IoCreateDevice(DriverObject, 0, 
						  &usDeviceName,
						  FILE_DEVICE_UNKNOWN,
						  0,
						  FALSE,
						  &pDeviceObject) ;

		if (!NT_SUCCESS( status )) {
				leave ;
		}

		IsDeviceObject = TRUE ;

		//
		// Initialize Dispatch Routines
		//
		DriverObject->MajorFunction[IRP_MJ_CREATE]=MADispatchCreate ;
		DriverObject->MajorFunction[IRP_MJ_CLOSE]=MADispatchClose ;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]= MADispatchIoctl ;
		
		//
		// Set the flags
		//
		pDeviceObject->Flags |= DO_BUFFERED_IO ;
		pDeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);
		
		RtlInitUnicodeString(&usDosDeviceName, MAFILTER_SYMLINK_NAME) ;

		status = IoCreateSymbolicLink((PUNICODE_STRING)&usDosDeviceName, 
							(PUNICODE_STRING)&usDeviceName);
		
		if (!NT_SUCCESS( status )) 	{
				leave ;
		}

		IsSymLink = TRUE ;

		// if here, we can save DriverObject state to our global variables
		gpDriverObject = DriverObject ;

		
		status = RegisterMAFilter() ;
		if (!NT_SUCCESS( status )) {
				leave ;
		}

		status = PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, FALSE) ;
		if (!NT_SUCCESS( status )) {
				leave ;
		}
		else{
			IsProcessCreateNotify = TRUE;
		}

		status = CmRegisterCallback(RegistryCallback, &RegContext, &RegContext.Cookie);
		if (!NT_SUCCESS( status )) {
			leave ;
		}
		else {
			IsRegistryCallback = TRUE;
		}

	
		//ObCallbackRegistration.Version = OB_FLT_REGISTRATION_VERSION;
		//
		//RtlInitUnicodeString(&ObCallbackRegistration.Altitude, "370030");

		//ObCallbackRegistration.OperationRegistrationCount = 1;
		//ObCallbackRegistration.RegistrationContext = NULL;
		//ObCallbackRegistration.OperationRegistration = &ObOperation ;

		//ObOperation.ObjectType = PsProcessType ;
		//ObOperation.Operations = OB_OPERATION_HANDLE_CREATE;
		//ObOperation.PreOperation  = ObPreCallback;
		//ObOperation.PostOperation = ObPostCallback;


		//status = ObRegisterCallbacks(&ObCallbackRegistration, &RegistrationHandle) ;
		//		if (!NT_SUCCESS( status )) {
		//		leave ;
		//}

		//
		// Initialize the Look aside List for having trusted PIDs
		//
		ExInitializeNPagedLookasideList(
			&gPidList,
			NULL, NULL,
			0, sizeof(PROCESS_LIST),
			'DIPM', 0 );

		InitializeListHead(&leProcessList);

	} finally {

			if (!NT_SUCCESS( status )) {

				if (IsSymLink) {
					IoDeleteSymbolicLink(&usDosDeviceName) ;
					IsSymLink = FALSE ;
				}
				if (IsDeviceObject) {
					IoDeleteDevice(DriverObject->DeviceObject) ;
					IsDeviceObject = FALSE  ;
				}

				if(IsProcessCreateNotify)
					status = PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, TRUE) ;
				
				if(IsRegistryCallback)
					status = CmUnRegisterCallback(RegContext.Cookie);

				UnRegisterMAFilter();
			}

	}
	return status ;
}

VOID CleanDriver()
{
	UNICODE_STRING	usDosDeviceName ;

	RtlInitUnicodeString(&usDosDeviceName, MAFILTER_SYMLINK_NAME) ;

	IoDeleteSymbolicLink(&usDosDeviceName);

	IoDeleteDevice(gpDriverObject->DeviceObject) ;

	Clean_gPidList() ;

	ExDeleteNPagedLookasideList(&gPidList);

}

NTSTATUS RegisterMAFilter()
{
	NTSTATUS				status ;
	PSECURITY_DESCRIPTOR	sd ;
    OBJECT_ATTRIBUTES		oa;
    UNICODE_STRING			uniString;

	try {
		
		//
		//  Register with FltMgr to tell it our callback routines
		//
		status = FltRegisterFilter( gpDriverObject,
									&FilterRegistration,
									&gFilterHandle );

        if (!NT_SUCCESS( status )) {
           leave;
        }

		//
		// build a security descriptor
		status = FltBuildDefaultSecurityDescriptor(&sd, FLT_PORT_ALL_ACCESS) ;
		if (!NT_SUCCESS(status)) {
            leave;
        }

	    RtlInitUnicodeString( &uniString, MAFILTER_PORT_NAME );

		InitializeObjectAttributes( &oa,
                                    &uniString,
                                    OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    sd );
		//
		// Create a server communication port
        status = FltCreateCommunicationPort( gFilterHandle,
                                             &gServerPort,
                                             &oa,
                                             NULL,
                                             MAFilterConnect,
                                             MAFilterDisconnect,
                                             MAFilterMessage,
                                             2 );
        
		FltFreeSecurityDescriptor(sd);
		
		if (!NT_SUCCESS(status)) {
            leave;
        }

		status = FltStartFiltering(gFilterHandle);

		

	} finally {

			if (!NT_SUCCESS(status)) {
				// 
				// Close the  handle to filter
				if (NULL != gFilterHandle) 
					FltUnregisterFilter(gFilterHandle);

				// close the communication port
				if(NULL != gServerPort)
					FltCloseCommunicationPort(gServerPort);

			} // if
		} // finally
	
	return status ;
} // RegisterFilter

VOID UnRegisterMAFilter()
{
	//
	// Close the communication port
	if (NULL != gServerPort)
		FltCloseCommunicationPort(gServerPort) ;

	// 
	// Close the  handle to filter
	if (NULL != gFilterHandle) 
		FltUnregisterFilter(gFilterHandle);

	DbgPrint("unregistered filter \n");
} //UnRegisterPecFilter

VOID Clean_gPidList()
{
	PLIST_ENTRY		pList;
	PPROCESS_LIST	pTemp;

	while(!IsListEmpty(&leProcessList) )
	{
		pList = RemoveHeadList(&leProcessList);
		pTemp = (PPROCESS_LIST)CONTAINING_RECORD(pList, PROCESS_LIST, ListEntry);
		ExFreeToNPagedLookasideList(&gPidList, (PVOID)pTemp);
	}
}