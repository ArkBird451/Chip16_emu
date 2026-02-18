#pragma once

#include <cstdint>
#include "Memory.h"

// Controller button bit masks
#define BTN_UP      0x01
#define BTN_DOWN    0x02
#define BTN_LEFT    0x04
#define BTN_RIGHT   0x08
#define BTN_SELECT  0x10
#define BTN_START   0x20
#define BTN_A       0x40
#define BTN_B       0x80

// I/O port addresses
#define IO_CONTROLLER1  0xFFF0
#define IO_CONTROLLER2  0xFFF2

class Input {
private:
    Memory& memory;
    uint16_t controller1;
    uint16_t controller2;

public:
    Input(Memory& mem);

    // Update controller state from keyboard
    void update();

    // Get controller state
    uint16_t getController1() const { return controller1; }
    uint16_t getController2() const { return controller2; }

    // Reset input state
    void reset();
};
