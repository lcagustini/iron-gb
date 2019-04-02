#include <SDL2/SDL.h>

#include <unistd.h>
#include <time.h>

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

void writeByte(uint16_t addr, uint8_t value) {
    //if (addr == LCDC) debug = true;

    switch (addr) {
        case DIV:
        case LY:
            ram[addr] = 0;
            break;
        default:
            ram[addr] = value;
    }

    if (addr >= 0xE000 && addr < 0xFE00) {
        ram[addr-0x2000] = value;
    }
    else if (addr >= 0xC000 && addr < 0xDE00) {
        ram[addr+0x2000] = value;
    }
}

void nextInstruction() {
    if (debug) {
        printf(COLOR_YELLOW "A:" COLOR_RESET " %02X " COLOR_YELLOW "F:" COLOR_RESET " %02X " COLOR_YELLOW "AF:" COLOR_RESET " %04X " COLOR_BLUE "(", rb.a, rb.f, rb.af);

        rb.f & 0b10000000 ? printf(COLOR_GREEN "Z") : printf(COLOR_RED "-");
        rb.f & 0b01000000 ? printf(COLOR_GREEN "N") : printf(COLOR_RED "-");
        rb.f & 0b00100000 ? printf(COLOR_GREEN "H") : printf(COLOR_RED "-");
        rb.f & 0b00010000 ? printf(COLOR_GREEN "C") : printf(COLOR_RED "-");
        printf(COLOR_BLUE ")\n");

        printf(COLOR_YELLOW "B:" COLOR_RESET " %02X " COLOR_YELLOW "C:" COLOR_RESET " %02X " COLOR_YELLOW "BC:" COLOR_RESET " %04X\n", rb.b, rb.c, rb.bc);
        printf(COLOR_YELLOW "D:" COLOR_RESET " %02X " COLOR_YELLOW "E:" COLOR_RESET " %02X " COLOR_YELLOW "DE:" COLOR_RESET " %04X\n", rb.d, rb.e, rb.de);
        printf(COLOR_YELLOW "H:" COLOR_RESET " %02X " COLOR_YELLOW "L:" COLOR_RESET " %02X " COLOR_YELLOW "HL:" COLOR_RESET " %04X\n", rb.h, rb.l, rb.hl);
        printf(COLOR_YELLOW "SP:" COLOR_RESET " %04X " COLOR_YELLOW "PC:" COLOR_RESET " %04X " COLOR_BLUE "-> ", rb.sp, rb.pc);
    }

    rb.f &= 0xF0;

    uint8_t time = 0;
    uint8_t byte = ram[rb.pc];
    switch (byte) {
        case 0x00:
            {
                if (debug) {
                    printf("NOP");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x01:
            {
                uint16_t imm = ram[rb.pc+1] | (ram[rb.pc+2] << 8);

                rb.bc = imm;

                if (debug) {
                    printf("LD BC, 0x%04X", imm);
                }
                rb.pc += 3;

                time = 12;
            }
            break;
        case 0x02:
            {
                writeByte(rb.bc, rb.a);

                if (debug) {
                    printf("LD (BC), A");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x03:
            {
                rb.bc++;

                if (debug) {
                    printf("INC BC");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x04:
            {
                rb.b++;

                if (rb.b == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                // TODO: half-carry flag

                if (debug) {
                    printf("INC B");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x05:
            {
                rb.b--;

                if (rb.b == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                // TODO: half-carry flag

                if (debug) {
                    printf("DEC B");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x06:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.b = imm;

                if (debug) {
                    printf("LD B, 0x%02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0x07:
            {
                bool carry = rb.c & 0b10000000;

                rb.a <<= 1;
                rb.a |= (carry >> 7);
                if (carry) rb.f |= 0b00010000;
                else rb.f &= 0b11101111;

                rb.f &= 0b00011111;

                if (debug) {
                    printf("RLCA");
                }

                rb.pc++;

                time = 4;
            }
            break;
        case 0x09:
            {
                uint16_t prev_hl = rb.hl;
                uint16_t prev_bc = rb.bc;
                rb.hl = rb.hl + rb.bc;

                rb.f &= 0b10111111;
                if ((rb.hl & 0xF) + (rb.bc & 0xF) > 0xF) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (rb.hl < prev_hl || rb.hl < prev_bc) rb.f |= 0b00010000;
                else rb.f &= ~0b00010000;

                if (debug) {
                    printf("ADD HL, BC");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x0A:
            {
                rb.a = ram[rb.bc];

                if (debug) {
                    printf("LD A, (BC)");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x0B:
            {
                rb.bc--;

                if (debug) {
                    printf("DEC BC");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x0C:
            {
                rb.c++;

                if (rb.c == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                // TODO: half-carry flag

                if (debug) {
                    printf("INC C");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x0D:
            {
                rb.c--;

                if (rb.c == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                // TODO: half-carry flag

                if (debug) {
                    printf("DEC C");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x0E:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.c = imm;

                if (debug) {
                    printf("LD C, 0x%02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0x11:
            {
                uint16_t imm = ram[rb.pc+1] | (ram[rb.pc+2] << 8);

                rb.de = imm;

                if (debug) {
                    printf("LD DE, 0x%04X", imm);
                }
                rb.pc += 3;

                time = 12;
            }
            break;
        case 0x12:
            {
                writeByte(rb.de, rb.a);

                if (debug) {
                    printf("LD (DE), A");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x13:
            {
                rb.de++;

                if (debug) {
                    printf("INC DE");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x15:
            {
                rb.d--;

                if (rb.d == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                // TODO: half-carry flag

                if (debug) {
                    printf("DEC D");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x16:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.d = imm;

                if (debug) {
                    printf("LD D, 0x%02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0x17:
            {
                bool carry = rb.a & 0b10000000;

                rb.a <<= 1;
                rb.a |= (rb.f >> 4) & 1;
                if (carry) rb.f |= 0b00010000;
                else rb.f &= 0b11101111;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10011111;

                if (debug) {
                    printf("RL A");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x18:
            {
                int8_t imm = ram[rb.pc+1];

                if (imm >= 0) rb.pc += imm;
                else rb.pc -= (uint8_t) (-imm);

                if (debug) {
                    printf("JR %s0x%02X", imm < 0 ? "-" : "", imm < 0 ? -imm : imm);
                }
                rb.pc += 2;

                time = 12;
            }
            break;
        case 0x19:
            {
                uint16_t prev_hl = rb.hl;
                uint16_t prev_de = rb.de;
                rb.hl = rb.hl + rb.de;

                rb.f &= 0b10111111;
                if ((rb.hl & 0xF) + (rb.de & 0xF) > 0xF) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (rb.hl < prev_hl || rb.hl < prev_de) rb.f |= 0b00010000;
                else rb.f &= ~0b00010000;

                if (debug) {
                    printf("ADD HL, DE");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x1A:
            {
                rb.a = ram[rb.de];

                if (debug) {
                    printf("LD A, (DE)");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x1C:
            {
                rb.e++;

                if (rb.e == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                // TODO: half-carry flag

                if (debug) {
                    printf("INC E");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x1D:
            {
                rb.e--;

                if (rb.e == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                // TODO: half-carry flag

                if (debug) {
                    printf("DEC E");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x1E:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.e = imm;

                if (debug) {
                    printf("LD E, 0x%02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0x20:
            {
                int8_t imm = ram[rb.pc+1];

                if (!(rb.f & 0b10000000)) {
                    if (imm >= 0) rb.pc += imm;
                    else rb.pc -= (uint8_t) (-imm);

                    time = 12;
                }
                else {
                    time = 8;
                }

                if (debug) {
                    printf("JR NZ, %s0x%02X", imm < 0 ? "-" : "", imm < 0 ? -imm : imm);
                }
                rb.pc += 2;
            }
            break;
        case 0x21:
            {
                uint16_t imm = ram[rb.pc+1] | (ram[rb.pc+2] << 8);

                rb.hl = imm;

                if (debug) {
                    printf("LD HL, 0x%04X", imm);
                }
                rb.pc += 3;

                time = 12;
            }
            break;
        case 0x22:
            {
                writeByte(rb.hl, rb.a);
                rb.hl++;

                if (debug) {
                    printf("LDI (HL), A");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x23:
            {
                rb.hl++;

                if (debug) {
                    printf("INC HL");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x24:
            {
                rb.h++;

                if (rb.h == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                // TODO: half-carry flag

                if (debug) {
                    printf("INC H");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x26:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.h = imm;

                if (debug) {
                    printf("LD H, 0x%02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0x27:
            {
                if (!(rb.f & 0b01000000)) {  // after an addition, adjust if (half-)carry occurred or if result is out of bounds
                    if ((rb.f & 0b00010000) || rb.a > 0x99) {
                        rb.a += 0x60;
                        rb.f |= 0b00010000;
                    }
                    if ((rb.f & 0b00100000) || (rb.a & 0x0f) > 0x09) {
                        rb.a += 0x6;
                    }
                } else {  // after a subtraction, only adjust if (half-)carry occurred
                    if ((rb.f & 0b00010000)) {
                        rb.a -= 0x60;
                    }
                    if ((rb.f & 0b00100000)) {
                        rb.a -= 0x6;
                    }
                }
                // these flags are always updated
                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= ~0b00100000; // h flag is always cleared

                if (debug) {
                    printf("DAA");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x28:
            {
                int8_t imm = ram[rb.pc+1];

                if (rb.f & 0b10000000) {
                    if (imm >= 0) rb.pc += imm;
                    else rb.pc -= (uint8_t) (-imm);

                    time = 12;
                }
                else {
                    time = 8;
                }

                if (debug) {
                    printf("JR Z, %s0x%02X", imm < 0 ? "-" : "", imm < 0 ? -imm : imm);
                }
                rb.pc += 2;
            }
            break;
        case 0x2A:
            {
                rb.a = ram[rb.hl];
                rb.hl++;

                if (debug) {
                    printf("LDI A, (HL)");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x2C:
            {
                rb.l++;

                if (rb.l == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                // TODO: half-carry flag

                if (debug) {
                    printf("INC L");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x2D:
            {
                rb.l--;

                if (rb.l == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                // TODO: half-carry flag

                if (debug) {
                    printf("DEC L");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x2E:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.l = imm;

                if (debug) {
                    printf("LD L, 0x%02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0x2F:
            {
                rb.a = ~rb.a;

                rb.f |= 0b01100000;

                if (debug) {
                    printf("CPL");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x31:
            {
                uint16_t imm = ram[rb.pc+1] | (ram[rb.pc+2] << 8);
                rb.sp = imm;

                if (debug) {
                    printf("LD SP, 0x%04X", imm);
                }
                rb.pc += 3;

                time = 12;
            }
            break;
        case 0x32:
            {
                writeByte(rb.hl, rb.a);
                rb.hl--;

                if (debug) {
                    printf("LDD (HL), A");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x34:
            {
                writeByte(rb.hl, ram[rb.hl] +1);

                if (ram[rb.hl] == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                // TODO: half-carry flag

                if (debug) {
                    printf("INC (HL)");
                }
                rb.pc++;

                time = 12;
            }
            break;
        case 0x35:
            {
                writeByte(rb.hl, ram[rb.hl] -1);

                if (ram[rb.hl] == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                // TODO: half-carry flag

                if (debug) {
                    printf("DEC (HL)");
                }
                rb.pc++;

                time = 12;
            }
            break;
        case 0x36:
            {
                uint8_t imm = ram[rb.pc+1];

                writeByte(rb.hl, imm);

                if (debug) {
                    printf("LD (HL), 0x%02X", imm);
                }
                rb.pc += 2;

                time = 12;
            }
            break;
        case 0x39:
            {
                uint16_t prev_hl = rb.hl;
                uint16_t prev_sp = rb.sp;
                rb.hl = rb.hl + rb.sp;

                rb.f &= 0b10111111;
                if ((rb.hl & 0xF) + (rb.sp & 0xF) > 0xF) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (rb.hl < prev_hl || rb.hl < prev_sp) rb.f |= 0b00010000;
                else rb.f &= ~0b00010000;

                if (debug) {
                    printf("ADD HL, SP");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x3A:
            {
                rb.a = ram[rb.hl];
                rb.hl--;

                if (debug) {
                    printf("LDD A, (HL)");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x3C:
            {
                rb.a++;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                // TODO: half-carry flag

                if (debug) {
                    printf("INC A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x3D:
            {
                rb.a--;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                // TODO: half-carry flag

                if (debug) {
                    printf("DEC A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x3E:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.a = imm;

                if (debug) {
                    printf("LD A, 0x%02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0x40:
            {
                if (debug) {
                    printf("LD B, B");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x46:
            {
                rb.b = ram[rb.hl];

                if (debug) {
                    printf("LD B, (HL)");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x47:
            {
                rb.b = rb.a;

                if (debug) {
                    printf("LD B, A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x4E:
            {
                rb.c = ram[rb.hl];

                if (debug) {
                    printf("LD C, (HL)");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x4F:
            {
                rb.c = rb.a;

                if (debug) {
                    printf("LD C, A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x54:
            {
                rb.d = rb.h;

                if (debug) {
                    printf("LD D, H");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x56:
            {
                rb.d = ram[rb.hl];

                if (debug) {
                    printf("LD D, (HL)");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x57:
            {
                rb.d = rb.a;

                if (debug) {
                    printf("LD D, A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x5D:
            {
                rb.e = rb.l;

                if (debug) {
                    printf("LD E, L");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x5E:
            {
                rb.e = ram[rb.hl];

                if (debug) {
                    printf("LD E, (HL)");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x5F:
            {
                rb.e = rb.a;

                if (debug) {
                    printf("LD E, A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x60:
            {
                rb.h = rb.b;

                if (debug) {
                    printf("LD H, B");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x62:
            {
                rb.h = rb.d;

                if (debug) {
                    printf("LD H, D");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x65:
            {
                rb.h = rb.l;

                if (debug) {
                    printf("LD H, L");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x67:
            {
                rb.h = rb.a;

                if (debug) {
                    printf("LD H, A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x69:
            {
                rb.l = rb.c;

                if (debug) {
                    printf("LD L, C");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x6B:
            {
                rb.l = rb.e;

                if (debug) {
                    printf("LD L, E");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x6C:
            {
                rb.l = rb.h;

                if (debug) {
                    printf("LD L, H");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x6F:
            {
                rb.l = rb.a;

                if (debug) {
                    printf("LD L, A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x71:
            {
                writeByte(rb.hl, rb.c);

                if (debug) {
                    printf("LD (HL), C");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x72:
            {
                writeByte(rb.hl, rb.d);

                if (debug) {
                    printf("LD (HL), D");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x73:
            {
                writeByte(rb.hl, rb.e);

                if (debug) {
                    printf("LD (HL), E");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x77:
            {
                writeByte(rb.hl, rb.a);

                if (debug) {
                    printf("LD (HL), A");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x78:
            {
                rb.a = rb.b;

                if (debug) {
                    printf("LD A, B");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x79:
            {
                rb.a = rb.c;

                if (debug) {
                    printf("LD A, C");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x7A:
            {
                rb.a = rb.d;

                if (debug) {
                    printf("LD A, D");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x7B:
            {
                rb.a = rb.e;

                if (debug) {
                    printf("LD A, E");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x7C:
            {
                rb.a = rb.h;

                if (debug) {
                    printf("LD A, H");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x7D:
            {
                rb.a = rb.l;

                if (debug) {
                    printf("LD A, L");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x7E:
            {
                rb.a = ram[rb.hl];

                if (debug) {
                    printf("LD A, (HL)");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0x80:
            {
                uint8_t prev_a = rb.a;
                rb.a = rb.a + rb.b;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                if ((rb.a & 0xF) + (rb.l & 0xF) > 0xF) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (rb.a < prev_a) rb.f |= 0b00010000;
                else rb.f &= ~0b00010000;

                if (debug) {
                    printf("ADD A, B");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x85:
            {
                uint8_t prev_a = rb.a;
                rb.a = rb.a + rb.l;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                if ((rb.a & 0xF) + (rb.l & 0xF) > 0xF) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (rb.a < prev_a) rb.f |= 0b00010000;
                else rb.f &= ~0b00010000;

                if (debug) {
                    printf("ADD A, L");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x86:
            {
                uint8_t prev_a = rb.a;
                rb.a = rb.a + ram[rb.hl];

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                if ((rb.a & 0xF) + (ram[rb.hl] & 0xF) > 0xF) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (rb.a < prev_a) rb.f |= 0b00010000;
                else rb.f &= ~0b00010000;

                if (debug) {
                    printf("ADD A, (HL)");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x87:
            {
                uint8_t prev_a = rb.a;
                rb.a = rb.a + rb.a;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                if ((rb.a & 0xF) + (rb.a & 0xF) > 0xF) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (rb.a < prev_a) rb.f |= 0b00010000;
                else rb.f &= ~0b00010000;

                if (debug) {
                    printf("ADD A, A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x8E:
            {
                uint8_t prev_a = rb.a;
                rb.a = rb.a + ram[rb.hl] + ((rb.f >> 4) & 1);

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                if ((rb.a & 0xF) + (ram[rb.hl] & 0xF) > 0xF) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (rb.a < prev_a) rb.f |= 0b00010000;
                else rb.f &= ~0b00010000;

                if (debug) {
                    printf("ADC A, (HL)");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0x90:
            {
                uint16_t prev_a = rb.a;
                rb.a = rb.a - rb.b;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                if ((int)(rb.a & 0xf) - (int)(rb.b & 0xf) < 0) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (rb.a > prev_a) rb.f |= 0b00010000;
                else rb.f &= 0b00010000;

                if (debug) {
                    printf("SUB A, B");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0xA1:
            {
                rb.a = rb.a & rb.c;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b00100000;
                rb.f &= 0b10101111;

                if (debug) {
                    printf("AND C");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0xA7:
            {
                rb.a = rb.a & rb.a;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b00100000;
                rb.f &= 0b10101111;

                if (debug) {
                    printf("AND A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0xA9:
            {
                rb.a = rb.a ^ rb.c;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10001111;

                if (debug) {
                    printf("XOR C");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0xAF:
            {
                rb.a = rb.a ^ rb.a;

                rb.f = 0b10000000;

                if (debug) {
                    printf("XOR A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0xB0:
            {
                rb.a = rb.a | rb.b;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10001111;

                if (debug) {
                    printf("OR B");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0xB1:
            {
                rb.a = rb.a | rb.c;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10001111;

                if (debug) {
                    printf("OR C");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0xB7:
            {
                rb.a = rb.a | rb.a;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10001111;

                if (debug) {
                    printf("OR A");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0xBE:
            {
                uint16_t temp = rb.a - ram[rb.hl];
                if (temp == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                if ((int)(rb.a & 0xf) - (int)(ram[rb.hl] & 0xf) < 0) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (temp > 255) rb.f |= 0b00010000;
                else rb.f &= 0b11101111;

                if (debug) {
                    printf("CP (HL)");
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0xC0:
            {
                if (!(rb.f & 0b10000000)) {
                    rb.pc = ram[rb.sp] | ram[rb.sp+1] << 8;
                    rb.sp += 2;

                    time = 20;
                }
                else {
                    rb.pc++;

                    time = 8;
                }

                if (debug) {
                    printf("RET NZ");
                }
            }
            break;
        case 0xC1:
            {
                rb.c = ram[rb.sp];
                rb.b = ram[rb.sp+1];
                rb.sp += 2;

                if (debug) {
                    printf("POP BC");
                }
                rb.pc++;

                time = 12;
            }
            break;
        case 0xC2:
            {
                uint16_t addr = ram[rb.pc+1] | (ram[rb.pc+2] << 8);

                if (!(rb.f & 0b10000000)) {
                    rb.pc = addr;

                    time = 16;
                }
                else {
                    time = 12;
                }

                if (debug) {
                    printf("JP NZ, 0x%04X", addr);
                }
                rb.pc += 3;
            }
            break;
        case 0xC3:
            {
                uint16_t addr = ram[rb.pc+1] | (ram[rb.pc+2] << 8);

                if (debug) {
                    printf("JP 0x%04X", addr);
                }
                rb.pc = addr;

                time = 16;
            }
            break;
        case 0xC5:
            {
                writeByte(rb.sp -1, rb.b);
                writeByte(rb.sp -2, rb.c);
                rb.sp -= 2;

                if (debug) {
                    printf("PUSH BC");
                }
                rb.pc++;

                time = 16;
            }
            break;
        case 0xC6:
            {
                uint8_t imm = ram[rb.pc+1];

                uint8_t prev_a = rb.a;
                rb.a = rb.a + imm;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                if ((rb.a & 0xF) + (imm & 0xF) > 0xF) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (rb.a < prev_a) rb.f |= 0b00010000;
                else rb.f &= ~0b00010000;

                if (debug) {
                    printf("ADD A, %02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0xC8:
            {
                if (rb.f & 0b10000000) {
                    rb.pc = ram[rb.sp] | ram[rb.sp+1] << 8;
                    rb.sp += 2;

                    time = 20;
                }
                else {
                    rb.pc++;

                    time = 8;
                }

                if (debug) {
                    printf("RET Z");
                }
            }
            break;
        case 0xC9:
            {
                rb.pc = ram[rb.sp] | ram[rb.sp+1] << 8;
                rb.sp += 2;

                if (debug) {
                    printf("RET");
                }

                time = 16;
            }
            break;
        case 0xCA:
            {
                uint16_t addr = ram[rb.pc+1] | (ram[rb.pc+2] << 8);

                if (rb.f & 0b10000000) {
                    rb.pc = addr;

                    time = 16;
                }
                else {
                    rb.pc += 3;

                    time = 12;
                }

                if (debug) {
                    printf("JP Z, 0x%04X", addr);
                }
            }
            break;
        case 0xCB:
            {
                rb.pc++;
                uint8_t byte = ram[rb.pc];
                switch (byte) {
                    case 0x11:
                        {
                            bool carry = rb.c & 0b10000000;

                            rb.c <<= 1;
                            rb.c |= (rb.f >> 4) & 1;
                            if (carry) rb.f |= 0b00010000;
                            else rb.f &= 0b11101111;

                            if (rb.c == 0) rb.f |= 0b10000000;
                            else rb.f &= 0b01111111;
                            rb.f &= 0b10011111;

                            if (debug) {
                                printf("RL C");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x27:
                        {
                            bool carry = rb.c & 0b10000000;

                            rb.a <<= 1;
                            if (carry) rb.f |= 0b00010000;
                            else rb.f &= 0b11101111;

                            if (rb.a == 0) rb.f |= 0b10000000;
                            else rb.f &= 0b01111111;
                            rb.f &= 0b10011111;

                            if (debug) {
                                printf("SLA A");
                            }

                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x33:
                        {
                            rb.e = rb.e >> 4 | rb.e << 4;

                            if (debug) {
                                printf("SWAP E");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x37:
                        {
                            rb.a = rb.a >> 4 | rb.a << 4;

                            if (debug) {
                                printf("SWAP A");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x3F:
                        {
                            bool carry = rb.c & 0b1;

                            rb.a >>= 1;
                            if (carry) rb.f |= 0b00010000;
                            else rb.f &= 0b11101111;

                            if (rb.a == 0) rb.f |= 0b10000000;
                            else rb.f &= 0b01111111;
                            rb.f &= 0b10011111;

                            if (debug) {
                                printf("SRL A");
                            }

                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x40:
                        {
                            if (rb.b & 0b1) rb.f &= 0b01111111;
                            else rb.f |= 0b10000000;

                            if (debug) {
                                printf("BIT 0, B");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x50:
                        {
                            if (rb.b & 0b100) rb.f &= 0b01111111;
                            else rb.f |= 0b10000000;

                            if (debug) {
                                printf("BIT 2, B");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x58:
                        {
                            if (rb.b & 0b1000) rb.f &= 0b01111111;
                            else rb.f |= 0b10000000;

                            if (debug) {
                                printf("BIT 3, B");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x5F:
                        {
                            if (rb.a & 0b1000) rb.f &= 0b01111111;
                            else rb.f |= 0b10000000;

                            if (debug) {
                                printf("BIT 3, A");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x60:
                        {
                            if (rb.b & 0b10000) rb.f &= 0b01111111;
                            else rb.f |= 0b10000000;

                            if (debug) {
                                printf("BIT 4, B");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x68:
                        {
                            if (rb.b & 0b100000) rb.f &= 0b01111111;
                            else rb.f |= 0b10000000;

                            if (debug) {
                                printf("BIT 5, B");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x77:
                        {
                            if (rb.a & 0b1000000) rb.f &= 0b01111111;
                            else rb.f |= 0b10000000;

                            if (debug) {
                                printf("BIT 6, A");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x7C:
                        {
                            if (rb.h & 0b10000000) rb.f &= 0b01111111;
                            else rb.f |= 0b10000000;

                            if (debug) {
                                printf("BIT 7, H");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x7E:
                        {
                            if (ram[rb.hl] & 0b10000000) rb.f &= 0b01111111;
                            else rb.f |= 0b10000000;

                            if (debug) {
                                printf("BIT 7, (HL)");
                            }
                            rb.pc++;

                            time = 12;
                        }
                        break;
                    case 0x7F:
                        {
                            if (rb.a & 0b10000000) rb.f &= 0b01111111;
                            else rb.f |= 0b10000000;

                            if (debug) {
                                printf("BIT 7, A");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0x86:
                        {
                            writeByte(rb.hl, ram[rb.hl] & ~0b1);

                            if (debug) {
                                printf("RES 0, (HL)");
                            }
                            rb.pc++;

                            time = 16;
                        }
                        break;
                    case 0x87:
                        {
                            rb.a &= ~0b1;

                            if (debug) {
                                printf("RES 0, A");
                            }
                            rb.pc++;

                            time = 8;
                        }
                        break;
                    case 0xFE:
                        {
                            ram[rb.hl] |= 0b10000000;

                            if (debug) {
                                printf("SET 7, (HL)");
                            }
                            rb.pc++;

                            time = 16;
                        }
                        break;
                    default:
                        printf("\nUnimplemented extended instruction: CB%02X\n", byte);
                        exit(1);
                }
            }
            break;
        case 0xCD:
            {
                uint16_t addr = ram[rb.pc+1] | (ram[rb.pc+2] << 8);
                rb.pc += 3;

                writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
                writeByte(rb.sp -2, (rb.pc & 0xFF));
                rb.pc = addr;
                rb.sp -= 2;

                if (debug) {
                    printf("CALL 0x%04X", addr);
                }

                time = 24;
            }
            break;
        case 0xD0:
            {
                if (!(rb.f & 0b00010000)) {
                    rb.pc = ram[rb.sp] | ram[rb.sp+1] << 8;
                    rb.sp += 2;

                    time = 20;
                }
                else {
                    rb.pc++;

                    time = 8;
                }

                if (debug) {
                    printf("RET NC");
                }
            }
            break;
        case 0xD1:
            {
                rb.e = ram[rb.sp];
                rb.d = ram[rb.sp+1];
                rb.sp += 2;

                if (debug) {
                    printf("POP DE");
                }
                rb.pc++;

                time = 12;
            }
            break;
        case 0xD5:
            {
                writeByte(rb.sp -1, rb.d);
                writeByte(rb.sp -2, rb.e);
                rb.sp -= 2;

                if (debug) {
                    printf("PUSH DE");
                }
                rb.pc++;

                time = 16;
            }
            break;
        case 0xD6:
            {
                uint8_t imm = ram[rb.pc+1];

                uint16_t prev_a = rb.a;
                rb.a = rb.a - imm;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                if ((int)(rb.a & 0xf) - (int)(imm & 0xf) < 0) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (rb.a > prev_a) rb.f |= 0b00010000;
                else rb.f &= 0b00010000;

                if (debug) {
                    printf("SUB A, %02X", imm);
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0xD9:
            {
                rb.pc = ram[rb.sp] | ram[rb.sp+1] << 8;
                rb.sp += 2;
                IME = true;

                if (debug) {
                    printf("RETI");
                }

                time = 16;
            }
            break;
        case 0xDF:
            {
                rb.pc++;
                writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
                writeByte(rb.sp -2, (rb.pc & 0xFF));
                rb.pc = 0x18;
                rb.sp = rb.sp-2;

                if (debug) {
                    printf("RST 0x18");
                }

                time = 16;
            }
            break;
        case 0xE0:
            {
                uint16_t imm = ram[rb.pc+1];
                imm += 0xFF00;
                writeByte(imm, rb.a);

                if (debug) {
                    printf("LDH (0x%04X), A", imm);
                }
                rb.pc += 2;

                time = 12;
            }
            break;
        case 0xE1:
            {
                rb.l = ram[rb.sp];
                rb.h = ram[rb.sp+1];
                rb.sp += 2;

                if (debug) {
                    printf("POP HL");
                }
                rb.pc++;

                time = 12;
            }
            break;
        case 0xE2:
            {
                uint16_t imm = rb.c;
                imm += 0xFF00;
                writeByte(imm, rb.a);

                if (debug) {
                    printf("LDH (C), A", imm);
                }
                rb.pc++;

                time = 8;
            }
            break;
        case 0xE5:
            {
                writeByte(rb.sp -1, rb.h);
                writeByte(rb.sp -2, rb.l);
                rb.sp -= 2;

                if (debug) {
                    printf("PUSH HL");
                }
                rb.pc++;

                time = 16;
            }
            break;
        case 0xE6:
            {
                uint8_t imm = ram[rb.pc+1];
                rb.a = rb.a & imm;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b00100000;
                rb.f &= 0b10101111;

                if (debug) {
                    printf("AND 0x%02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0xE9:
            {
                rb.pc = rb.hl;

                if (debug) {
                    printf("JP (HL)");
                }

                time = 4;
            }
            break;
        case 0xEA:
            {
                uint16_t addr = ram[rb.pc+1] | (ram[rb.pc+2] << 8);
                writeByte(addr, rb.a);

                if (debug) {
                    printf("LD (0x%04X), A", addr);
                }
                rb.pc += 3;

                time = 16;
            }
            break;
        case 0xEE:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.a = rb.a ^ imm;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10001111;

                if (debug) {
                    printf("XOR %02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0xEF:
            {
                rb.pc++;
                writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
                writeByte(rb.sp -2, (rb.pc & 0xFF));
                rb.pc = 0x28;
                rb.sp = rb.sp-2;

                if (debug) {
                    printf("RST 0x28");
                }

                time = 16;
            }
            break;
        case 0xF0:
            {
                uint16_t imm = ram[rb.pc+1];
                imm += 0xFF00;
                rb.a = ram[imm];

                if (debug) {
                    printf("LDH A, (0x%04X)", imm);
                }
                rb.pc += 2;

                time = 12;
            }
            break;
        case 0xF1:
            {
                rb.f = ram[rb.sp];
                rb.a = ram[rb.sp+1];
                rb.sp += 2;

                if (debug) {
                    printf("POP AF");
                }
                rb.pc++;

                time = 12;
            }
            break;
        case 0xF3:
            {
                IME = false;

                if (debug) {
                    printf("DI");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0xF5:
            {
                writeByte(rb.sp -1, rb.a);
                writeByte(rb.sp -2, rb.f);
                rb.sp -= 2;

                if (debug) {
                    printf("PUSH AF");
                }
                rb.pc++;

                time = 16;
            }
            break;
        case 0xF6:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.a |= imm;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10001111;

                if (debug) {
                    printf("OR 0x%02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0xFA:
            {
                uint16_t addr = ram[rb.pc+1] | (ram[rb.pc+2] << 8);

                rb.a = ram[addr];

                if (debug) {
                    printf("LD A, (%04X)", addr);
                }
                rb.pc += 3;

                time = 16;
            }
            break;
        case 0xFB:
            {
                IME = true;

                if (debug) {
                    printf("EI");
                }
                rb.pc++;

                time = 4;
            }
            break;
        case 0xFE:
            {
                uint8_t imm = ram[rb.pc+1];

                uint16_t temp = rb.a - imm;
                if (temp == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                if ((int)(rb.a & 0xf) - (int)(imm & 0xf) < 0) rb.f |= 0b00100000;
                else rb.f &= ~0b00100000;
                if (temp > 255) rb.f |= 0b00010000;
                else rb.f &= 0b11101111;

                if (debug) {
                    printf("CP 0x%02X", imm);
                }
                rb.pc += 2;

                time = 8;
            }
            break;
        case 0xFF:
            {
                rb.pc++;
                writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
                writeByte(rb.sp -2, (rb.pc & 0xFF));
                rb.pc = 0x38;
                rb.sp = rb.sp-2;

                if (debug) {
                    printf("RST 0x38");
                }

                time = 16;
            }
            break;
        default:
            printf("\nUnimplemented instruction: %02X\n", byte);
            exit(1);
    }
    cpu_clock += time;
    gpu_clock += time;

    if (debug) {
        printf("\n\n");
    }
}

void DMGReset() {
    FILE *rom = fopen("tetris.gb", "rb");
    fread(&ram, 0x8000, 1, rom);
    fclose(rom);

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

void handleInterrupts() {
    if (!IME) return;

    int time = 0;
    //VBlank
    if ((ram[IE] & 0b1) && (ram[IF] & 0b1)) {
        IME = false;
        ram[IF] &= ~0b1;

        writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
        writeByte(rb.sp -2, (rb.pc & 0xFF));
        rb.pc = 0x40;
        rb.sp -= 2;

        time = 20;
    }

    cpu_clock += time;
}

void putPixel(SDL_Surface *surface, int x, int y, Uint32 pixel){
    Uint32 *pixels = (Uint32 *)surface->pixels;
    pixels[(y*surface->w) + x] = pixel;
}

void getInput() {
    ram[P1] |= 0b1111;

    if (ram[P1] & 0x20) {
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT]) {
            ram[P1] &= ~0b0001;
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT]) {
            ram[P1] &= ~0b0010;
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_UP]) {
            ram[P1] &= ~0b0100;
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_DOWN]) {
            ram[P1] &= ~0b1000;
        }
    }
    if (ram[P1] & 0x10) {
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_Z]) {
            ram[P1] &= ~0b0001;
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_X]) {
            ram[P1] &= ~0b0010;
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_SPACE]) {
            ram[P1] &= ~0b0100;
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RETURN]) {
            ram[P1] &= ~0b1000;
        }
    }
}

void updateRegisters() {
    ram[DIV] += rand() % 5;

    if (ram[LYC] == ram[LY]) {
        ram[STAT] |= 0b100;
        if (ram[STAT] & 0b1000000) ram[IF] |= 0b10;
    }
    else {
        ram[STAT] &= ~0b100;
    }
}

int main(int argc, char* archv[]) {
    SDL_Window *window = NULL;

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    window = SDL_CreateWindow("iron-gb", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160, 144, SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
    if(window == NULL){
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    if (!window) exit(1);

    srand(time(NULL));

    SDL_Surface *screen_surface = SDL_GetWindowSurface(window);
    DMGReset();

    SDL_Event e;
    while(1){
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT)
                exit(0);
        }

        if (BIOS == MAPPED && rb.pc == 0x100) {
            BIOS = UNMAPPED;
            FILE *rom = fopen("tetris.gb", "rb");
            fread(&ram, 0x100, 1, rom);
            fclose(rom);
        }

        getInput();
        handleInterrupts();
        updateRegisters();
        nextInstruction();

        if (debug) {
            char c = getchar();
            if (c == 'c') debug = false;
        }

#ifdef SINGLE_OUTPUT
        if (debug) {
            printf("\033[1A");
            printf("\033[2K");
            printf("\033[1A");
            printf("\033[2K");
            printf("\033[1A");
            printf("\033[2K");
            printf("\033[1A");
            printf("\033[2K");
            printf("\033[1A");
            printf("\033[2K");
            printf("\033[1A");
            printf("\033[2K");
        }
#endif

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
                    for (int x = 0; x < 160; x++) {
                        if (ram[LCDC] & 0b1) {
                            uint8_t sx = x + ram[SCX];
                            uint8_t sy = y + ram[SCY];

                            uint64_t i = 32*(sy/8) + sx/8;
                            uint8_t tile = ram[(ram[LCDC] & 0b1000 ? 0x9C00 : 0x9800) + i];

                            uint8_t low = ram[(ram[LCDC] & 0b10000 ? 0x8000 : 0x8800) + tile*16 + 2*(sy % 8)];
                            uint8_t high = ram[(ram[LCDC] & 0b10000 ? 0x8001 : 0x8801) + tile*16 + 2*(sy % 8)];

                            uint8_t tx = sx % 8;
                            uint8_t pixel = (((low << tx) & 0xFF) >> 7) | ((((high << tx) & 0xFF) >> 7) << 1);
                            uint32_t color = 0xFFFFFF - (((ram[BGP] >> (2*pixel)) & 0b11) * 5592405);

                            putPixel(screen_surface, x, y, color);
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
                            SDL_FillRect(screen_surface, NULL, 0xFFFFFF);
                        }
                        SDL_UpdateWindowSurface(window); //Should use seperate surface for rendering to allow for scaling

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

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
