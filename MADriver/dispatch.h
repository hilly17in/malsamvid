# pragma once


NTSTATUS 
MADispatchCreate
(
 PDEVICE_OBJECT pDevObject, 
 PIRP pIrp
) ;

NTSTATUS 
MADispatchClose
(
 PDEVICE_OBJECT pDevObj, 
 PIRP pIrp
) ;

NTSTATUS 
MADispatchIoctl
(
 PDEVICE_OBJECT pDevObj, 
 PIRP pIrp
 ) ;
