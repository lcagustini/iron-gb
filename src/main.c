#include <unistd.h>

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
        case 0x04:
            {
                rb.b++;

                if (rb.b == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                // TODO: half-carry flag

                printf("INC B");
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
        case 0x0C:
            {
                rb.c++;

                if (rb.c == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                // TODO: half-carry flag

                printf("INC C");
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
        case 0x11:
            {
                uint16_t imm = ram[rb.pc+1] | (ram[rb.pc+2] << 8);

                rb.de = imm;

                printf("LD DE, 0x%04X", imm);
                rb.pc += 3;
            }
            break;
        case 0x13:
            {
                rb.de++;

                printf("INC DE");
                rb.pc++;
            }
            break;
        case 0x16:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.d = imm;

                printf("LD D, 0x%02X", imm);
                rb.pc += 2;
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

                printf("RL A");
                rb.pc++;
            }
            break;
        case 0x18:
            {
                int8_t imm = ram[rb.pc+1];

                if (imm >= 0) rb.pc += imm;
                else rb.pc -= (uint8_t) (-imm);

                printf("JR %s0x%02X", imm < 0 ? "-" : "", imm < 0 ? -imm : imm);
                rb.pc += 2;
            }
            break;
        case 0x19:
            {
                uint16_t prev_hl = rb.hl;
                uint16_t prev_de = rb.de;
                rb.hl = rb.hl + rb.de;

                rb.f &= 0b10111111;
                // TODO: Half carry flag
                if (rb.hl < prev_hl || rb.hl < prev_de) rb.f |= 0b00010000;

                printf("ADD HL, DE");
                rb.pc++;
            }
            break;
        case 0x1A:
            {
                rb.a = ram[rb.de];

                printf("LD A, (DE)");
                rb.pc++;
            }
            break;
        case 0x1E:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.e = imm;

                printf("LD E, 0x%02X", imm);
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
        case 0x22:
            {
                ram[rb.hl] = rb.a;
                rb.hl++;

                printf("LDI (HL), A");
                rb.pc++;
            }
            break;
        case 0x23:
            {
                rb.hl++;

                printf("INC HL");
                rb.pc++;
            }
            break;
        case 0x28:
            {
                int8_t imm = ram[rb.pc+1];

                if (rb.f & 0b10000000) {
                    if (imm >= 0) rb.pc += imm;
                    else rb.pc -= (uint8_t) (-imm);
                }

                printf("JR Z, %s0x%02X", imm < 0 ? "-" : "", imm < 0 ? -imm : imm);
                rb.pc += 2;
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
        case 0x2E:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.l = imm;

                printf("LD L, 0x%02X", imm);
                rb.pc += 2;
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
        case 0x3D:
            {
                rb.a--;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b01000000;
                // TODO: half-carry flag

                printf("DEC A");
                rb.pc++;
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
        case 0x56:
            {
                rb.d = ram[rb.hl];

                printf("LD D, (HL)");
                rb.pc++;
            }
            break;
        case 0x57:
            {
                rb.d = rb.a;

                printf("LD D, A");
                rb.pc++;
            }
            break;
        case 0x5E:
            {
                rb.e = ram[rb.hl];

                printf("LD E, (HL)");
                rb.pc++;
            }
            break;
        case 0x5F:
            {
                rb.e = rb.a;

                printf("LD E, A");
                rb.pc++;
            }
            break;
        case 0x67:
            {
                rb.h = rb.a;

                printf("LD H, A");
                rb.pc++;
            }
            break;
        case 0x77:
            {
                ram[rb.hl] = rb.a;

                printf("LD (HL), A");
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
        case 0x79:
            {
                rb.a = rb.c;

                printf("LD A, C");
                rb.pc++;
            }
            break;
        case 0x7B:
            {
                rb.a = rb.e;

                printf("LD A, E");
                rb.pc++;
            }
            break;
        case 0x87:
            {
                uint8_t prev_a = rb.a;
                rb.a = rb.a + rb.a;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10111111;
                // TODO: Half carry flag
                if (rb.a < prev_a) rb.f |= 0b00010000;

                printf("ADD A, A");
                rb.pc++;
            }
            break;
        case 0xA1:
            {
                rb.a = rb.a & rb.c;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f |= 0b00100000;
                rb.f &= 0b10101111;

                printf("AND C");
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
        case 0xC1:
            {
                rb.c = ram[rb.sp];
                rb.b = ram[rb.sp+1];
                rb.sp += 2;

                printf("POP BC");
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
        case 0xC5:
            {
                ram[rb.sp-1] = rb.b;
                ram[rb.sp-2] = rb.c;
                rb.sp -= 2;

                printf("PUSH BC");
                rb.pc++;
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

                            printf("RL C");
                            rb.pc++;
                        }
                        break;
                    case 0x37:
                        {
                            rb.a = rb.a >> 4 | rb.a << 4;

                            printf("SWAP A");
                            rb.pc++;
                        }
                        break;
                    case 0x7C:
                        {
                            if (rb.h & 0b10000000) rb.f &= 0b01111111;
                            else rb.f |= 0b10000000;

                            printf("BIT 7, H");
                            rb.pc++;
                        }
                        break;
                    case 0xFE:
                        {
                            ram[rb.hl] |= 0b10000000;

                            printf("SET 7, (HL)");
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
                rb.sp -= 2;

                printf("CALL 0x%04X", addr);
            }
            break;
        case 0xD5:
            {
                ram[rb.sp-1] = rb.d;
                ram[rb.sp-2] = rb.e;
                rb.sp -= 2;

                printf("PUSH DE");
                rb.pc++;
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
        case 0xE1:
            {
                rb.l = ram[rb.sp];
                rb.h = ram[rb.sp+1];
                rb.sp += 2;

                printf("POP HL");
                rb.pc++;
            }
            break;
        case 0xE2:
            {
                uint16_t imm = rb.c;
                imm += 0xFF00;
                ram[imm] = rb.a;

                printf("LDH (C), A", imm);
                rb.pc++;
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
        case 0xE9:
            {
                rb.pc = rb.hl;

                printf("JP (HL)");
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
        case 0xEF:
            {
                ram[rb.sp-1] = (rb.pc & 0xFF00) >> 8;
                ram[rb.sp-2] = (rb.pc & 0xFF);
                rb.pc = 0x28;
                rb.sp = rb.sp-2;

                printf("RST 0x28");
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
        case 0xF6:
            {
                uint8_t imm = ram[rb.pc+1];

                rb.a |= imm;

                if (rb.a == 0) rb.f |= 0b10000000;
                else rb.f &= 0b01111111;
                rb.f &= 0b10001111;

                printf("OR 0x%02X", imm);
                rb.pc += 2;
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
    rb.pc = 0x0;

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

void runBIOS() {
    FILE *rom = fopen("bios.gb", "rb");

    if (!rom) {
        printf("No BIOS found.");
        rb.pc = 0x100;
        return;
    }

    fread(&ram, 256, 1, rom);

    while (rb.pc < 0x100) {
        nextInstruction();

#ifdef SINGLE_OUTPUT
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
        usleep(10);
#endif
    }
}

int main(int argc, char* archv[]) {
    FILE *rom = fopen("tetris.gb", "rb");

    reset();

    runBIOS();
    fread(&ram, 0x8000, 1, rom);

    while (1) {
        nextInstruction();

#ifdef SINGLE_OUTPUT
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
        usleep(10);
#endif
    }

    return 0;
}
