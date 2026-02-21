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

// Rendering always runs at 60fps for responsive input
// Game speed is controlled by VBLANK_DIVISOR:
//   1 = 60fps game speed (full speed)
//   2 = 30fps game speed (half speed)
//   3 = 20fps game speed (third speed)
#define TARGET_FPS 60
#define VBLANK_DIVISOR 2

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return 1;
    }
    
    // Initialize window
    InitWindow(SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE, "Chip16 Emulator");
    SetTargetFPS(TARGET_FPS);
    
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

    // Pixel buffer for texture update
    Color* pixels = new Color[SCREEN_WIDTH * SCREEN_HEIGHT];

    double cycleAccumulator = 0.0;
    int frameCounter = 0;

    // Main loop
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        frameCounter++;

        // Calculate how many CPU cycles to run this frame
        cycleAccumulator += deltaTime * CPU_CLOCK_HZ;
        int cyclesToRun = (int)cycleAccumulator;
        cycleAccumulator -= cyclesToRun;

        // Update input state every frame so it stays responsive
        input->update();

        // Only fire VBLANK every VBLANK_DIVISOR frames to control game speed
        bool fireVBlank = (frameCounter % VBLANK_DIVISOR == 0);

        // Clear VBLANK at start of frame
        graphics->setVBlank(false);

        // VBLANK occurs at 90% through the frame (visible scanlines done)
        int vblankCycle = (cyclesToRun * 9) / 10;

        // Execute CPU cycles for this frame
        for (int i = 0; i < cyclesToRun; i++) {
            if (fireVBlank && i == vblankCycle) {
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
