# pragma once

//
// This function intercepts the IRP_MJ_CREATE before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreCreate (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    );

//
// This function intercepts the IRP_MJ_WRITE before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreRead(
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    );

//
// This function intercepts the IRP_MJ_WRITE before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreWrite (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    );

//
// This function intercepts the IRP_MJ_SET_INFORMATION before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreSetInformation (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    );

//
// This function intercepts the IRP_MJ_SET_SECURITY before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreSetSecurity (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    );

//
// This function intercepts the IRP_MJ_DEVICE_CONTROL before it happens
//
FLT_PREOP_CALLBACK_STATUS
PtPreDeviceControl (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    );

//
// This function intercepts the IRP_MJ_CREATE AFTER it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostCreate (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );


//
// This function intercepts the IRP_MJ_WRITE after it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostRead (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

//
// This function intercepts the IRP_MJ_WRITE after it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostWrite (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

//
// This function intercepts the IRP_MJ_SET_INFORMATION after it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostSetInformation (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

//
// This function intercepts the IRP_MJ_SET_SECURITY after it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostSetSecurity (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

//
// This function intercepts the IRP_MJ_DEVICE_CONTROL after it happens
//
FLT_POSTOP_CALLBACK_STATUS
PtPostDeviceControl (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );
