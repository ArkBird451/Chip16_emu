#pragma once

#include <cstdint>
#include <cstring>

// Chip16 Graphics specifications
#define GFX_WIDTH  320
#define GFX_HEIGHT 240
#define PALETTE_SIZE 16

// Default Chip16 palette (RGB values)
struct Color32 {
    uint8_t r, g, b, a;
};

// Graphics state and display buffer
class Graphics {
private:
    // Display buffers (4-bit indexed color, 2 pixels per byte)
    uint8_t foreground[GFX_WIDTH * GFX_HEIGHT / 2];  // FG layer
    uint8_t background;  // BG is single color index

    // Sprite state registers
    uint8_t spriteWidth;   // Width in bytes (2 pixels per byte)
    uint8_t spriteHeight;  // Height in pixels
    bool hFlip;            // Horizontal flip
    bool vFlip;            // Vertical flip

    // Palette (16 colors, RGBA)
    Color32 palette[PALETTE_SIZE];

    // VBLANK flag
    bool vblankFlag;

public:
    Graphics();

    // Display buffer access
    void clearForeground();
    void setBackgroundColor(uint8_t colorIndex);
    uint8_t getBackgroundColor() const { return background; }

    // Sprite state
    void setSpriteSize(uint8_t width, uint8_t height);
    void setFlipFlags(bool horizontal, bool vertical);

    // Drawing operations
    bool drawSprite(int16_t x, int16_t y, const uint8_t* spriteData, uint8_t* memory);

    // Palette operations
    void loadDefaultPalette();
    void loadCustomPalette(const uint8_t* paletteData);
    const Color32* getPalette() const { return palette; }

    // Get pixel color (for rendering)
    Color32 getPixelColor(int x, int y) const;

    // VBLANK
    void setVBlank(bool flag) { vblankFlag = flag; }
    bool getVBlank() const { return vblankFlag; }

    // Reset
    void reset();
};
