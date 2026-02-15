#include <iostream>
#include "Memory.h"
#include "CPU.h"
#include "ROMLoader.h"
#include "raylib.h"

// Chip16 screen resolution
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define WINDOW_SCALE 3

int main(int argc, char* argv[]) {
    std::cout << "Chip16 Emulator" << std::endl;
    
    // Check command line arguments
    if (argc < 2) {
        std::cout << "Usage: Chip16_emu <rom_file.c16|.bin>" << std::endl;
        std::cout << "Supports both raw binary and C16 format ROMs" << std::endl;
        return 1;
    }
    
    // Initialize window
    InitWindow(SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE, "Chip16 Emulator");
    SetTargetFPS(60);
    
    // Create emulator components (heap allocation)
    Memory* memory = new Memory();
    CPU* cpu = new CPU(*memory);
    ROMLoader loader;
    
    // Load ROM
    uint16_t startAddr = 0x0000;
    if (!loader.loadROM(argv[1], *memory, startAddr)) {
        std::cerr << "Failed to load ROM" << std::endl;
        delete cpu;
        delete memory;
        CloseWindow();
        return 1;
    }
    
    // Reset CPU to start address
    cpu->reset(startAddr);
    
    std::cout << std::endl << "Starting Execution" << std::endl << std::endl;
    
    // Main loop
    while (!WindowShouldClose()) {
        // Execute one CPU instruction per frame (for now)
        cpu->step();
        
        // Render
        BeginDrawing();
        ClearBackground(BLACK);
        
        // Draw placeholder text
        DrawText("Chip16 Emulator", 10, 10, 20, WHITE);
        DrawText("Graphics not yet implemented", 10, 40, 20, GRAY);
        
        EndDrawing();
    }
    
    std::cout << std::endl << "Execution Finished" << std::endl << std::endl;
    
    // Dump final CPU state
    cpu->dumpState();
    
    // Cleanup
    delete cpu;
    delete memory;
    CloseWindow();
    
    return 0;
}
