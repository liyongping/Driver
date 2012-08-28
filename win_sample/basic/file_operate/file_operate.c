#include "stdafx.h"

void file_opretateUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS file_opretateCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS file_opretateDefaultHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS IOpenFile( PUNICODE_STRING fileName, PHANDLE pFileHandle);
NTSTATUS ICopyFile( PUNICODE_STRING srcFileName, PUNICODE_STRING dstFileName);

typedef struct _deviceExtension
{
	PDEVICE_OBJECT DeviceObject;
	PDEVICE_OBJECT TargetDeviceObject;
	PDEVICE_OBJECT PhysicalDeviceObject;
	UNICODE_STRING DeviceInterface;
} file_opretate_DEVICE_EXTENSION, *Pfile_opretate_DEVICE_EXTENSION;

// {6995cd10-6948-48f0-8b07-2c6ebdf4d287}
static const GUID GUID_file_opretateInterface = {0x6995CD10, 0x6948, 0x48f0, {0x8b, 0x7, 0x2c, 0x6e, 0xbd, 0xf4, 0xd2, 0x87 } };

#ifdef __cplusplus
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING  RegistryPath);
#endif

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING  RegistryPath)
{
	unsigned i;

    NTSTATUS status;
    UNICODE_STRING uSrcFileName = RTL_CONSTANT_STRING(L"\\??\\d:\\1.txt");
    UNICODE_STRING uDstFileName = RTL_CONSTANT_STRING(L"\\??\\d:\\2.txt");
    status = ICopyFile(&uSrcFileName,&uDstFileName);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("copy file unsuccessfully!\n"));
    }
    else
    {
        KdPrint(("copy file successfully!\n"));
    }
	DbgPrint("Hello from file_opretate!\n");

	
	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
		DriverObject->MajorFunction[i] = file_opretateDefaultHandler;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = file_opretateCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = file_opretateCreateClose;

	DriverObject->DriverUnload = file_opretateUnload;
	DriverObject->DriverStartIo = NULL;


	return STATUS_SUCCESS;
}

NTSTATUS IOpenFile( PUNICODE_STRING fileName, PHANDLE pFileHandle)
{
    NTSTATUS status = STATUS_SUCCESS;
    IO_STATUS_BLOCK io_status = {0};
    OBJECT_ATTRIBUTES object_attributes = {0};
    InitializeObjectAttributes(
        &object_attributes,
        fileName,
        OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
        NULL,
        NULL);
    status = ZwCreateFile(
        pFileHandle,
        GENERIC_READ|GENERIC_WRITE,
        &object_attributes,
        &io_status,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OPEN_IF,
        FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("Open file:%wZ unsuccessfully!\n"));
        return status;
    }
    return STATUS_SUCCESS;
}

NTSTATUS ICopyFile( PUNICODE_STRING srcFileName, PUNICODE_STRING dstFileName)
{
    HANDLE srcFileHandle = NULL, dstFileHandle = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    PVOID fileBuffer = NULL;
    int bufferLength = 4*1024;
    LARGE_INTEGER offset = {0};
    IO_STATUS_BLOCK io_status = {0};
    status = IOpenFile(srcFileName,&srcFileHandle);
    if (!NT_SUCCESS(status))
    {
        return status;
    }
    status = IOpenFile(dstFileName,&dstFileHandle);
    if (!NT_SUCCESS(status))
    {
        goto exitCopy;
    }
    do 
    {
        fileBuffer = ExAllocatePoolWithTag(NonPagedPool,bufferLength,'IMyT');
        if (fileBuffer == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exitCopy;
        }
        while (1)
        {
            status = ZwReadFile(srcFileHandle,NULL,NULL,NULL,
                &io_status,fileBuffer,bufferLength,&offset,NULL);
            if (!NT_SUCCESS(status))
            {
                if (status == STATUS_END_OF_FILE)
                    status = STATUS_SUCCESS;
                break;
            }
            bufferLength = io_status.Information;
            status = ZwWriteFile(dstFileHandle,NULL,NULL,NULL,&io_status,fileBuffer,bufferLength,&offset,NULL);
            if (!NT_SUCCESS(status))
                break;
            offset.QuadPart += bufferLength;
        }
    } while (0);

exitCopy:
    if (srcFileHandle != NULL)
    {
        ZwClose(srcFileHandle);
        srcFileHandle = NULL;
    }
    if (dstFileHandle != NULL)
    {
        ZwClose(dstFileHandle);
        dstFileHandle = NULL;
    }
    if (fileBuffer != NULL)
    {
        ExFreePool(fileBuffer);
        fileBuffer = NULL;
    }
    return status;
}

void file_opretateUnload(IN PDRIVER_OBJECT DriverObject)
{
	DbgPrint("Goodbye from file_opretate!\n");
}

NTSTATUS file_opretateCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS file_opretateDefaultHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	Pfile_opretate_DEVICE_EXTENSION deviceExtension = NULL;
	
	IoSkipCurrentIrpStackLocation(Irp);
	deviceExtension = (Pfile_opretate_DEVICE_EXTENSION) DeviceObject->DeviceExtension;
	return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);
}


