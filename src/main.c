#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define P1 0xFF00
#define SB 0xFF01
#define SC 0xFF02
#define DIV 0xFF04
#define TIMA 0xFF05
#define TMA 0xFF06
#define TAC 0xFF07
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
uint8_t ram[0xFFFF];
bool IME;

void nextInstruction() {
    printf("A: %02X F: %02X AF: %04X (", rb.a, rb.f, rb.af);

    rb.f & 0b10000000 ? printf("Z") : printf("-");
    rb.f & 0b01000000 ? printf("N") : printf("-");
    rb.f & 0b00100000 ? printf("H") : printf("-");
    rb.f & 0b00010000 ? printf("C") : printf("-");
    printf(")\n");

    printf("B: %02X C: %02X BC: %04X\n", rb.b, rb.c, rb.bc);
    printf("D: %02X E: %02X DE: %04X\n", rb.d, rb.e, rb.de);
    printf("H: %02X L: %02X HL: %04X\n", rb.h, rb.l, rb.hl);
    printf("SP: %04X PC: %04X -> ", rb.sp, rb.pc);

    rb.f &= 0xF0;

    uint8_t byte = ram[rb.pc];
    switch (byte) {
        case 0x00:
            {
                printf("NOP");
                rb.pc++;
            }
            break;
        case 0x01:
            {
                uint16_t imm = ram[rb.pc+1] | (ram[rb.pc+2] << 8);

                rb.bc = imm;

                printf("LD BC, 0x%04X", imm);
                rb.pc += 3;
            }
            break;
        case 0x02:
            {
                ram[rb.bc] = rb.a;

                printf("LD (BC), A");
                rb.pc++;
            }
            break;
        case 0x03:
            {
                rb.bc++;

                printf("INC BC");
                rb.pc++;
            }
            break;
        case 0x05:
            {
                rb.b--;

                if (rb.b == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                // TODO: half-carry flag

                printf("DEC B");
                rb.pc++;
            }
            break;
        case 0x06:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.b = imm;

                printf("LD B, 0x%02X", imm);
                rb.pc += 2;
            }
            break;
        case 0x0B:
            {
                rb.bc--;

                printf("DEC BC");
                rb.pc++;
            }
            break;
        case 0x0D:
            {
                rb.c--;

                if (rb.c == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                // TODO: half-carry flag

                printf("DEC C");
                rb.pc++;
            }
            break;
        case 0x0E:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.c = imm;

                printf("LD C, 0x%02X", imm);
                rb.pc += 2;
            }
            break;
        case 0x20:
            {
                int8_t imm = ram[rb.pc+1];

                if (!(rb.f & 0b10000000)) {
                    if (imm >= 0) rb.pc += imm;
                    else rb.pc -= (uint8_t) (-imm);
                }

                printf("JR NZ, %s0x%02X", imm < 0 ? "-" : "", imm < 0 ? -imm : imm);
                rb.pc += 2;
            }
            break;
        case 0x21:
            {
                uint16_t imm = ram[rb.pc+1] | (ram[rb.pc+2] << 8);

                rb.hl = imm;

                printf("LD HL, 0x%04X", imm);
                rb.pc += 3;
            }
            break;
        case 0x2A:
            {
                rb.a = ram[rb.hl];
                rb.hl++;

                printf("LDI A, (HL)");
                rb.pc++;
            }
            break;
        case 0x2F:
            {
                rb.a = ~rb.a;

                rb.f |= 0b01100000;

                printf("CPL");
                rb.pc++;
            }
            break;
        case 0x31:
            {
                uint16_t imm = ram[rb.pc+1] | (ram[rb.pc+2] << 8);
                rb.sp = imm;

                printf("LD SP, 0x%04X", imm);
                rb.pc += 3;
            }
            break;
        case 0x32:
            {
                ram[rb.hl] = rb.a;
                rb.hl--;

                printf("LDD (HL), A");
                rb.pc++;
            }
            break;
        case 0x36:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.hl = imm;

                printf("LD (HL), 0x%04X", imm);
                rb.pc += 2;
            }
            break;
        case 0x3E:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.a = imm;

                printf("LD A, 0x%02X", imm);
                rb.pc += 2;
            }
            break;
        case 0x47:
            {
                rb.b = rb.a;

                printf("LD B, A");
                rb.pc++;
            }
            break;
        case 0x4F:
            {
                rb.c = rb.a;

                printf("LD C, A");
                rb.pc++;
            }
            break;
        case 0x78:
            {
                rb.a = rb.b;

                printf("LD A, B");
                rb.pc++;
            }
            break;
        case 0xA9:
            {
                rb.a = rb.a ^ rb.c;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10001111;

                printf("XOR C");
                rb.pc++;
            }
            break;
        case 0xAF:
            {
                rb.a = rb.a ^ rb.a;

                rb.f = 0b10000000;

                printf("XOR A");
                rb.pc++;
            }
            break;
        case 0xB0:
            {
                rb.a = rb.a | rb.b;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10001111;

                printf("OR B");
                rb.pc++;
            }
            break;
        case 0xB1:
            {
                rb.a = rb.a | rb.c;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10001111;

                printf("OR C");
                rb.pc++;
            }
            break;
        case 0xC3:
            {
                uint16_t addr = ram[rb.pc+1] | (ram[rb.pc+2] << 8);

                printf("JP 0x%04X", addr);
                rb.pc = addr;
            }
            break;
        case 0xC9:
            {
                rb.pc = ram[rb.sp] | ram[rb.sp+1] << 8;
                rb.sp += 2;

                printf("RET");
            }
            break;
        case 0xCB:
            {
                rb.pc++;
                uint8_t byte = ram[rb.pc];
                switch (byte) {
                    case 0x37:
                        {
                            rb.a = rb.a >> 4 | rb.a << 4;

                            printf("SWAP A");
                            rb.pc++;
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

                ram[rb.sp-1] = (rb.pc & 0xFF00) >> 8;
                ram[rb.sp-2] = (rb.pc & 0xFF);
                rb.pc = addr;
                rb.sp = rb.sp-2;

                printf("CALL 0x%04X", addr);
            }
            break;
        case 0xE0:
            {
                uint16_t imm = ram[rb.pc+1];
                imm += 0xFF00;
                ram[imm] = rb.a;

                printf("LDH (0x%04X), A", imm);
                rb.pc += 2;
            }
            break;
        case 0xE2:
            {
                uint16_t imm = rb.c;
                imm += 0xFF00;
                ram[imm] = rb.a;

                printf("LDH (C), A", imm);
                rb.pc += 2;
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

                printf("AND 0x%02X", imm);
                rb.pc += 2;
            }
            break;
        case 0xEA:
            {
                uint16_t addr = ram[rb.pc+1] | (ram[rb.pc+2] << 8);
                ram[addr] = rb.a;

                printf("LD (0x%04X), A", addr);
                rb.pc += 3;
            }
            break;
        case 0xF0:
            {
                uint16_t imm = ram[rb.pc+1];
                imm += 0xFF00;
                rb.a = ram[imm];

                printf("LDH A, (0x%04X)", imm);
                rb.pc += 2;
            }
            break;
        case 0xF3:
            {
                IME = false;

                printf("DI");
                rb.pc++;
            }
            break;
        case 0xFB:
            {
                IME = true;

                printf("EI");
                rb.pc++;
            }
            break;
        case 0xFE:
            {
                uint8_t imm = ram[rb.pc+1];

                uint16_t temp = rb.a - imm;
                if (temp == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                // TODO: Half carry flag
                if (temp > 255) rb.f |= 0b00010000;
                else rb.f &= 0b11101111;

                printf("CP 0x%04X", imm);
                rb.pc += 2;
            }
            break;
        default:
            printf("\nUnimplemented instruction: %02X\n", byte);
            exit(1);
    }
    printf("\n\n");
}

void reset() {
    IME = false;

    rb.f = 0xB0;
    rb.bc = 0x13;
    rb.de = 0xD8;
    rb.hl = 0x14D;
    rb.sp = 0xFFFE;
    rb.pc = 0x100;

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
    ram[SCY] = 0x00;
    ram[SCX] = 0x00;
    ram[LYC] = 0x00;
    ram[BGP] = 0xFC;
    ram[OBP0] = 0xFF;
    ram[OBP1] = 0xFF;
    ram[WY] = 0x00;
    ram[WX] = 0x00;
    ram[IE] = 0x00;
}

int main(int argc, char* archv[]) {
    FILE *rom = fopen("tetris.gb", "rb");

    fread(&ram, 0x8000, 1, rom);
    reset();

    while (1) {
        nextInstruction();

        ram[LY]++;
    }

    return 0;
}
