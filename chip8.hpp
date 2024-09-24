#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cstdint>
#include <string>

#define WIDTH 64
#define HEIGHT 32
#define SCALE 12

using namespace std;

unsigned char chip8_fontset[80] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

uint8_t gfx[WIDTH * HEIGHT];
bool drawFlag;

// CHIP-8 Keypad
uint8_t key[16];

class chip8
{
public:
    void initialize();
    void emulateCycle();
    void loadGame(string);

private:
    // Current opcode
    uint16_t opcode;

    // CHIP-8 memory
    static const int MEMORY_SIZE = 4096;
    uint8_t memory[MEMORY_SIZE];

    // CPU registers
    uint8_t V[16];

    // Index register and program counter
    uint16_t I;
    uint16_t pc;

    // Timers
    uint8_t delay_timer;
    uint8_t sound_timer;

    // Stack
    static const int STACK_SIZE = 16;
    uint16_t stack[STACK_SIZE];
    uint16_t sp;

    void executeOpcode(uint16_t opcode);
};
void chip8::loadGame(string filename)
{
    FILE *file = fopen(filename.c_str(), "rb");
    if (file == NULL)
    {
        printf("Error: Unable to open file %s\n", filename);
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // Allocate memory to store the ROM
    unsigned char *buffer = (unsigned char *)malloc(fileSize);
    if (buffer == NULL)
    {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return;
    }

    // Read the file
    size_t result = fread(buffer, 1, fileSize, file);
    if (result != fileSize)
    {
        printf("Error: Failed to read the file\n");
        free(buffer);
        fclose(file);
        return;
    }

    // Check if ROM fits in Chip-8 memory
    if (fileSize > (0xFFF - 0x200))
    {
        printf("Error: ROM too big for CHIP-8 memory\n");
        free(buffer);
        fclose(file);
        return;
    }

    // Load ROM into Chip-8 memory
    for (int i = 0; i < fileSize; ++i)
    {
        memory[i + 0x200] = buffer[i];
    }

    // Clean up
    free(buffer);
    fclose(file);

    // Set program counter to start of ROM
    pc = 0x200;

    printf("ROM loaded successfully\n");
    return;
}

void chip8::initialize()
{
    // Initialize registers and memory once
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    for (int i = 0; i < 80; i++)
    {
        memory[i] = chip8_fontset[i];
    }
}

void chip8::emulateCycle()
{
    // Fetch Opcode
    opcode = memory[pc] << 8 | memory[pc + 1];
    // Execute Opcode
    executeOpcode(opcode);

    // Update timers
    if (delay_timer > 0)
    {
        --delay_timer;
    }
    if (sound_timer > 0)
    {
        if (sound_timer == 1)
        {
            printf("beep!\n");
        }
        --sound_timer;
    }
}

void chip8::executeOpcode(uint16_t opcode)
{
    unsigned int decoded_opcode = opcode & 0xF000;
    switch (decoded_opcode)
    {
    case 0x0000:
        switch (opcode & 0x000F)
        {
        case 0x0000: // 0x00E0: Clears the screen
            memset(gfx, 0, sizeof(gfx));
            drawFlag = true;
            pc += 2;
            break;

        case 0x000E: // 0x00EE: Returns from subroutine
            --sp;
            pc = stack[sp];
            pc += 2;
            break;
        }
        break;
    case 0x1000:
        // Jump to location nnn.
        pc = opcode & 0x0FFF;
        break;

    case 0x2000:
        // The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
        stack[sp] = pc;
        ++sp;
        pc = opcode & 0x0FFF;
        break;
    // 3xkk
    case 0x3000:
        // Skip next instruction if Vx = kk.
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
        {
            pc += 2;
        }
        pc += 2;
        break;
    case 0x4000:
        // Skip next instruction if Vx != kk.
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
        {
            pc += 2;
        }
        pc += 2;
        break;
    case 0x5000:
        // Skip next instruction if Vx = Vy.
        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
        {
            pc += 2;
        }
        pc += 2;
        break;
    case 0x6000:
        // Set Vx = kk.
        V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        pc += 2;
        break;
    case 0x7000:
        // Set Vx = Vx + kk.
        V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        pc += 2;
        break;
    // 0x8xyN
    case 0x8000:
    {
        unsigned int x = (opcode & 0x0F00) >> 8;
        unsigned int y = (opcode & 0x00F0) >> 4;
        switch (opcode & 0x000F)
        {
        case 0x0000:
            // Set Vx = Vy.
            // Stores the value of register Vy in register Vx.
            V[x] = V[y];
            break;
        case 0x0001:
            // Set Vx = Vx OR Vy.
            V[x] |= V[y];
            break;
        case 0x0002:
            // Set Vx = Vx AND Vy.
            V[x] &= V[y];
            break;
        case 0x0003:
            // Set Vx = Vx XOR Vy.
            V[x] ^= V[y];
            break;
        case 0x0004:
            // Set Vx = Vx + Vy, set VF = carry.
            {
                uint16_t sum = V[x] + V[y];
                uint8_t carry = (sum > 0xFF) ? 1 : 0;
                V[x] = sum & 0xFF; // Keep only the lower 8 bits
                V[0xF] = carry;    // Set carry flag last
            }
            break;
        case 0x0005:
            // Set Vx = Vx - Vy, set VF = NOT borrow
            {
                uint8_t vx = V[x];
                uint8_t vy = V[y];
                uint8_t result = vx - vy;
                V[x] = result;               // The result is already wrapped due to uint8_t overflow
                V[0xF] = (vx >= vy) ? 1 : 0; // Set VF to 1 if there's NO borrow, 0 if there is
            }
            break;
        case 0x0006:
            // Set Vx = Vx SHR 1
            {
                uint8_t lsb = V[x] & 0x01;
                V[x] >>= 1;   // Shift Vx right by 1
                V[0xF] = lsb; // Set VF to the least significant bit of Vx
            }
            break;
        case 0x0007:
            V[x] = (V[y] - V[x]) & 0xFF; // Ensure wrap-around
            V[0xF] = (V[y] >= V[x]) ? 1 : 0;
            break;
        case 0x000E:
            // Set Vx = Vx SHL 1.
            uint8_t msb = (V[x] & 0x80) >> 7;
            V[x] <<= 1;
            V[0xF] = msb;
            break;
        }
        pc += 2;
        break;
    }
    case 0x9000:
        // Skip next instruction if Vx != Vy.
        if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
        {
            pc += 2;
        }
        pc += 2;
        break;
    case 0xA000:
        // ANNN: Sets I to address NNN
        I = opcode & 0x0FFF;
        pc += 2;
        break;
    case 0xB000:
        // Jump to location nnn + V0.
        pc = (opcode & 0x0FFF) + V[0]; // nnn + V[0]
        break;
    case 0xC000:
    {
        // Set Vx = random byte AND kk.
        unsigned int random_byte = rand() % 255 + 1;
        V[(opcode & 0x0F00) >> 8] = random_byte & (opcode & 0x00FF);
        pc += 2;
        break;
    }
    case 0xD000:
    {
        // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        unsigned int height = opcode & 0x000F;
        unsigned int x = V[(opcode & 0x0F00) >> 8];
        unsigned int y = V[(opcode & 0x00F0) >> 4];
        unsigned int pixel;

        V[0xF] = 0;
        for (int yline = 0; yline < height; yline++)
        {
            pixel = memory[I + yline];
            for (int xline = 0; xline < 8; xline++)
            {
                if ((pixel & (0x80 >> xline)) != 0)
                {
                    if (gfx[(x + xline + ((y + yline) * 64))] == 1)
                        V[0xF] = 1;
                    gfx[x + xline + ((y + yline) * 64)] ^= 1;
                }
            }
        }
        drawFlag = true;
        pc += 2;
        break;
    }
    case 0xE000:
    {
        switch (opcode & 0x00FF)
        {
        case 0x009E:
            // Skip next instruction if key with the value of Vx is pressed.
            if (key[V[(opcode & 0x0F00) >> 8]] != 0)
            {
                pc += 2;
            }
            pc += 2;
            break;
        case 0x00A1:
            // Skip next instruction if key with the value of Vx is not pressed.
            if (key[V[(opcode & 0x0F00) >> 8]] == 0)
            {
                pc += 2;
            }
            pc += 2;
            break;
        }
        break;
    }
    case 0xF000:
    {
        switch (opcode & 0x00FF)
        {
        case 0x0007:
            // Set Vx = delay timer value.
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            pc += 2;
            break;
        case 0x000A:
        {
            bool keyPressed = false;
            // Wait for a key press, store the value of the key in Vx.
            for (int i = 0; i < 16; i++)
            {
                if (key[i] != 0)
                {
                    V[(opcode & 0x0F00) >> 8] = key[i];
                    keyPressed = true;
                    break;
                }
            }
            if (keyPressed)
                return;
            pc += 2;
            break;
        }
        case 0x0015:
            // Set delay timer = Vx.
            delay_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x0018:
            // Set sound timer = Vx.
            sound_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x001E:
            // Set I = I + Vx.
            I += V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x0029:
            // Set I = location of sprite for digit Vx.
            I = V[(opcode & 0x0F00) >> 8] * 5;
            pc += 2;
            break;
        case 0x0033:
            // Store BCD representation of Vx in memory locations I, I+1, and I+2.
            memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            pc += 2;
            break;
        case 0x0055:
        {
            // Store registers V0 through Vx in memory starting at location I.
            unsigned int n = (opcode & 0x0F00) >> 8;
            for (int i = 0; i <= n; i++)
            {
                memory[I + i] = V[i];
            }
            pc += 2;
            break;
        }
        case 0x0065:
        {
            // Read registers V0 through Vx from memory starting at location I.
            unsigned int n = (opcode & 0x0F00) >> 8;
            for (unsigned int i = 0; i <= n; i++)
            {
                V[i] = memory[I + i];
            }
            pc += 2;
            break;
        }
        default:
            printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
        }
        break;
    }
    default:
        printf("Unknown opcode: 0x%X\n", opcode);
        break;
    }
}