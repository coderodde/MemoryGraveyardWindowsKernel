//
//
//// test_graveyard.c
//// test_graveyard.c
//#include <Windows.h>
//#include <stdio.h>
//
//int main(void) {
//    HANDLE h = CreateFileA(
//        "\\\\.\\MemoryGraveyard",
//        GENERIC_READ | GENERIC_WRITE,
//        0,                    // no sharing
//        NULL,                 // default security
//        FILE_SHARE_READ | FILE_SHARE_WRITE,
//        FILE_ATTRIBUTE_NORMAL,// synchronous I/O
//        NULL
//    );
//
//    if (h == INVALID_HANDLE_VALUE) {
//        printf("CreateFile failed: %u\n", GetLastError());
//        return 1;
//    }
//
//    // --- Seek to offset 200 ---
//    LARGE_INTEGER pos;
//    pos.QuadPart = 200;
//    if (!SetFilePointerEx(h, pos, NULL, FILE_BEGIN)) {
//        printf("SetFilePointerEx failed: %u\n", GetLastError());
//        CloseHandle(h);
//        return 2;
//    }
//
//    // --- Write 4 bytes at offset 200 ---
//    /*
//    BYTE data[] = {0x11, 0x22, 0x33, 0x44};
//    DWORD written;
//    if (!WriteFile(h, data, sizeof(data), &written, NULL)) {
//        printf("WriteFile failed: %u\n", GetLastError());
//        CloseHandle(h);
//        return 3;
//    }
//    printf("Wrote %u bytes at offset %lld\n", written, pos.QuadPart);
//    */
//    // --- Seek back to offset 200 ---
//    pos.QuadPart = 200;
//    if (!SetFilePointerEx(h, pos, NULL, FILE_BEGIN)) {
//        printf("SetFilePointerEx (read) failed: %u\n", GetLastError());
//        CloseHandle(h);
//        return 4;
//    }
//
//    // --- Read 4 bytes at offset 200 ---
//    BYTE buf[4] = {0};
//    DWORD read;
//    if (!ReadFile(h, buf, sizeof(buf), &read, NULL)) {
//        printf("ReadFile failed: %u\n", GetLastError());
//        CloseHandle(h);
//        return 5;
//    }
//
//    printf("Read %u bytes:", read);
//    for (DWORD i = 0; i < read; ++i) {
//        printf(" %02X", buf[i]);
//    }
//    printf("\n");
//
//    CloseHandle(h);
//    return 0;
//}
