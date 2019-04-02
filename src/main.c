#include <SDL2/SDL.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define COLOR_BLUE "\033[0;34m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_YELLOW "\033[01;33m"
#define COLOR_RESET "\033[0m"

#define P1 0xFF00
#define SB 0xFF01
#define SC 0xFF02
#define DIV 0xFF04
#define TIMA 0xFF05
#define TMA 0xFF06
#define TAC 0xFF07
#define IF 0xFF0F
#define NR10 0xFF10
#define NR11 0xFF11
#define NR12 0xFF12
#define NR14 0xFF14
#define NR21 0xFF16
#define NR22 0xFF17
#define NR24 0xFF19
#define NR30 0xFF1A
#define NR31 0xFF1B
#define NR32 0xFF1C
#define NR33 0xFF1E
#define NR41 0xFF20
#define NR42 0xFF21
#define NR43 0xFF22
#define NR44 0xFF23
#define NR50 0xFF24
#define NR51 0xFF25
#define NR52 0xFF26
#define LCDC 0xFF40
#define STAT 0xFF41
#define SCY 0xFF42
#define SCX 0xFF43
#define LY 0xFF44
#define LYC 0xFF45
#define BGP 0xFF47
#define OBP0 0xFF48
#define OBP1 0xFF49
#define WY 0xFF4A
#define WX 0xFF4B
#define IE 0xFFFF

#define bool uint8_t
#define true 1
#define false 0

#define ZOOM 3

struct {
    struct {
        union {
            struct {
                uint8_t f;
                uint8_t a;
            };
            uint16_t af;
        };
    };

    struct {
        union {
            struct {
                uint8_t c;
                uint8_t b;
            };
            uint16_t bc;
        };
    };

    struct {
        union {
            struct {
                uint8_t e;
                uint8_t d;
            };
            uint16_t de;
        };
    };

    struct {
        union {
            struct {
                uint8_t l;
                uint8_t h;
            };
            uint16_t hl;
        };
    };

    uint16_t sp;
    uint16_t pc;
} rb;
uint64_t cpu_clock;
uint64_t gpu_clock;
uint8_t ram[0x10000];
bool IME;
enum {
    MAPPED,
    UNMAPPED,
} BIOS;
bool debug = false;

#include "mmu.c"
#include "cpu.c"

void DMGReset(FILE *rom) {
    fread(&ram, 0x8000, 1, rom);

    FILE *bios = fopen("bios.gb", "rb");
    if (!bios) {
        printf("No BIOS found\n");
        BIOS = UNMAPPED;
        rb.pc = 0x100;
    }
    else {
        printf("Loaded BIOS\n");
        BIOS = MAPPED;
        rb.pc = 0x0;

        fread(&ram, 256, 1, bios);
        fclose(bios);
    }

    cpu_clock = 0;
    gpu_clock = 0;
    IME = false;

    rb.f = 0xB0;
    rb.bc = 0x13;
    rb.de = 0xD8;
    rb.hl = 0x14D;
    rb.sp = 0xFFFE;

    ram[P1] = 0xFF;
    ram[TIMA] = 0;
    ram[TMA] = 0;
    ram[TAC] = 0;
    ram[NR10] = 0x80;
    ram[NR11] = 0xBF;
    ram[NR12] = 0xF3;
    ram[NR14] = 0xBF;
    ram[NR21] = 0x3F;
    ram[NR22] = 0x00;
    ram[NR24] = 0xBF;
    ram[NR30] = 0x7F;
    ram[NR31] = 0xFF;
    ram[NR32] = 0x9F;
    ram[NR33] = 0xBF;
    ram[NR41] = 0xFF;
    ram[NR42] = 0x00;
    ram[NR43] = 0x00;
    ram[NR44] = 0xBF;
    ram[NR50] = 0x77;
    ram[NR51] = 0xF3;
    ram[NR52] = 0xF1;
    ram[LCDC] = 0x91;
    ram[STAT] = 0x2;
    ram[SCY] = 0x00;
    ram[SCX] = 0x00;
    ram[LY] = 0x00;
    ram[LYC] = 0x00;
    ram[BGP] = 0xFC;
    ram[OBP0] = 0xFF;
    ram[OBP1] = 0xFF;
    ram[WY] = 0x00;
    ram[WX] = 0x00;
    ram[IE] = 0x00;
}

int main(int argc, char* argv[]) {
    SDL_Window *window = NULL;

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    window = SDL_CreateWindow("iron-gb", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ZOOM*160, ZOOM*144, SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
    if(window == NULL){
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    if (!window) exit(1);

    SDL_Surface *screen_surface = SDL_GetWindowSurface(window);
    SDL_Surface *draw_surface = SDL_CreateRGBSurface(0, 160, 144, screen_surface->format->BitsPerPixel,
                                                     screen_surface->format->Rmask, screen_surface->format->Gmask,
                                                     screen_surface->format->Bmask, screen_surface->format->Amask);

    FILE *rom = fopen(argv[1], "rb");
    DMGReset(rom);

    SDL_Event e;
    while(1){
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT)
                exit(0);
        }

        if (BIOS == MAPPED && rb.pc == 0x100) {
            BIOS = UNMAPPED;
            fread(&ram, 0x100, 1, rom);
        }

        getInput();
        handleInterrupts();
        updateRegisters();
        nextInstruction();

        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D]) {
            debug = true;
        }
        if (debug) {
            char c = getchar();
            if (c == 'c') debug = false;
        }

        int mode = ram[STAT] & 0b11;
        switch (mode) {
            //OAM Access
            case 2:
                if (gpu_clock >= 80) {
                    gpu_clock = 0;
                    ram[STAT] &= ~0b11;
                    ram[STAT] |= 0b11;
                }
                break;
                //VRAM Access
            case 3:
                if (gpu_clock >= 172) {
                    gpu_clock = 0;
                    ram[STAT] &= ~0b11;

                    if (ram[STAT] & 0b1000) ram[IF] |= 0b10;

                    // Render a line
                    uint8_t y = ram[LY];
                    if (ram[LCDC] & 0b1) {
                        for (int x = 0; x < 160; x++) {
                            uint8_t sx = x /*+ ram[SCX]*/; //TODO: Fix scrolling
                            uint8_t sy = y /*+ ram[SCY]*/;

                            uint64_t i = 32*(sy/8) + sx/8;
                            uint8_t tile = ram[(ram[LCDC] & 0b1000 ? 0x9C00 : 0x9800) + i];

                            uint8_t low = ram[(ram[LCDC] & 0b10000 ? 0x8000 : 0x8800) + tile*16 + 2*(sy % 8)];
                            uint8_t high = ram[(ram[LCDC] & 0b10000 ? 0x8001 : 0x8801) + tile*16 + 2*(sy % 8)];

                            uint8_t tx = sx % 8;
                            uint8_t pixel = (((low << tx) & 0xFF) >> 7) | ((((high << tx) & 0xFF) >> 7) << 1);
                            uint32_t color = 0xFFFFFF - (((ram[BGP] >> (2*pixel)) & 0b11) * 5592405);

                            Uint32 *pixels = (Uint32 *)draw_surface->pixels;
                            pixels[(y*draw_surface->w) + x] = color;
                        }
                    }
                }
                break;
                //HBlank
            case 0:
                if (gpu_clock >= 204) {
                    gpu_clock = 0;
                    ram[LY]++;

                    ram[STAT] &= ~0b11;
                    if (ram[LY] > 143) {
                        ram[STAT] |= 0b01;

                        if (ram[STAT] & 0b10000) ram[IF] |= 0b10;

                        if (!(ram[LCDC] & 0b10000000)) {
                            SDL_FillRect(draw_surface, NULL, 0xFFFFFF);
                        }
                        SDL_BlitScaled(draw_surface, NULL, screen_surface, NULL);
                        SDL_UpdateWindowSurface(window);

                        ram[IF] |= 0b1;
                    }
                    else {
                        ram[STAT] |= 0b10;

                        if (ram[STAT] & 0b100000) ram[IF] |= 0b10;
                    }
                }
                break;
                //VBlank
            case 1:
                if (gpu_clock >= 456) {
                    gpu_clock = 0;
                    ram[LY]++;

                    if (ram[LY] > 153) {
                        ram[STAT] &= ~0b11;
                        ram[STAT] |= 0b10;

                        if (ram[STAT] & 0b100000) ram[IF] |= 0b10;

                        ram[LY] = 0;
                    }
                }
                break;
        }
    }

    fclose(rom);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
