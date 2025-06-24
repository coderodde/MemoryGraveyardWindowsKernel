#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <Windows.h>

#define BUFF_SIZE 1024

static alignas(512) BYTE readBuffer [BUFF_SIZE];
static alignas(512) BYTE writeBuffer[BUFF_SIZE];

static bool validParameters(const long long offset, const DWORD length) {

    if (offset < 0 || offset >= BUFF_SIZE) {

        std::cerr << "[ERROR] Offset out of bounds (" 
                  << offset 
                  << "). Must be between 0 and "
                  << BUFF_SIZE - 1
                  << ".\n";

        return false;
    }

    if (length <= 0 || offset + length > BUFF_SIZE) {

        std::cerr << "[ERROR] Length out of bounds ("
                  << length
                  << "). Must be positive and not exceed buffer size ("
                  << BUFF_SIZE
                  << ").\n";

        return false;
    }

    return true;
}

static std::vector<std::string> split_by_whitespace(const std::string& input) {

    std::stringstream ss(input);
    std::vector<std::string> tokens;
    std::string word;

    while (ss >> word) {
        tokens.push_back(word);
    }

    return tokens;
}

static void doRead(HANDLE device, const std::vector<std::string> tokens) {
    if (tokens.size() < 3) {
        std::cerr << "[ERROR] Invalid read command. "
                     "Must be 'read OFFSET LENGTH'\n";
        return;
    }

    auto offset = std::atoi(tokens[1].c_str());
    auto length = std::atoi(tokens[2].c_str());

    if (!validParameters(offset, length)) {
        return;
    }

    LARGE_INTEGER li;
    li.QuadPart = offset;

    if (!SetFilePointerEx(device, li, NULL, FILE_BEGIN)) {
        std::cerr << "[ERROR] Could not set file pointer to offset "
                  << offset
                  << ". Error code: "
                  << GetLastError()
                  << ".\n";
        return;
    }

    DWORD dwBytesRead = 0;

    if (!ReadFile(device, 
                  readBuffer + offset, 
                  length, 
                  &dwBytesRead, 
                  NULL)) {

        std::cerr << "[ERROR] Could not read from memory graveyard."
                     "Error code: "
                  << GetLastError()
                  << ".\n";
        return;
    }

    std::string text((char*) readBuffer + offset, length);

    std::cout << "[RESULT] '" << text << "'\n";
}

static void doWrite(HANDLE device, const std::vector<std::string> tokens) {
    if (tokens.size() < 3) {
        std::cerr << "[ERROR] Invalid write command. "
                     "Must be 'write OFFSET TEXT'\n";
        return;
    }

    auto offset = std::atoi(tokens[1].c_str());
    auto text = tokens[2];
    auto ctext = text.c_str();
    auto length = text.length() + 1; // +1 for null terminator!

    if (!validParameters(offset, length)) {
        return;
    }

    LARGE_INTEGER li;
    li.QuadPart = offset;

    if (!SetFilePointerEx(device, li, NULL, FILE_BEGIN)) {
        std::cerr << "[ERROR] Could not set file pointer to offset "
                  << offset
                  << ". Error code: "
                  << GetLastError()
                  << ".\n";
        return;
    }

    DWORD dwBytesWritten = 0;

    std::memcpy(&writeBuffer[offset],
                ctext, 
                tokens[2].length() + 1);

    if (!WriteFile(device,
                   writeBuffer, 
                   length, 
                   &dwBytesWritten,
                   NULL)) {

        std::cerr << "[ERROR] Could not write to memory graveyard. "
                  << "Error code: "
                  << GetLastError()
                  << ".\n";
    } else {
        std::cout << "[RESULT] Wrote '"
                  << text 
                  << "' to offset " 
                  << offset
                  << ".\n";
    }
}

int main() {

    HANDLE device = CreateFileA("\\\\.\\MemoryGraveyard",
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

    std::cout << "[INFO] Successfully opened a handle to MemoryGraveyard "
                 "device driver.\n"
                 "'read OFFSET LENGTH' or 'write OFFSET TEXT' "
                 "to interact.\n";

    while (true) {
        if (std::cin.eof()) {
            break;
        }

        if (std::cin.fail()) {
            std::cerr << "[ERROR] Console input failed.\n";
            CloseHandle(device);
            return EXIT_FAILURE;
        }

        std::string cmd;
        std::getline(std::cin, cmd);
        std::vector<std::string> tokens = split_by_whitespace(cmd);

        if (tokens.empty()) {
            continue;
        }

        if (tokens[0] == std::string("read")) {
            doRead(device, tokens);
        } else if (tokens[0] == std::string("write")) {
            doWrite(device, tokens);
        } else if (tokens[0] == std::string("exit")) {
            break;
        }
    }

    CloseHandle(device);
    std::cout << "[INFO] Closed the driver handle.\n";
    return EXIT_SUCCESS;
}