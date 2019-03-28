#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

void nextInstruction() {
    printf("%04X -> ", rb.pc);

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
        case 0x0D:
            {
                rb.c--;

                if (rb.c == 0) rb.f |= 0b10000000;
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
        case 0xAF:
            {
                rb.a = rb.a ^ rb.a;

                rb.f = 0b10000000;

                printf("XOR A");
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
        case 0xCD:
            {
                // TODO: call instruction
            }
            break;
        case 0xE0:
            {
                uint8_t imm = ram[rb.pc+1];
                imm += 0xFF00;
                ram[imm] = rb.a;

                printf("LDH (0x%04X), A", imm);
                rb.pc += 2;
            }
            break;
        case 0xE2:
            {
                uint8_t imm = rb.c;
                imm += 0xFF00;
                ram[imm] = rb.a;

                printf("LDH (C), A", imm);
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
                uint8_t imm = ram[rb.pc+1];
                imm += 0xFF00;
                rb.a = ram[imm];

                printf("LDH A, (0x%04X)", imm);
                rb.pc += 2;
            }
            break;
        case 0xF3:
            {
                // TODO: Disable interrupts

                printf("DI");
                rb.pc++;
            }
            break;
        case 0xFE:
            {
                uint8_t imm = ram[rb.pc+1];

                uint16_t temp = rb.a - imm;
                if (temp == 0) rb.f |= 0b10000000;
                rb.f |= 0b01000000;
                // TODO: Half carry flag
                if (temp > 255) rb.f |= 0b00010000;

                printf("CP 0x%04X", imm);
                rb.pc += 2;
            }
            break;
        default:
            printf("\nUnimplemented instruction: %02X\n", byte);
            exit(1);
    }
    printf("\n");
}

int main(int argc, char* archv[]) {
    FILE *rom = fopen("tetris.gb", "rb");

    fread(&ram, 0x8000, 1, rom);

    rb.pc = 0x100;
    while (1) {
        nextInstruction();
    }

    return 0;
}
