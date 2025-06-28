#include <ntddk.h>

/*
This device driver is not supposed to synchronize access to the graveyard buffer.
Also, it is not supposed to initialize the graveyard.
*/

static constexpr size_t BUFF_SIZE = 1024;

static char graveyard[BUFF_SIZE];

// Marks the Irp as completed and returns an error status for bad parameters:
static NTSTATUS BadParamsReturn(PIRP Irp) {
    Irp->IoStatus.Status      = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return STATUS_INVALID_PARAMETER;
}

static NTSTATUS InsufficientResourcesReturn(PIRP Irp) {
    Irp->IoStatus.Status      = STATUS_INSUFFICIENT_RESOURCES;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return STATUS_INSUFFICIENT_RESOURCES;
}

// Marks the Irp as completed and returns a normal status with transferred bytes:
static NTSTATUS NormalReturn(PIRP Irp, 
                             ULONG transferred = 0) {

    Irp->IoStatus.Status      = STATUS_SUCCESS;
    Irp->IoStatus.Information = transferred;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return STATUS_SUCCESS;
}

// Checks if the parameters are valid:
static bool BadParams(PVOID     userBuffer, 
                      ULONGLONG offsetBytes,
                      ULONG     lengthBytes) {

    if (userBuffer == nullptr || lengthBytes == 0) {
        return true;
    }

    // The size of the graveyard buffer:
    const ULONGLONG graveyardSize = sizeof(graveyard);

    if (offsetBytes >= graveyardSize) {
        return true;
    }

    return lengthBytes > graveyardSize - offsetBytes;
}

// Read handler:
static void ServeReadRequest(PVOID     userBuffer,
                             ULONGLONG offset, 
                             ULONG     length) {

    RtlCopyMemory(userBuffer, 
                  &graveyard[(size_t) offset],
                  length);
}

// Write handler:
static void ServeWriteRequest(PVOID     userBuffer,
                              ULONGLONG offset,
                              ULONG     length) {

    RtlCopyMemory(&graveyard[(size_t) offset],
                  userBuffer, 
                  length);
}

// The read handler for the driver:
NTSTATUS OnRead(IN PDEVICE_OBJECT DeviceObject, 
                IN PIRP           Irp) {

    UNREFERENCED_PARAMETER(DeviceObject);

    const PIO_STACK_LOCATION       stackLocation = IoGetCurrentIrpStackLocation(Irp);
    const ULONG                    length        = stackLocation->Parameters.Read.Length;
    const LARGE_INTEGER            byteOffset    = stackLocation->Parameters.Read.ByteOffset;
    const ULONGLONG                offset        = byteOffset.QuadPart;
          PVOID                    kernelBuffer  = NULL;

    if (Irp->MdlAddress) {
        kernelBuffer = 
            MmGetSystemAddressForMdlSafe(
                Irp->MdlAddress,
                NormalPagePriority);

        if (kernelBuffer == NULL) {
            return InsufficientResourcesReturn(Irp);
        }
    }

    if (BadParams(kernelBuffer, 
                  offset, 
                  length)) {

        return BadParamsReturn(Irp);
    }

    ServeReadRequest(kernelBuffer, 
                     offset, 
                     length);

    return NormalReturn(Irp, 
                        length);
}

// The write handler for the driver:
NTSTATUS OnWrite(IN PDEVICE_OBJECT DeviceObject, 
                 IN PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    const PIO_STACK_LOCATION stack  = IoGetCurrentIrpStackLocation(Irp);
    const ULONG              length = stack->Parameters.Write.Length;
    const ULONGLONG          offset = stack->Parameters.Write.ByteOffset.QuadPart;
          PVOID              userBuffer;

    if (Irp->MdlAddress == NULL) {
        return InsufficientResourcesReturn(Irp);
    }

    userBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress,
                                              NormalPagePriority);

    if (userBuffer == NULL) {
        return InsufficientResourcesReturn(Irp);
    }

    if (BadParams(userBuffer, 
                  offset, 
                  length)) {

        return BadParamsReturn(Irp);
    }

    ServeWriteRequest(userBuffer,
                      offset,
                      length);

    return NormalReturn(Irp, 
                        length);
}

NTSTATUS OnCreateClose(IN PDEVICE_OBJECT DeviceObject, 
                       IN PIRP Irp) {

    UNREFERENCED_PARAMETER(DeviceObject);
    return NormalReturn(Irp);
}

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject) {

    UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\MemoryGraveyard");
    IoDeleteSymbolicLink(&symbolicLink);
    IoDeleteDevice(DriverObject->DeviceObject);
}

extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    UNICODE_STRING deviceName   = RTL_CONSTANT_STRING(L"\\Device\\MemoryGraveyard");
    UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\MemoryGraveyard");
    PDEVICE_OBJECT DeviceObject = nullptr;

    NTSTATUS status = 
        IoCreateDevice(
             DriverObject,
             0,
             &deviceName,
             FILE_DEVICE_UNKNOWN,
             0,
             FALSE,
             &DeviceObject);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    DeviceObject->Flags |= DO_DIRECT_IO;

    status = IoCreateSymbolicLink(&symbolicLink, &deviceName);

    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(DeviceObject);
        return status;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = OnCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = OnCreateClose;
    DriverObject->MajorFunction[IRP_MJ_READ]   = OnRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE]  = OnWrite;
    DriverObject->DriverUnload                 = DriverUnload;

    return STATUS_SUCCESS;
}
    