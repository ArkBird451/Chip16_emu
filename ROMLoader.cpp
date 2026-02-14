#include "ROMLoader.h"
#include <fstream>
#include <iostream>
#include <vector>

// Load ROM from file
bool ROMLoader::loadROM(const std::string& filepath, Memory& memory, uint16_t& startAddr) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filepath << std::endl;
        return false;
    }
    
    // Get file size
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (fileSize < 4) {
        std::cerr << "Error: File too small" << std::endl;
        return false;
    }
    
    // Read first 4 bytes to check format
    char magic[4];
    file.read(magic, 4);
    file.seekg(0, std::ios::beg);
    
    // Check if it's a C16 format file
    if (magic[0] == 'C' && magic[1] == 'H' && magic[2] == '1' && magic[3] == '6') {
        // C16 format with header
        if (fileSize < sizeof(C16Header)) {
            std::cerr << "Error: Invalid C16 header" << std::endl;
            return false;
        }
        
        // Read header
        C16Header header;
        file.read((char*)&header, sizeof(C16Header));
        
        // Validate header
        if (header.magic[0] != 'C' || header.magic[1] != 'H' || 
            header.magic[2] != '1' || header.magic[3] != '6') {
            std::cerr << "Error: Invalid magic number" << std::endl;
            return false;
        }
        
        // Read ROM data
        std::vector<uint8_t> romData(header.romSize);
        file.read((char*)romData.data(), header.romSize);
        
        if (!file) {
            std::cerr << "Error: Could not read ROM data" << std::endl;
            return false;
        }
        
        // Load ROM into memory
        memory.reset();
        memory.loadROM(romData.data(), header.romSize, header.startAddr);
        startAddr = header.startAddr;
        
        std::cout << "Loaded C16 ROM: " << filepath << std::endl;
        std::cout << "  ROM Size: " << header.romSize << " bytes" << std::endl;
        std::cout << "  Start Address: 0x" << std::hex << startAddr << std::endl;
        std::cout << "  Spec Version: 0x" << (int)header.specVersion << std::dec << std::endl;
        
    } else {
        // Raw binary format
        std::vector<uint8_t> romData(fileSize);
        file.read((char*)romData.data(), fileSize);
        
        if (!file) {
            std::cerr << "Error: Could not read ROM data" << std::endl;
            return false;
        }
        
        // Load ROM into memory at 0x0000
        memory.reset();
        memory.loadROM(romData.data(), fileSize, 0x0000);
        startAddr = 0x0000;
        
        std::cout << "Loaded raw binary ROM: " << filepath << std::endl;
        std::cout << "  ROM Size: " << fileSize << " bytes" << std::endl;
        std::cout << "  Start Address: 0x0000" << std::endl;
    }
    
    file.close();
    return true;
}
