#include <ntddk.h>

/*
This device driver is not supposed to synchronize access to the graveyard buffer.
Also, it is not supposed to initialize the graveyard.
*/

#define BUFF_SIZE 1024

static char graveyard[BUFF_SIZE];

// Marks the Irp as completed and returns an error status for bad parameters:
static NTSTATUS badParamsReturn(PIRP Irp) {
    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_PARAMETER;
}

// Marks the Irp as completed and returns a normal status with transferred bytes:
static NTSTATUS normalReturn(PIRP Irp, 
                             ULONG transferred = 0) {

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = transferred;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

// Checks if the parameters are valid:
static bool badParams(PVOID     userBuffer, 
                      ULONGLONG offsetBytes,
                      ULONG     lengthBytes) {

    if (userBuffer == nullptr || lengthBytes == 0) return true;

    // The size of the graveyard buffer:
    ULONGLONG graveyardSize = sizeof(graveyard);

    if (offsetBytes >= graveyardSize) {
        return true;
    }

    return lengthBytes > graveyardSize - offsetBytes;
}

// Read handler:
static void serveReadRequest(PVOID     userBuffer,
                             ULONGLONG offset, 
                             ULONG     length) {

    RtlCopyMemory(userBuffer, 
                  &graveyard[(size_t) offset],
                  length);
}

// Write handler:
static void serveWriteRequest(PVOID     userBuffer,
                              ULONGLONG offset,
                              ULONG     length) {

    RtlCopyMemory(&graveyard[(size_t) offset],
                  userBuffer, 
                  length);
}

// The read handler for the driver:
NTSTATUS MyRead(IN PDEVICE_OBJECT DeviceObject, 
                IN PIRP           Irp) {

    UNREFERENCED_PARAMETER(DeviceObject);

    auto stack  = IoGetCurrentIrpStackLocation(Irp);
    auto length = stack->Parameters.Read.Length;
    auto offset = stack->Parameters.Read.ByteOffset.QuadPart;

    DbgPrint("MyRead: offset %llu, length %lu\n",
             offset, 
             length);

    PVOID userBuffer = Irp->UserBuffer;

    if (badParams(userBuffer, offset, length)) {
        return badParamsReturn(Irp);
    }

    serveReadRequest(userBuffer, 
                     offset, 
                     length);

    return normalReturn(Irp, 
                        length);
}

// The write handler for the driver:
NTSTATUS MyWrite(IN PDEVICE_OBJECT DeviceObject, 
                 IN PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    auto stack  = IoGetCurrentIrpStackLocation(Irp);
    auto length = stack->Parameters.Write.Length;
    auto offset = stack->Parameters.Write.ByteOffset.QuadPart;

    DbgPrint("MyWrite: offset %llu, length %lu\n", 
             offset,
             length);

    PVOID userBuffer = Irp->UserBuffer;

    if (badParams(userBuffer, offset, length)) {
        return badParamsReturn(Irp);
    }

    serveWriteRequest(userBuffer,
                      offset,
                      length);

    return normalReturn(Irp, 
                        length);
}

NTSTATUS MyCreateClose(IN PDEVICE_OBJECT DeviceObject, 
                       IN PIRP Irp) {

    UNREFERENCED_PARAMETER(DeviceObject);
    return normalReturn(Irp);
}

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject) {

    UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\MemoryGraveyard");
    IoDeleteSymbolicLink(&symbolicLink);
    IoDeleteDevice(DriverObject->DeviceObject);
    DbgPrint("MemoryGraveyard driver unloaded.\n");
}

extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    UNICODE_STRING deviceName   = RTL_CONSTANT_STRING(L"\\Device\\MemoryGraveyard");
    UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\MemoryGraveyard");
    PDEVICE_OBJECT DeviceObject = nullptr;

    NTSTATUS status = IoCreateDevice(
        DriverObject,
        0,
        &deviceName,
        FILE_DEVICE_UNKNOWN,
        0,
        FALSE,
        &DeviceObject);

    if (!NT_SUCCESS(status)) return status;
    graveyard[0] = 'a';
    graveyard[1] = 'b';
    graveyard[2] = 'c';
    DeviceObject->Flags |= DO_BUFFERED_IO;
//    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    status = IoCreateSymbolicLink(&symbolicLink, &deviceName);

    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(DeviceObject);
        return status;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = MyCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = MyCreateClose;
    DriverObject->MajorFunction[IRP_MJ_READ]   = MyRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE]  = MyWrite;
    DriverObject->DriverUnload                 = DriverUnload;

    DbgPrint("MemoryGraveyard driver loaded.\n");

    return STATUS_SUCCESS;
}
    