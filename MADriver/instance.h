# pragma once

NTSTATUS
PtUnload (
    __in FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS
PtInstanceSetup (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_SETUP_FLAGS Flags,
    __in DEVICE_TYPE VolumeDeviceType,
    __in FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID
PtInstanceTeardownStart (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID
PtInstanceTeardownComplete (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );


NTSTATUS
PtInstanceQueryTeardown (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

VOID
PtOperationStatusCallback (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    __in NTSTATUS OperationStatus,
    __in PVOID RequesterContext
    );

FLT_PREOP_CALLBACK_STATUS
PtPreOperationNoPostOperationPassThrough (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    );

BOOLEAN
PtDoRequestOperationStatus(
    __in PFLT_CALLBACK_DATA Data
    );
