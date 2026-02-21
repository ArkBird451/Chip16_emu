# Chip16 Emulator

A Chip16 emulator written in C++ using Raylib for graphics, input, and audio.

## About

The hardware is fictional but closely mimics the constraints of real retro systems - limited memory, a small color palette, and hardware-synced game logic via VBlank. This emulator implements the full Chip16 specification including all CPU opcodes, sprite rendering with collision detection, two-controller input, and real-time audio synthesis.

## Requirements

- Windows
- Visual Studio
- [Raylib](https://www.raylib.com/)

## Building

Open `Chip16_emu.sln` in Visual Studio and build in Release or Debug mode.

## Usage

```
Chip16_emu.exe <rom_file>
```

Supports both `.c16` (with header) and `.bin` (raw binary) ROM formats.

## Controls

| Key | Action |
|-----|--------|
| Arrow Keys | D-Pad |
| Z | A Button |
| X | B Button |
| Enter / Space | Start |
| Tab | Select |
| W/A/S/D | Controller 2 D-Pad |
| J / K | Controller 2 A / B |

## Configuration

At the top of `Chip16_emu.cpp`:

| Setting | Default | Description |
|---------|---------|-------------|
| `CPU_CLOCK_HZ` | `1000000` | CPU speed (1MHz) |
| `TARGET_FPS` | `60` | Rendering frame rate |
| `VBLANK_DIVISOR` | `2` | Game speed (1 = full, 2 = half, 3 = third) |
| `WINDOW_SCALE` | `3` | Window size multiplier (320x240 base) |

## Architecture

| File | Description |
|------|-------------|
| `CPU.cpp` | Fetch/decode/execute, all opcodes |
| `Memory.cpp` | 64KB RAM, ROM loading |
| `Graphics.cpp` | 320x240 framebuffer, 4-bit color, sprites |
| `Input.cpp` | Keyboard to controller mapping |
| `Sound.cpp` | Real-time audio synthesis, ADSR envelope |
| `ROMLoader.cpp` | Loads `.c16` and `.bin` ROMs |

## Chip16 Specs

- **CPU**: 16-bit, 1MHz
- **Memory**: 64KB
- **Screen**: 320x240, 16 colors (4-bit indexed)
- **Registers**: 16x 16-bit (R0-RF), PC, SP, Flags (C/Z/O/N)
- **Stack**: 512 bytes at 0xFDF0
- **I/O Ports**: 0xFFF0-0xFFF3 (controllers)
