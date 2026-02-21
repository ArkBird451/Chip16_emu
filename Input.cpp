#include "Input.h"
#include "raylib.h"

// Constructor
Input::Input(Memory& mem) : memory(mem), controller1(0), controller2(0) {
}

// Reset input state
void Input::reset() {
    controller1 = 0;
    controller2 = 0;
    memory.writeWord(IO_CONTROLLER1, 0);
    memory.writeWord(IO_CONTROLLER2, 0);
}

// Update controller state from keyboard input
void Input::update() {
    controller1 = 0;
    controller2 = 0;

    // Controller 1 - Arrow keys + Z/X for A/B, ENTER/SPACE for START/SELECT
    if (IsKeyDown(KEY_UP))      controller1 |= BTN_UP;
    if (IsKeyDown(KEY_DOWN))    controller1 |= BTN_DOWN;
    if (IsKeyDown(KEY_LEFT))    controller1 |= BTN_LEFT;
    if (IsKeyDown(KEY_RIGHT))   controller1 |= BTN_RIGHT;
    if (IsKeyDown(KEY_TAB))     controller1 |= BTN_SELECT;
    if (IsKeyDown(KEY_ENTER))   controller1 |= BTN_START;
    if (IsKeyDown(KEY_SPACE))   controller1 |= BTN_START;  // Alternative for START
    if (IsKeyDown(KEY_Z))       controller1 |= BTN_A;
    if (IsKeyDown(KEY_X))       controller1 |= BTN_B;

    // Controller 2 - WASD + J/K/U/I
    if (IsKeyDown(KEY_W))       controller2 |= BTN_UP;
    if (IsKeyDown(KEY_S))       controller2 |= BTN_DOWN;
    if (IsKeyDown(KEY_A))       controller2 |= BTN_LEFT;
    if (IsKeyDown(KEY_D))       controller2 |= BTN_RIGHT;
    if (IsKeyDown(KEY_U))       controller2 |= BTN_SELECT;
    if (IsKeyDown(KEY_I))       controller2 |= BTN_START;
    if (IsKeyDown(KEY_J))       controller2 |= BTN_A;
    if (IsKeyDown(KEY_K))       controller2 |= BTN_B;

    // Write to I/O ports in memory
    memory.writeWord(IO_CONTROLLER1, controller1);
    memory.writeWord(IO_CONTROLLER2, controller2);
}
