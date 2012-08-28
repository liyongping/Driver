#include "stdafx.h"


void key_opretateUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS key_opretateDefaultHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

typedef struct _deviceExtension
{
	PDEVICE_OBJECT DeviceObject;
	PDEVICE_OBJECT TargetDeviceObject;
	PDEVICE_OBJECT PhysicalDeviceObject;
	UNICODE_STRING DeviceInterface;
} key_opretate_DEVICE_EXTENSION, *Pkey_opretate_DEVICE_EXTENSION;

// {300dd3d9-002b-4fb3-a4f9-b0ce83fa4a11}
static const GUID GUID_key_opretateInterface = {0x300DD3D9, 0x2b, 0x4fb3, {0xa4, 0xf9, 0xb0, 0xce, 0x83, 0xfa, 0x4a, 0x11 } };

#ifdef __cplusplus
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING  RegistryPath);
#endif

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING  RegistryPath)
{
	unsigned i;
    
    NTSTATUS status = STATUS_SUCCESS;
    IO_STATUS_BLOCK io_status = {0};
    OBJECT_ATTRIBUTES object_attributes = {0};
    UNICODE_STRING keyPath = RTL_CONSTANT_STRING(L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
    UNICODE_STRING valueName = RTL_CONSTANT_STRING(L"SystemRoot");
    HANDLE rKey = NULL;
    KEY_VALUE_PARTIAL_INFORMATION key_infor;
    PKEY_VALUE_PARTIAL_INFORMATION ac_key_infor;
    ULONG key_infor_length = 0;
    InitializeObjectAttributes(
        &object_attributes,
        &keyPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);
    status = ZwOpenKey(&rKey,KEY_READ,&object_attributes);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("open key unsuccessfully!\n"));
    }

    status = ZwQueryValueKey(rKey, &valueName, KeyValuePartialInformation,
        &key_infor,sizeof(KEY_VALUE_PARTIAL_INFORMATION),&key_infor_length);
    if (!NT_SUCCESS(status) && status!=STATUS_BUFFER_OVERFLOW && status!=STATUS_BUFFER_TOO_SMALL)
    {
        KdPrint(("ZwQueryValueKey unsuccessfully!\n"));
    }
    ac_key_infor = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool,key_infor_length,'ITAG');
    if (ac_key_infor == NULL)
    {
        KdPrint(("ExAllocatePoolWithTag unsuccessfully!\n"));
    }
    status = ZwQueryValueKey(rKey, &valueName, KeyValuePartialInformation,
        ac_key_infor,key_infor_length,&key_infor_length);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("ZwQueryValueKey key unsuccessfully!\n"));
    }else
    {
        KdPrint(("ZwQueryValueKey key successfully!\n"));
    }
    if (rKey)
    {
        ZwClose(rKey);
    }

	DbgPrint("Hello from key_opretate!\n");
	
	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
		DriverObject->MajorFunction[i] = key_opretateDefaultHandler;

	DriverObject->DriverUnload = key_opretateUnload;
	DriverObject->DriverStartIo = NULL;

	return STATUS_SUCCESS;
}

void key_opretateUnload(IN PDRIVER_OBJECT DriverObject)
{
	DbgPrint("Goodbye from key_opretate!\n");
}

NTSTATUS key_opretateDefaultHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	Pkey_opretate_DEVICE_EXTENSION deviceExtension = NULL;
	
	IoSkipCurrentIrpStackLocation(Irp);
	deviceExtension = (Pkey_opretate_DEVICE_EXTENSION) DeviceObject->DeviceExtension;
	return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);
}
