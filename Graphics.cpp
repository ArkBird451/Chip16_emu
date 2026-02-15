#include "Graphics.h"

// Default Chip16 palette
static const Color32 DEFAULT_PALETTE[PALETTE_SIZE] = {
    {0x00, 0x00, 0x00, 0x00},  // 0x0: Black (transparent in FG)
    {0x00, 0x00, 0x00, 0xFF},  // 0x1: Black
    {0x88, 0x88, 0x88, 0xFF},  // 0x2: Gray
    {0xBF, 0x39, 0x32, 0xFF},  // 0x3: Red
    {0xDE, 0x7A, 0xAE, 0xFF},  // 0x4: Pink
    {0x4C, 0x3D, 0x21, 0xFF},  // 0x5: Dark brown
    {0x90, 0x5F, 0x25, 0xFF},  // 0x6: Brown
    {0xE4, 0x94, 0x52, 0xFF},  // 0x7: Orange
    {0xEA, 0xD9, 0x79, 0xFF},  // 0x8: Yellow
    {0x53, 0x7A, 0x3B, 0xFF},  // 0x9: Green
    {0xAB, 0xD5, 0x4A, 0xFF},  // 0xA: Light green
    {0x25, 0x2E, 0x38, 0xFF},  // 0xB: Dark blue
    {0x00, 0x46, 0x7F, 0xFF},  // 0xC: Blue
    {0x68, 0xAB, 0xCC, 0xFF},  // 0xD: Light blue
    {0xBC, 0xDE, 0xE4, 0xFF},  // 0xE: Sky blue
    {0xFF, 0xFF, 0xFF, 0xFF}   // 0xF: White
};

// Constructor
Graphics::Graphics() {
    reset();
}

// Reset graphics to initial state
void Graphics::reset() {
    // Clear foreground buffer
    memset(foreground, 0, sizeof(foreground));

    // Reset background to black
    background = 0;

    // Reset sprite state
    spriteWidth = 0;
    spriteHeight = 0;
    hFlip = false;
    vFlip = false;

    // Load default palette
    loadDefaultPalette();

    // Clear VBLANK flag
    vblankFlag = false;
}

// Clear foreground layer
void Graphics::clearForeground() {
    memset(foreground, 0, sizeof(foreground));
}

// Set background color
void Graphics::setBackgroundColor(uint8_t colorIndex) {
    background = colorIndex & 0x0F;  // 4-bit color
}

// Set sprite dimensions
void Graphics::setSpriteSize(uint8_t width, uint8_t height) {
    spriteWidth = width;   // Width in bytes
    spriteHeight = height; // Height in pixels
}

// Set flip flags
void Graphics::setFlipFlags(bool horizontal, bool vertical) {
    hFlip = horizontal;
    vFlip = vertical;
}

// Draw sprite to foreground buffer
bool Graphics::drawSprite(int16_t x, int16_t y, const uint8_t* spriteData, uint8_t* memory) {
    bool collision = false;

    return collision;
}

// Load default Chip16 palette
void Graphics::loadDefaultPalette() {
    for (int i = 0; i < PALETTE_SIZE; i++) {
        palette[i] = DEFAULT_PALETTE[i];
    }
}

// Load custom palette from memory
void Graphics::loadCustomPalette(const uint8_t* paletteData) {
    // Palette format: RGB (3 bytes per color, 48 bytes total)
    for (int i = 0; i < PALETTE_SIZE; i++) {
        palette[i].r = paletteData[i * 3 + 0];
        palette[i].g = paletteData[i * 3 + 1];
        palette[i].b = paletteData[i * 3 + 2];
        palette[i].a = (i == 0) ? 0x00 : 0xFF;  // Index 0 is transparent
    }
}

// Get pixel color at screen coordinates
Color32 Graphics::getPixelColor(int x, int y) const {
    if (x < 0 || x >= GFX_WIDTH || y < 0 || y >= GFX_HEIGHT) {
        return { 0, 0, 0, 0xFF };  // Black for out of bounds
    }

    // Calculate byte position (2 pixels per byte)
    int byteIndex = (y * GFX_WIDTH + x) / 2;
    uint8_t byte = foreground[byteIndex];

    // Extract 4-bit color index
    uint8_t colorIndex;
    if (x & 1) {
        // Odd pixel (high nibble)
        colorIndex = (byte >> 4) & 0x0F;
    }
    else {
        // Even Pixel (low nibble)
        colorIndex = byte & 0x0F;
    }

    // If foreground is transparent (0), use background color
    if (colorIndex == 0) {
        colorIndex = background;
    }

    return palette[colorIndex];
}
