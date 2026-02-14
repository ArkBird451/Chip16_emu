// Chip16 Emulator - Main Entry Point

#include <iostream>
#include "Memory.h"
#include "CPU.h"
#include "ROMLoader.h"
#include "raylib.h"  // Test Raylib include

int main(int argc, char* argv[]) {
    std::cout << "Chip16 Emulator" << std::endl;
    
    // Check command line arguments
    if (argc < 2) {
        std::cout << "Usage: Chip16_emu <rom_file.c16|.bin>" << std::endl;
        std::cout << "  Supports both binary and C16 format ROMs" << std::endl;
        return 1;
    }
    
    // Create emulator components
    Memory memory;
    CPU cpu(memory);
    ROMLoader loader;
    
    // Load ROM
    uint16_t startAddr = 0x0000;
    if (!loader.loadROM(argv[1], memory, startAddr)) {
        std::cerr << "Failed to load ROM" << std::endl;
        return 1;
    }
    
    // Reset CPU to start address
    cpu.reset(startAddr);
    
    std::cout << std::endl << "Starting Execution" << std::endl << std::endl;
    
    // Run emulator
    cpu.run(-1);
    
    std::cout << std::endl << "Execution Finished" << std::endl << std::endl;
    
    // Dump final CPU state
    cpu.dumpState();
    
    return 0;
}
