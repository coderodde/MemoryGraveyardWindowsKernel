#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>

static constexpr std::size_t BUFF_SIZE = 1024;
static char readBuffer[BUFF_SIZE];

int main(int argc, char* argv[]) {
    std::fstream device("\\\\.\\MemoryGraveyard");

    if (!device.good()) {
        std::cerr << "[ERROR] Could not open device. Error code: "
                  << GetLastError()
                  << ".\n";

        return EXIT_FAILURE;
    }

    if (argc < 3) {
        std::cerr << "[ERROR] Invalid command. Usage: "
                  << "GraveyardReader.exe OFFSET LENGTH\n";
                
        return EXIT_FAILURE;
    }

    std::size_t offset = std::atoi(argv[1]);
    std::size_t length = std::atoi(argv[2]);

    device.seekg(offset, std::ios::beg);
    device.read(readBuffer, length);
    readBuffer[length] = '\0';

    if (device.fail()) {
        std::cerr << "[ERROR] Could not read from memory graveyard. "
                  << "Error code: "
                  << GetLastError()
                  << ".\n";

        return EXIT_FAILURE;
    }

    std::cout << "[RESULT] Read from offset " 
              << offset 
              << ": '"
              << readBuffer 
              << "'\n";

    return EXIT_SUCCESS;
}