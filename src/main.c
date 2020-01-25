#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <pspctrl.h>

#include <pspgu.h>
#include <pspgum.h>

#include "include/SDL.h"

/* Define the module info section */
PSP_MODULE_INFO("IRONGB", 0, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_MAX();
/* Define printf, just to make typing easier */
//#define printf	pspDebugScreenPrintf

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
#define DMA 0xFF46
#define BGP 0xFF47
#define OBP0 0xFF48
#define OBP1 0xFF49
#define WY 0xFF4A
#define WX 0xFF4B
#define IE 0xFFFF

#define bool uint8_t
#define true 1
#define false 0

#define ZOOM 1

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

struct {
  uint8_t cartridge_type;
  uint8_t rom_bank;
  uint8_t eram_bank;
  bool eram_enable;

  FILE *rom;
} cartridge;

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

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();

	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

void DMGReset() {
  fread(&ram, 0x8000, 1, cartridge.rom);

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

  rb.af = 0x01B0;
  rb.bc = 0x13;
  rb.de = 0xD8;
  rb.hl = 0x14D;
  rb.sp = 0xFFFE;

  ram[P1] = 0xFF;
  ram[DIV] = 0;
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

uint8_t drawWindow(SDL_Surface *draw_surface, uint8_t x, uint8_t y) {
  if ((ram[LCDC] & 0b1) && (ram[LCDC] & 0b100000)) {
    uint8_t wx = ram[WX];
    uint8_t wy = ram[WY];

    if (wy > 143 || wx > 166) return 0;

    int16_t sx = x - (wx - 7);
    int16_t sy = y - wy;

    if (sx < 0 || sy < 0) return 0;

    uint64_t i = 32*(sy/8) + sx/8;
    uint8_t tile = ram[(ram[LCDC] & 0b1000000 ? 0x9C00 : 0x9800) + i];
    uint8_t tileset = ram[LCDC] & 0b10000;
    if (!tileset) {
      tile += 128;
    }

    uint8_t low = ram[(tileset ? 0x8000 : 0x8800) + tile*16 + 2*(sy % 8)];
    uint8_t high = ram[(tileset ? 0x8001 : 0x8801) + tile*16 + 2*(sy % 8)];

    uint8_t tx = sx % 8;
    uint8_t pixel = (((low << tx) & 0xFF) >> 7) | ((((high << tx) & 0xFF) >> 7) << 1);
    uint32_t color = 0xFFFFFF - (((ram[BGP] >> (2*pixel)) & 0b11) * 5592405);

    uint32_t *pixels = (uint32_t *)draw_surface->pixels;
    pixels[(y*draw_surface->w) + x] = color;

    return pixel;
  }
  else {
    return 0;
  }
}

uint8_t drawBG(SDL_Surface *draw_surface, uint8_t x, uint8_t y) {
  if (ram[LCDC] & 0b1) {
    uint8_t sx = x + ram[SCX];
    //if (sx > 160) sx = 160;
    uint8_t sy = y + ram[SCY];
    //if (sy > 144) sy = 144;

    uint64_t i = 32*(sy/8) + sx/8;
    uint8_t tile = ram[(ram[LCDC] & 0b1000 ? 0x9C00 : 0x9800) + i];
    uint8_t tileset = ram[LCDC] & 0b10000;
    if (!tileset) {
      tile += 128;
    }

    uint8_t low = ram[(tileset ? 0x8000 : 0x8800) + tile*16 + 2*(sy % 8)];
    uint8_t high = ram[(tileset ? 0x8001 : 0x8801) + tile*16 + 2*(sy % 8)];

    uint8_t tx = sx % 8;
    uint8_t pixel = (((low << tx) & 0xFF) >> 7) | ((((high << tx) & 0xFF) >> 7) << 1);
    uint32_t color = 0xFFFFFF - (((ram[BGP] >> (2*pixel)) & 0b11) * 5592405);

    uint32_t *pixels = (uint32_t *)draw_surface->pixels;
    pixels[(y*draw_surface->w) + x] = color;

    return pixel;
  }
  else {
    uint32_t *pixels = (uint32_t *)draw_surface->pixels;
    pixels[(y*draw_surface->w) + x] = 0xFFFFFF;

    return 0;
  }
}

void printHeader() {
  fseek(cartridge.rom, 0x0134, SEEK_SET);

  unsigned char buffer[20] = {0};
  fread(buffer, 16, 1, cartridge.rom);

  printf("ROM Loaded: %s\n", buffer);

  fseek(cartridge.rom, 0x0147, SEEK_SET);
  fread(buffer, 1, 1, cartridge.rom);
  cartridge.cartridge_type = buffer[0];
  printf("Cartridge Type: ");
  switch (buffer[0]) {
    case 0x00:
      printf("ROM Only");
      break;
    case 0x01:
      printf("MBC1");
      break;
    case 0x02:
      printf("MBC1 + RAM");
      break;
    case 0x03:
      printf("MBC1 + RAM + Battery");
      break;
    case 0x05:
      printf("MBC2");
      break;
    case 0x06:
      printf("MBC2 + RAM + Battery");
      break;
    case 0x08:
      printf("ROM + RAM");
      break;
    case 0x09:
      printf("ROM + RAM + Battery");
      break;
    case 0x0B:
      printf("MMM01");
      break;
    case 0x0C:
      printf("MMM01 + RAM");
      break;
    case 0x0D:
      printf("MMM01 + RAM + Battery");
      break;
    case 0x0F:
      printf("MBC3 + Timer + Battery");
      break;
    case 0x10:
      printf("MBC3 + RAM + Timer + Battery");
      break;
    case 0x11:
      printf("MBC3");
      break;
    case 0x12:
      printf("MBC3 + RAM");
      break;
    case 0x13:
      printf("MBC3 + RAM + Battery");
      break;
    case 0x19:
      printf("MBC5");
      break;
    case 0x1A:
      printf("MBC5 + RAM");
      break;
    case 0x1B:
      printf("MBC5 + RAM + Battery");
      break;
    case 0x1C:
      printf("MBC5 + Rumble");
      break;
    case 0x1D:
      printf("MBC5 + RAM + Rumble");
      break;
    case 0x1E:
      printf("MBC5 + RAM + Battery + Rumble");
      break;
    case 0x20:
      printf("MBC6 + RAM + Battery");
      break;
    case 0x22:
      printf("MBC7 + RAM + Bat. + Accelerometer");
      break;
    case 0xFC:
      printf("POCKET CAMERA");
      break;
    case 0xFD:
      printf("BANDAI TAMA5");
      break;
    case 0xFE:
      printf("HuC3");
      break;
    case 0xFF:
      printf("HuC1 + RAM + Battery");
      break;
    default:
      printf("Unknown");
  }
  printf("\n");

  fread(buffer, 2, 1, cartridge.rom);
  int ram_size;
  switch (buffer[2]) {
    case 1:
      ram_size = 2;
      break;
    case 2:
      ram_size = 8;
      break;
    case 3:
      ram_size = 32;
      break;
    case 4:
      ram_size = 128;
      break;
    case 5:
      ram_size = 64;
      break;
    default:
      ram_size = 0;
  }
  printf("ROM Size: %dKB - RAM Size: %dKB\n", 32 << buffer[1], ram_size);

  rewind(cartridge.rom);
}

void loadCartridge(char *path) {
  cartridge.rom = fopen(path, "rb");
  printHeader();
  cartridge.rom_bank = 1;
  cartridge.eram_bank = 0;
  cartridge.eram_enable = false;

  DMGReset();
}

int main(int argc, char* argv[]) {
	SetupCallbacks();

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

  loadCartridge("tetris.gb");

  SDL_Event e;
  while(1){
    if (BIOS == MAPPED && rb.pc == 0x100) {
      BIOS = UNMAPPED;
      rewind(cartridge.rom);
      fread(&ram, 0x100, 1, cartridge.rom);
    }

    getInput();
    updateRegisters();
    handleInterrupts();
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

          if (ram[STAT] & 0b100000) ram[IF] |= 0b10;
        }
        break;
        //VRAM Access
      case 3:
        if (gpu_clock >= 172) {
          gpu_clock = 0;
          ram[STAT] &= ~0b11;

          if (ram[STAT] & 0b1000) ram[IF] |= 0b10;

          // Render a line
          // TODO: Better pipeline for sprite priority drawing
          // TODO: Slow to go over every sprite for every pixel
          uint8_t y = ram[LY];
          for (uint8_t x = 0; x < 160; x++) {
            int bg_pixel = drawBG(draw_surface, x, y);
            bg_pixel |= drawWindow(draw_surface, x, y);
            if (ram[LCDC] & 0b10) {
              for (int s = 0; s < 40; s++) {
                int16_t sy = ram[0xFE00 + 4*s] - 16;
                int16_t sx = ram[0xFE00 + 4*s + 1] - 8;
                if ((int)y >= sy && (int)y < sy + 8 &&
                    (int)x  >= sx && (int)x < sx + 8) {
                  uint8_t tx = x - sx;
                  uint8_t ty = y - sy;

                  if (ram[0xFE00 + 4*s + 3] & 0b100000) {
                    tx = 7 - tx;
                  }
                  if (ram[0xFE00 + 4*s + 3] & 0b1000000) {
                    ty = 7 - ty;
                  }

                  uint8_t tile = ram[0xFE00 + 4*s + 2];

                  uint8_t low = ram[0x8000 + tile*16 + 2*(ty % 8)];
                  uint8_t high = ram[0x8001 + tile*16 + 2*(ty % 8)];

                  uint8_t pixel = (((low << tx) & 0xFF) >> 7) | ((((high << tx) & 0xFF) >> 7) << 1);
                  if (pixel) {
                    bool priority = ram[0xFE00 + 4*s + 3] & 0b10000000;
                    if (!priority || (priority && !bg_pixel)) {
                      uint8_t palette = (ram[0xFE00 + 4*s + 3] & 0b10000) ? ram[OBP1] : ram[OBP0];
                      uint32_t color = 0xFFFFFF - (((palette >> (2*pixel)) & 0b11) * 5592405);

                      Uint32 *pixels = (Uint32 *)draw_surface->pixels;
                      pixels[(y*draw_surface->w) + x] = color;
                    }
                    break;
                  }
                }
              }
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
#ifdef SHOW_TILESET
            for (int tile = 0; tile < 384; tile++) {
              int tx = 8*(tile % (ZOOM*20));
              int ty = 8*(tile / (ZOOM*20));
              for (int y = 0; y < 8; y++) {
                uint8_t low = ram[0x8000 + 16*tile + 2*y];
                uint8_t high = ram[0x8001 + 16*tile + 2*y];

                for (int x = 0; x < 8; x++) {
                  uint8_t pixel = (((low << x) & 0xFF) >> 7) | ((((high << x) & 0xFF) >> 7) << 1);
                  uint32_t color = 0xFFFFFF - (((ram[BGP] >> (2*pixel)) & 0b11) * 5592405);

                  Uint32 *pixels = (Uint32 *)screen_surface->pixels;
                  pixels[((ty+y)*screen_surface->w) + (tx+x)] = color;
                }
              }
            }
#endif
            SDL_UpdateWindowSurface(window);

            ram[IF] |= 0b1;
          }
          else {
            ram[STAT] |= 0b10;

            if (ram[STAT] & 0b1000) ram[IF] |= 0b10;
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

            if (ram[STAT] & 0b10000) ram[IF] |= 0b10;

            //mem_dump(0xFE00, 0xFE9F);
            //puts("");

            ram[LY] = 0;

            while(SDL_PollEvent(&e)){
              if(e.type == SDL_QUIT)
                exit(0);
            }
          }
        }
        break;
    }
  }

  fclose(cartridge.rom);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
