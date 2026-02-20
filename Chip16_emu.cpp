#include <iostream>
#include "Memory.h"
#include "CPU.h"
#include "Graphics.h"
#include "Input.h"
#include "Sound.h"
#include "ROMLoader.h"
#include "raylib.h"

// Chip16 screen resolution
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define WINDOW_SCALE 3

// CPU timing (1MHz = 1,000,000 cycles per second)
#define CPU_CLOCK_HZ 1000000

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
    Graphics* graphics = new Graphics();
    Input* input = new Input(*memory);
    SoundManager* sound = new SoundManager();
    CPU* cpu = new CPU(*memory);
    cpu->setGraphics(graphics);
    cpu->setSound(sound);
    ROMLoader loader;
    
    // Initialize audio system
    sound->init();
    
    // Create texture for rendering
    Image screenImage = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
    Texture2D screenTexture = LoadTextureFromImage(screenImage);
    UnloadImage(screenImage);
    
    // Load ROM
    uint16_t startAddr = 0x0000;
    if (!loader.loadROM(argv[1], *memory, startAddr)) {
        std::cerr << "Failed to load ROM" << std::endl;
        UnloadTexture(screenTexture);
        delete cpu;
        delete sound;
        delete input;
        delete graphics;
        delete memory;
        CloseWindow();
        return 1;
    }
    
    // Reset CPU to start address
    cpu->reset(startAddr);
    
    std::cout << std::endl << "Starting Execution" << std::endl << std::endl;
    
    // Pixel buffer for texture update
    Color* pixels = new Color[SCREEN_WIDTH * SCREEN_HEIGHT];
    
    // Timing variables
    double cycleAccumulator = 0.0;
    
    // Main loop
    while (!WindowShouldClose()) {
        // Get delta time (seconds since last frame)
        float deltaTime = GetFrameTime();
        
        // Calculate how many cycles to run based on elapsed time
        cycleAccumulator += deltaTime * CPU_CLOCK_HZ;
        int cyclesToRun = (int)cycleAccumulator;
        cycleAccumulator -= cyclesToRun;
        
        // Update input state (reads keyboard, writes to I/O ports)
        input->update();
        
        // Clear VBLANK at start of frame
        graphics->setVBlank(false);
        
        // Calculate when VBLANK should occur (after ~90% of cycles = visible portion)
        int vblankCycle = (cyclesToRun * 9) / 10;
        
        // Execute CPU cycles for this frame
        for (int i = 0; i < cyclesToRun; i++) {
            // Set VBLANK flag when we reach the blanking period
            if (i == vblankCycle) {
                graphics->setVBlank(true);
            }
            cpu->step();
        }
        
        // Update sound system
        sound->update(deltaTime);
        
        // Copy graphics buffer to pixel array
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                Color32 c = graphics->getPixelColor(x, y);
                pixels[y * SCREEN_WIDTH + x] = { c.r, c.g, c.b, c.a };
            }
        }
        
        // Update texture with new pixel data
        UpdateTexture(screenTexture, pixels);
        
        // Render
        BeginDrawing();
        ClearBackground(BLACK);
        
        // Draw scaled screen texture
        DrawTextureEx(screenTexture, { 0, 0 }, 0.0f, (float)WINDOW_SCALE, WHITE);
        
        EndDrawing();
    }
    
    std::cout << std::endl << "Execution Finished" << std::endl << std::endl;
    
    // Dump final CPU state
    cpu->dumpState();
    
    // Cleanup
    delete[] pixels;
    UnloadTexture(screenTexture);
    delete cpu;
    delete sound;
    delete input;
    delete graphics;
    delete memory;
    CloseWindow();
    
    return 0;
}
