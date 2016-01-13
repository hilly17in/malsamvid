# include "passthrough.h"

#define REGISTRY_POOL_TAG 'pger'
#define MAX_REG_NAME_LEN 255


BOOLEAN 
GetRegistryObjectCompleteName
(
 PUNICODE_STRING pRegistryPath, 
 PUNICODE_STRING pPartialRegistryPath, 
 PVOID pRegistryObject
)
{
 	BOOLEAN foundCompleteName = FALSE;
 	BOOLEAN partial = FALSE;
 	

	/* Check to see if everything is valid */
    /* We sometimes see a partial registry object name which is actually complete
       however if fails one of these checks for some reason. Not sure whether to report
       this registry event */
    if((!MmIsAddressValid(pRegistryObject)) ||
        (pRegistryObject == NULL))
    {
        return FALSE;
    }
 	
    /* Check to see if the partial name is really the complete name */
    if(pPartialRegistryPath != NULL)
    {
        if( (((pPartialRegistryPath->Buffer[0] == '\\') || (pPartialRegistryPath->Buffer[0] == '%')) ||
            ((pPartialRegistryPath->Buffer[0] == 'T') && (pPartialRegistryPath->Buffer[1] == 'R') && (pPartialRegistryPath->Buffer[2] == 'Y') && (pPartialRegistryPath->Buffer[3] == '\\'))) )
        {
            RtlUnicodeStringCopy(pRegistryPath, pPartialRegistryPath);
            partial = TRUE;
            foundCompleteName = TRUE;
        }
    }
 	       
    if(!foundCompleteName)
    {
        /* Query the object manager in the kernel for the complete name */
        NTSTATUS status;
        ULONG returnedLength;
        PUNICODE_STRING pObjectName = NULL;
       
        status = ObQueryNameString(pRegistryObject, (POBJECT_NAME_INFORMATION)pObjectName, 0, &returnedLength );
        if(status == STATUS_INFO_LENGTH_MISMATCH)
        {
            pObjectName = ExAllocatePoolWithTag(NonPagedPool, returnedLength, REGISTRY_POOL_TAG);
            status = ObQueryNameString(pRegistryObject, (POBJECT_NAME_INFORMATION)pObjectName, returnedLength, &returnedLength );
            if(NT_SUCCESS(status))
            {
                RtlUnicodeStringCopy(pRegistryPath, pObjectName);
                foundCompleteName = TRUE;
            }
            ExFreePoolWithTag(pObjectName, REGISTRY_POOL_TAG);
        }
    }

    return foundCompleteName;
}


NTSTATUS 
RegistryCallback
(
 IN PVOID CallbackContext, 
 IN PVOID  Argument1, 
 IN PVOID  Argument2
)
{
	BOOLEAN			registryEventIsValid = FALSE;
	int				type;
	UNICODE_STRING	registryPath, valueName;
	ULONG			registryDataLength = 0;
	int				operation;
	unsigned long	Pid ;
	

	if(!gPid)
		return STATUS_SUCCESS;

	Pid = (unsigned long)PsGetCurrentProcessId();

	if(!IsPidPresentinList(Pid) && !(Pid == sPid) )
		return STATUS_SUCCESS;

	/* Allocate a large 64kb string ... maximum path name allowed in windows */
	registryPath.Length = 0;
	registryPath.MaximumLength = NTSTRSAFE_UNICODE_STRING_MAX_CCH * sizeof(WCHAR);
	registryPath.Buffer = ExAllocatePoolWithTag(NonPagedPool, registryPath.MaximumLength, REGISTRY_POOL_TAG);
	if (registryPath.Buffer == NULL)
		return STATUS_SUCCESS;
	

	/* Allocate a large  255 WCHAR string ... maximum name length allowed in windows */
	valueName.Length = 0;
	valueName.MaximumLength = MAX_REG_NAME_LEN * sizeof(WCHAR);
	valueName.Buffer = ExAllocatePoolWithTag(NonPagedPool, registryPath.MaximumLength, REGISTRY_POOL_TAG);
	if (valueName.Buffer == NULL)
		return STATUS_SUCCESS;

	// CAN NOTE THE TIME HERE.
	// SKIPPED

	type = (REG_NOTIFY_CLASS)Argument1;
	
	try{
		switch(type)
		{

		//case RegNtPreCreateKey:
		//	{
		//		HANDLE	handle;
		//		NTSTATUS tStat = 0;
		//		UNICODE_STRING usl;


		//		PREG_PRE_CREATE_KEY_INFORMATION createKey = (PREG_PRE_CREATE_KEY_INFORMATION)Argument2;
		//		OBJECT_ATTRIBUTES	oa;
		//		
		//		RtlZeroMemory(&oa, sizeof(OBJECT_ATTRIBUTES));

		//		RtlInitUnicodeString(&usl, L"\\Registry\\Machine\\Software\\Microsoft\\Cryptography\\RNG");
		//			// init the object attributes with the file name, etc. 
		//		InitializeObjectAttributes(&oa, 
		//					   &usl, 
		//					   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 
		//					   NULL, NULL); 

		//		tStat = ZwOpenKey(&handle,
		//					STANDARD_RIGHTS_READ,
		//					&oa);

		//		
		//		DbgPrint("getting the handle to key. %wZ status %x\n", createKey->CompleteName, tStat);

		//	}

		//	break;

		case RegNtPostCreateKey:
			{
				PREG_POST_CREATE_KEY_INFORMATION createKey = (PREG_POST_CREATE_KEY_INFORMATION)Argument2;
				if(NT_SUCCESS(createKey->Status))
				{
					PVOID* registryObject = createKey->Object;
					registryEventIsValid = GetRegistryObjectCompleteName(&registryPath, 
																		createKey->CompleteName, 
																		*registryObject);
					operation = CreateRegKey;
				}

				break;
			}

		case RegNtPostOpenKey:
			{
				PREG_POST_OPEN_KEY_INFORMATION openKey = (PREG_POST_OPEN_KEY_INFORMATION)Argument2;
				if(NT_SUCCESS(openKey->Status)) 
				{
					PVOID* registryObject = openKey->Object;
					registryEventIsValid = GetRegistryObjectCompleteName(&registryPath, 
																		 openKey->CompleteName, 
																		 *registryObject);

					operation = OpenRegKey;
				}
				break;
			}

		case RegNtPreDeleteKey:
			{
				PREG_DELETE_KEY_INFORMATION deleteKey = (PREG_DELETE_KEY_INFORMATION)Argument2;
				registryEventIsValid = GetRegistryObjectCompleteName(&registryPath, NULL, deleteKey->Object);				
			
				operation = DeleteKey;
				break;
			}

		case RegNtDeleteValueKey:
			{
				PREG_DELETE_VALUE_KEY_INFORMATION deleteValueKey = (PREG_DELETE_VALUE_KEY_INFORMATION)Argument2;
				registryEventIsValid = GetRegistryObjectCompleteName(&registryPath, NULL, deleteValueKey->Object);

				if((registryEventIsValid) && (deleteValueKey->ValueName->Length > 0))
				{
					 RtlUnicodeStringCatString(&valueName,L"\\");
					 RtlUnicodeStringCat(&valueName, deleteValueKey->ValueName);
				}

				operation = DeleteKey;

			}
			break;

		case RegNtPreSetValueKey:
			{
				PREG_SET_VALUE_KEY_INFORMATION setValueKey = (PREG_SET_VALUE_KEY_INFORMATION)Argument2;
				registryEventIsValid = GetRegistryObjectCompleteName(&registryPath, NULL, setValueKey->Object);

				if(registryEventIsValid)
				{
					RtlUnicodeStringCatString(&valueName,L"\\");
					RtlUnicodeStringCat(&valueName, setValueKey->ValueName);
				}

				operation = SetValueKey;
				break;
			}
				
		case RegNtEnumerateKey:
			{
				PREG_ENUMERATE_KEY_INFORMATION enumerateKey = (PREG_ENUMERATE_KEY_INFORMATION)Argument2;
				
				registryEventIsValid = GetRegistryObjectCompleteName(&registryPath, NULL, enumerateKey->Object);

				operation = EnumRegKey;
				
				break;
			}

		case RegNtEnumerateValueKey:
			{
				PREG_ENUMERATE_VALUE_KEY_INFORMATION enumerateValueKey = (PREG_ENUMERATE_VALUE_KEY_INFORMATION)Argument2;
				registryEventIsValid = GetRegistryObjectCompleteName(&registryPath, 
																	 NULL, 
																	 enumerateValueKey->Object);

				operation = RegEnumValueKey;
				break;
			}
	
		case RegNtQueryKey:
			{
				PREG_QUERY_KEY_INFORMATION queryKey = (PREG_QUERY_KEY_INFORMATION)Argument2;
				
				registryEventIsValid = GetRegistryObjectCompleteName(&registryPath, NULL, queryKey->Object);

				operation = RegQueryKey;
				break;
			}

		case RegNtQueryValueKey:
			{
				PREG_QUERY_VALUE_KEY_INFORMATION queryValueKey = (PREG_QUERY_VALUE_KEY_INFORMATION)Argument2;
				registryEventIsValid = GetRegistryObjectCompleteName(&registryPath, NULL, queryValueKey->Object);
				if((registryEventIsValid) && (queryValueKey->ValueName->Length > 0)) 
				{
					RtlUnicodeStringCatString(&registryPath,L"\\");
					RtlUnicodeStringCat(&registryPath, queryValueKey->ValueName);
				}
				operation = RegQueryValueKey;
				break;
			}

		} // switch on type
	}
	except( EXCEPTION_EXECUTE_HANDLER ) {
		registryEventIsValid = FALSE;
	}

	if(registryEventIsValid)
	{
		PREGISTRY_EVENT pRegistryEvent;
		pRegistryEvent = ExAllocatePoolWithTag(NonPagedPool, sizeof(REGISTRY_EVENT), REGISTRY_POOL_TAG);
		if(pRegistryEvent != NULL)
		{		
			UINT32 length ;
			RtlZeroMemory(pRegistryEvent, sizeof(REGISTRY_EVENT));

			pRegistryEvent->OpIndex = operation;

			length = (registryPath.Length < 1023)? registryPath.Length : 1022 ;
			RtlCopyBytes(pRegistryEvent->wRegPath, registryPath.Buffer, length);

			pRegistryEvent->wRegPath[length] = '\0';
			pRegistryEvent->wRegPath[length+1] = '\0';

			RtlCopyBytes(pRegistryEvent->wValueName, valueName.Buffer, valueName.Length);
			pRegistryEvent->wValueName[valueName.Length] = '\0';

			SendRegEventToUserMode(pRegistryEvent);
			ExFreePoolWithTag(pRegistryEvent, REGISTRY_POOL_TAG);
		}
	}

	if (registryPath.Buffer != NULL)
	{
		ExFreePoolWithTag(registryPath.Buffer, REGISTRY_POOL_TAG);
	}
	if (valueName.Buffer != NULL)
	{
		ExFreePoolWithTag(valueName.Buffer, REGISTRY_POOL_TAG);
	}

	return STATUS_SUCCESS;
}