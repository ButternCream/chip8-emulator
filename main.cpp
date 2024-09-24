#include "chip8.hpp"
#include <SDL2/SDL.h>
#include <string>

using namespace std;

chip8 myChip8;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
bool quit = false;

const SDL_Scancode keymap[16] = {
    SDL_SCANCODE_X, // 0
    SDL_SCANCODE_1, // 1
    SDL_SCANCODE_2, // 2
    SDL_SCANCODE_3, // 3
    SDL_SCANCODE_Q, // 4
    SDL_SCANCODE_W, // 5
    SDL_SCANCODE_E, // 6
    SDL_SCANCODE_A, // 7
    SDL_SCANCODE_S, // 8
    SDL_SCANCODE_D, // 9
    SDL_SCANCODE_Z, // A
    SDL_SCANCODE_C, // B
    SDL_SCANCODE_4, // C
    SDL_SCANCODE_R, // D
    SDL_SCANCODE_F, // E
    SDL_SCANCODE_V  // F
};

void setupGraphics()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("There was an error setting up SDL. %s\n", SDL_GetError());
        return;
    }

    window = SDL_CreateWindow("CHIP8 Emulator",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              800,
                              600,
                              SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("SDL window failed to initialize. %s\n", SDL_GetError());
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        printf("Renderer failed to initialize. %s\n", SDL_GetError());
        return;
    }
}

void draw()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            if (gfx[y * WIDTH + x])
            {
                SDL_Rect pixel = {x * SCALE, y * SCALE, SCALE, SCALE};
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
}

void handleQuitEvent()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            quit = true;
            break;
        }
    }
}

void updateKeys()
{
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < 16; i++)
    {
        bool newState = state[keymap[i]] ? 1 : 0;
        if (newState != key[i])
        {
            key[i] = newState;
        }
    }
}

void teardownGraphics()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage %s <path-to-file>\n", argv[0]);
        return 1;
    }

    string gamePath = argv[1];

    setupGraphics();

    myChip8.initialize();
    myChip8.loadGame(gamePath);

    while (!quit)
    {
        myChip8.emulateCycle();

        if (drawFlag)
        {
            // draw
            draw();
            drawFlag = false;
        }

        handleQuitEvent();
        updateKeys();
    }

    teardownGraphics();

    return 0;
}