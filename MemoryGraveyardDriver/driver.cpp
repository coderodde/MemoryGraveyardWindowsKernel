#include <ntddk.h>
#include <cstring>

/*
This device driver is not supposed to synchronize access to the graveyard buffer.
Also, it is not supposed to initialize the graveyard.
*/

#define BUFF_SIZE 1024

wchar_t graveyard[BUFF_SIZE];

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
static bool badParams(PVOID userBuffer, 
                      ULONGLONG offsetBytes,
                      ULONG lengthBytes) {

    if (userBuffer == nullptr) return true;

    if (lengthBytes == 0
        || offsetBytes % sizeof(wchar_t) != 0
        || lengthBytes % sizeof(wchar_t) != 0) {
     
        return true;
    }

    // The size of the graveyard buffer:
    ULONGLONG graveyardSizeBytes = sizeof(graveyard);

    // The actual bound check:
    return offsetBytes >= graveyardSizeBytes ||
           lengthBytes >  graveyardSizeBytes ||
           offsetBytes + lengthBytes > graveyardSizeBytes;
}

// Read handler:
static void serveReadRequest(PVOID userBuffer,
                             DWORD32 wcharOffset, 
                             ULONG wcharLength) {
    RtlCopyMemory(userBuffer, 
                  &graveyard[wcharOffset],
                  wcharLength * sizeof(wchar_t));
}

// Write handler:
static void serveWriteRequest(PVOID userBuffer,
                              DWORD32 wcharOffset,
                              ULONG wcharLength) {
    RtlCopyMemory(&graveyard[wcharOffset],
                  userBuffer, 
                  wcharLength * sizeof(wchar_t));
}

// The read handler for the driver:
NTSTATUS MyRead(IN PDEVICE_OBJECT DeviceObject, 
                IN PIRP Irp) {

    UNREFERENCED_PARAMETER(DeviceObject);

    auto stack = IoGetCurrentIrpStackLocation(Irp);
    auto lengthBytes = stack->Parameters.Read.Length;
    auto offsetBytes = stack->Parameters.Read.ByteOffset.QuadPart;

    DbgPrint("MyRead: Offset %llu, Length %lu\n",
             offsetBytes, 
             lengthBytes);

    PVOID userBuffer = Irp->AssociatedIrp.SystemBuffer;

    if (badParams(userBuffer, offsetBytes, lengthBytes)) {
        return badParamsReturn(Irp);
    }

    DWORD32 wcharOffset = static_cast<DWORD32>(offsetBytes / sizeof(wchar_t));
    ULONG wcharLength = lengthBytes / sizeof(wchar_t);

    serveReadRequest(userBuffer, wcharOffset, wcharLength);
    return normalReturn(Irp, wcharLength * sizeof(wchar_t));
}

// The write handler for the driver:
NTSTATUS MyWrite(IN PDEVICE_OBJECT DeviceObject, 
                 IN PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    auto stack = IoGetCurrentIrpStackLocation(Irp);
    auto lengthBytes = stack->Parameters.Write.Length;
    auto offsetBytes = stack->Parameters.Write.ByteOffset.QuadPart;

    DbgPrint("MyWrite: Offset %llu, Length %lu\n", 
             offsetBytes,
             lengthBytes);

    PVOID userBuffer = Irp->AssociatedIrp.SystemBuffer;

    if (badParams(userBuffer, offsetBytes, lengthBytes)) {
        return badParamsReturn(Irp);
    }

    DWORD32 wcharOffset = static_cast<DWORD32>(offsetBytes / sizeof(wchar_t));
    ULONG wcharLength = lengthBytes / sizeof(wchar_t);

    serveWriteRequest(userBuffer, wcharOffset, wcharLength);
    return normalReturn(Irp, wcharLength * sizeof(wchar_t));
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

    DriverObject->Flags |= DO_BUFFERED_IO;

    status = IoCreateSymbolicLink(&symbolicLink, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(DeviceObject);
        return status;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = MyCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = MyCreateClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = MyRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = MyWrite;
    DriverObject->DriverUnload = DriverUnload;

    DbgPrint("MemoryGraveyard driver loaded.\n");

    return STATUS_SUCCESS;
}
