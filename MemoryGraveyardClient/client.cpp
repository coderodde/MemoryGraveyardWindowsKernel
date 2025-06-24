#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <Windows.h>

static std::vector<std::wstring> split_by_whitespace(const std::wstring& input) {
    std::wstringstream wss(input);
    std::vector<std::wstring> tokens;
    std::wstring word;

    while (wss  >> word) {
        tokens.push_back(word);
    }

    return tokens;
}

static void doRead(HANDLE device, const std::vector<std::wstring> tokens) {
    if (tokens.size() < 3) {
        std::wcerr << L"[ERROR] Invalid read command. Must be 'read TEXT START_INDEX'\n";
        return;
    }

    const std::wstring  text = tokens[1];
    const std::wstring index = tokens[2];

    auto textLength = text.length() + 1;


}

static void doWrite(HANDLE device, const std::vector<std::wstring> tokens) {

}

int main() {

    HANDLE device = CreateFile(L"\\\\.\\MemoryGraveyard",
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               nullptr,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               nullptr);

    if (device == INVALID_HANDLE_VALUE) {
        std::wcerr << L"[ERROR] Failed to open device: " << GetLastError() << L"\n";
        return EXIT_FAILURE;
    }

    std::wcout << L"MemoryGraveyard Client started.\nType 'read TEXT START_INDEX' or 'write TEXT START_INDEX' to interact.\n";

    while (true) {
        if (std::wcin.eof()) {
            return EXIT_SUCCESS;
        }

        if (std::wcin.fail()) {
            return EXIT_FAILURE;
        }

        std::wstring cmd;
        std::getline(std::wcin, cmd);
        std::vector<std::wstring> tokens = split_by_whitespace(cmd);

        if (tokens.empty()) {
            continue;
        }

        if (tokens[0] == std::wstring(L"read")) {
            doRead(device, tokens);
        } else if (tokens[0] == std::wstring(L"write")) {
            doWrite(device, tokens);
        } else if (tokens[0] == std::wstring(L"exit")) {
            return EXIT_SUCCESS;
        }
    }

    return EXIT_SUCCESS;
}