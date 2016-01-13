# pragma once

typedef struct
{
	LARGE_INTEGER Cookie;
	//KSPIN_LOCK	  spinlock;
} REGISTRY_CONTEXT, *PREGISTRY_CONTEXT;

NTSTATUS 
RegistryCallback
(
 IN PVOID CallbackContext, 
 IN PVOID  Argument1, 
 IN PVOID  Argument2
);

