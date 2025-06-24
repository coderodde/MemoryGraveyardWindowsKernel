//#include <string>
//#include <iostream>
//#include <sstream>
//#include <vector>
//#include <Windows.h>
//
//static std::vector<std::wstring> split_by_whitespace(const std::wstring& input) {
//    std::wstringstream wss(input);
//    std::vector<std::wstring> tokens;
//    std::wstring word;
//
//    while (wss  >> word) {
//        tokens.push_back(word);
//    }
//
//    return tokens;
//}
//
//static void doRead(HANDLE device, const std::vector<std::wstring> tokens) {
//    if (tokens.size() < 3) {
//        std::wcerr << L"[ERROR] Invalid read command. Must be 'read TEXT START_INDEX'\n";
//        return;
//    }
//
//    const std::wstring  text = tokens[1];
//    const std::wstring index = tokens[2];
//
//    auto textLength = text.length() + 1;
//
//
//}
//
//static void doWrite(HANDLE device, const std::vector<std::wstring> tokens) {
//
//}
//
//int main() {
//
//    HANDLE device = CreateFile(L"\\\\.\\MemoryGraveyard",
//                               GENERIC_READ | GENERIC_WRITE,
//                               0,
//                               nullptr,
//                               OPEN_EXISTING,
//                               FILE_ATTRIBUTE_NORMAL,
//                               nullptr);
//
//    if (device == INVALID_HANDLE_VALUE) {
//        std::wcerr << L"[ERROR] Failed to open device: " << GetLastError() << L"\n";
//        return EXIT_FAILURE;
//    }
//
//    std::wcout << L"MemoryGraveyard Client started.\nType 'read TEXT START_INDEX' or 'write TEXT START_INDEX' to interact.\n";
//
//    while (true) {
//        if (std::wcin.eof()) {
//            return EXIT_SUCCESS;
//        }
//
//        if (std::wcin.fail()) {
//            return EXIT_FAILURE;
//        }
//
//        std::wstring cmd;
//        std::getline(std::wcin, cmd);
//        std::vector<std::wstring> tokens = split_by_whitespace(cmd);
//
//        if (tokens.empty()) {
//            continue;
//        }
//
//        if (tokens[0] == std::wstring(L"read")) {
//            doRead(device, tokens);
//        } else if (tokens[0] == std::wstring(L"write")) {
//            doWrite(device, tokens);
//        } else if (tokens[0] == std::wstring(L"exit")) {
//            return EXIT_SUCCESS;
//        }
//    }
//
//    return EXIT_SUCCESS;
//}

// test_graveyard.c
// test_graveyard.c
#include <Windows.h>
#include <stdio.h>

int main(void) {
    HANDLE h = CreateFileA(
        "\\\\.\\MemoryGraveyard",
        GENERIC_READ | GENERIC_WRITE,
        0,                    // no sharing
        NULL,                 // default security
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_ATTRIBUTE_NORMAL,// synchronous I/O
        NULL
    );

    if (h == INVALID_HANDLE_VALUE) {
        printf("CreateFile failed: %u\n", GetLastError());
        return 1;
    }

    // --- Seek to offset 200 ---
    LARGE_INTEGER pos;
    pos.QuadPart = 200;
    if (!SetFilePointerEx(h, pos, NULL, FILE_BEGIN)) {
        printf("SetFilePointerEx failed: %u\n", GetLastError());
        CloseHandle(h);
        return 2;
    }

    // --- Write 4 bytes at offset 200 ---
    /*
    BYTE data[] = {0x11, 0x22, 0x33, 0x44};
    DWORD written;
    if (!WriteFile(h, data, sizeof(data), &written, NULL)) {
        printf("WriteFile failed: %u\n", GetLastError());
        CloseHandle(h);
        return 3;
    }
    printf("Wrote %u bytes at offset %lld\n", written, pos.QuadPart);
    */
    // --- Seek back to offset 200 ---
    pos.QuadPart = 200;
    if (!SetFilePointerEx(h, pos, NULL, FILE_BEGIN)) {
        printf("SetFilePointerEx (read) failed: %u\n", GetLastError());
        CloseHandle(h);
        return 4;
    }

    // --- Read 4 bytes at offset 200 ---
    BYTE buf[4] = {0};
    DWORD read;
    if (!ReadFile(h, buf, sizeof(buf), &read, NULL)) {
        printf("ReadFile failed: %u\n", GetLastError());
        CloseHandle(h);
        return 5;
    }

    printf("Read %u bytes:", read);
    for (DWORD i = 0; i < read; ++i) {
        printf(" %02X", buf[i]);
    }
    printf("\n");

    CloseHandle(h);
    return 0;
}
