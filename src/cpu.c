void nextInstruction() {
  if (debug) {
    printf(COLOR_YELLOW "IF:" COLOR_RESET " %02X " COLOR_YELLOW "IE:" COLOR_RESET " %02X " COLOR_YELLOW "IME:" COLOR_RESET " %d\n", readByte(IF), readByte(IE), IME);
    puts(COLOR_BLUE "--------------------");

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
  uint8_t byte = readByte(rb.pc);
  switch (byte) {
    case 0x00:
    case 0xD3:
    case 0xDB:
    case 0xDD:
    case 0xE3:
    case 0xE4:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xF4:
    case 0xFC:
    case 0xFD:
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
        uint16_t imm = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);

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
        if ((rb.b & 0xF) + 1 > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.b++;

        if (rb.b == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("INC B");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x05:
      {
        if ((int)(rb.b & 0xF) - 1 < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.b--;

        if (rb.b == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("DEC B");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x06:
      {
        uint8_t imm = readByte(rb.pc+1);

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
        bool carry = rb.a & 0b10000000;

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
    case 0x08:
      {
        uint16_t imm = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);

        writeByte(imm+1, (rb.sp & 0xFF00) >> 8);
        writeByte(imm, (rb.sp & 0xFF));

        if (debug) {
          printf("LD (0x%04X), SP", imm);
        }
        rb.pc += 3;

        time = 20;
      }
      break;
    case 0x09:
      {
        rb.f &= 0b10111111;
        if ((rb.hl & 0xFFF) + (rb.bc & 0xFFF) > 0xFFF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.hl & 0xFFFF) + (rb.bc & 0xFFFF) > 0xFFFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.hl = rb.hl + rb.bc;

        if (debug) {
          printf("ADD HL, BC");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x0A:
      {
        rb.a = readByte(rb.bc);

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
        if ((rb.c & 0xF) + 1 > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.c++;

        if (rb.c == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("INC C");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x0D:
      {
        if ((int)(rb.c & 0xF) - 1 < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.c--;

        if (rb.c == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("DEC C");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x0E:
      {
        uint8_t imm = readByte(rb.pc+1);

        rb.c = imm;

        if (debug) {
          printf("LD C, 0x%02X", imm);
        }
        rb.pc += 2;

        time = 8;
      }
      break;
    case 0x0F:
      {
        bool carry = rb.a & 0b1;

        rb.a >>= 1;
        rb.a |= (carry << 7);
        if (carry) rb.f |= 0b00010000;
        else rb.f &= 0b11101111;

        rb.f &= 0b00011111;

        if (debug) {
          printf("RRCA");
        }

        rb.pc++;

        time = 4;
      }
      break;
    case 0x10:
      {
        if (debug) {
          printf("STOP");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x11:
      {
        uint16_t imm = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);

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
    case 0x14:
      {
        if ((rb.d & 0xF) + 1 > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.d++;

        if (rb.d == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("INC D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x15:
      {
        if ((int)(rb.d & 0xF) - 1 < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.d--;

        if (rb.d == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("DEC D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x16:
      {
        uint8_t imm = readByte(rb.pc+1);

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

        rb.f &= 0b00011111;

        if (debug) {
          printf("RLA");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x18:
      {
        int8_t imm = readByte(rb.pc+1);

        if (imm >= 0) rb.pc += imm;
        else rb.pc -= (uint8_t) (-imm);

        if (debug) {
          printf("JR %s0x%02X", imm < 0 ? "-" : "+", imm < 0 ? -imm : imm);
        }
        rb.pc += 2;

        time = 12;
      }
      break;
    case 0x19:
      {
        rb.f &= 0b10111111;
        if ((rb.hl & 0xFFF) + (rb.de & 0xFFF) > 0xFFF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.hl & 0xFFFF) + (rb.de & 0xFFFF) > 0xFFFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.hl = rb.hl + rb.de;

        if (debug) {
          printf("ADD HL, DE");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x1A:
      {
        rb.a = readByte(rb.de);

        if (debug) {
          printf("LD A, (DE)");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x1B:
      {
        rb.de--;

        if (debug) {
          printf("DEC DE");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x1C:
      {
        if ((rb.e & 0xF) + 1 > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.e++;

        if (rb.e == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("INC E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x1D:
      {
        if ((int)(rb.e & 0xF) - 1 < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.e--;

        if (rb.e == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("DEC E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x1E:
      {
        uint8_t imm = readByte(rb.pc+1);

        rb.e = imm;

        if (debug) {
          printf("LD E, 0x%02X", imm);
        }
        rb.pc += 2;

        time = 8;
      }
      break;
    case 0x1F:
      {
        bool carry = rb.a & 0b1;

        rb.a >>= 1;
        rb.a |= ((rb.f >> 4) & 1) << 7;
        if (carry) rb.f |= 0b00010000;
        else rb.f &= 0b11101111;

        rb.f &= 0b00011111;

        if (debug) {
          printf("RRA");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x20:
      {
        int8_t imm = readByte(rb.pc+1);

        if (!(rb.f & 0b10000000)) {
          if (imm >= 0) rb.pc += imm;
          else rb.pc -= (uint8_t) (-imm);

          time = 12;
        }
        else {
          time = 8;
        }

        if (debug) {
          printf("JR NZ, %s0x%02X", imm < 0 ? "-" : "+", imm < 0 ? -imm : imm);
        }
        rb.pc += 2;
      }
      break;
    case 0x21:
      {
        uint16_t imm = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);

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
        if ((rb.h & 0xF) + 1 > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.h++;

        if (rb.h == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("INC H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x25:
      {
        if ((int)(rb.h & 0xF) - 1 < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.h--;

        if (rb.h == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("DEC H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x26:
      {
        uint8_t imm = readByte(rb.pc+1);

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
        if (!(rb.f & 0b01000000)) {
          if ((rb.f & 0b00010000) || rb.a > 0x99) {
            rb.a += 0x60;
            rb.f |= 0b00010000;
          }
          if ((rb.f & 0b00100000) || (rb.a & 0x0f) > 0x09) {
            rb.a += 0x6;
          }
        }
        else {
          if ((rb.f & 0b00010000)) {
            rb.a -= 0x60;
          }
          if ((rb.f & 0b00100000)) {
            rb.a -= 0x6;
          }
        }

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= ~0b00100000;

        if (debug) {
          printf("DAA");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x28:
      {
        int8_t imm = readByte(rb.pc+1);

        if (rb.f & 0b10000000) {
          if (imm >= 0) rb.pc += imm;
          else rb.pc -= (uint8_t) (-imm);

          time = 12;
        }
        else {
          time = 8;
        }

        if (debug) {
          printf("JR Z, %s0x%02X", imm < 0 ? "-" : "+", imm < 0 ? -imm : imm);
        }
        rb.pc += 2;
      }
      break;
    case 0x29:
      {
        rb.f &= 0b10111111;
        if ((rb.hl & 0xFFF) + (rb.hl & 0xFFF) > 0xFFF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.hl & 0xFFFF) + (rb.hl & 0xFFFF) > 0xFFFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.hl = rb.hl + rb.hl;

        if (debug) {
          printf("ADD HL, HL");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x2A:
      {
        rb.a = readByte(rb.hl);
        rb.hl++;

        if (debug) {
          printf("LDI A, (HL)");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x2B:
      {
        rb.hl--;

        if (debug) {
          printf("DEC HL");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x2C:
      {
        if ((rb.l & 0xF) + 1 > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.l++;

        if (rb.l == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("INC L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x2D:
      {
        if ((int)(rb.l & 0xF) - 1 < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.l--;

        if (rb.l == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("DEC L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x2E:
      {
        uint8_t imm = readByte(rb.pc+1);

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
    case 0x30:
      {
        int8_t imm = readByte(rb.pc+1);

        if (!(rb.f & 0b00010000)) {
          if (imm >= 0) rb.pc += imm;
          else rb.pc -= (uint8_t) (-imm);

          time = 12;
        }
        else {
          time = 8;
        }

        if (debug) {
          printf("JR NC, %s0x%02X", imm < 0 ? "-" : "+", imm < 0 ? -imm : imm);
        }
        rb.pc += 2;
      }
      break;
    case 0x31:
      {
        uint16_t imm = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);
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
    case 0x33:
      {
        rb.sp++;

        if (debug) {
          printf("INC SP");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x34:
      {
        if ((readByte(rb.hl) & 0xF) + 1 > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        writeByte(rb.hl, readByte(rb.hl) +1);

        if (readByte(rb.hl) == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("INC (HL)");
        }
        rb.pc++;

        time = 12;
      }
      break;
    case 0x35:
      {
        if ((int)(readByte(rb.hl) & 0xF) - 1 < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        writeByte(rb.hl, readByte(rb.hl) -1);

        if (readByte(rb.hl) == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("DEC (HL)");
        }
        rb.pc++;

        time = 12;
      }
      break;
    case 0x36:
      {
        uint8_t imm = readByte(rb.pc+1);

        writeByte(rb.hl, imm);

        if (debug) {
          printf("LD (HL), 0x%02X", imm);
        }
        rb.pc += 2;

        time = 12;
      }
      break;
    case 0x37:
      {
        rb.f |= 0b00010000;
        rb.f &= 0b10011111;

        if (debug) {
          printf("SCF");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x38:
      {
        int8_t imm = readByte(rb.pc+1);

        if (rb.f & 0b00010000) {
          if (imm >= 0) rb.pc += imm;
          else rb.pc -= (uint8_t) (-imm);

          time = 12;
        }
        else {
          time = 8;
        }

        if (debug) {
          printf("JR C, %s0x%02X", imm < 0 ? "-" : "+", imm < 0 ? -imm : imm);
        }
        rb.pc += 2;
      }
      break;
    case 0x39:
      {
        rb.f &= 0b10111111;
        if ((rb.hl & 0xFFF) + (rb.sp & 0xFFF) > 0xFFF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.hl & 0xFFFF) + (rb.sp & 0xFFFF) > 0xFFFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.hl = rb.hl + rb.sp;

        if (debug) {
          printf("ADD HL, SP");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x3A:
      {
        rb.a = readByte(rb.hl);
        rb.hl--;

        if (debug) {
          printf("LDD A, (HL)");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x3B:
      {
        rb.sp--;

        if (debug) {
          printf("DEC SP");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x3C:
      {
        if ((rb.a & 0xF) + 1 > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.a++;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("INC A");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x3D:
      {
        if ((int)(rb.a & 0xF) - 1 < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;

        rb.a--;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("DEC A");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x3E:
      {
        uint8_t imm = readByte(rb.pc+1);

        rb.a = imm;

        if (debug) {
          printf("LD A, 0x%02X", imm);
        }
        rb.pc += 2;

        time = 8;
      }
      break;
    case 0x3F:
      {
        uint8_t carry = rb.f & 0b00010000;
        if (carry) rb.f &= ~0b00010000;
        else rb.f |= 0b00010000;
        rb.f &= 0b10011111;

        if (debug) {
          printf("CCF");
        }
        rb.pc++;

        time = 4;
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
    case 0x41:
      {
        rb.b = rb.c;

        if (debug) {
          printf("LD B, C");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x42:
      {
        rb.b = rb.d;

        if (debug) {
          printf("LD B, D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x43:
      {
        rb.b = rb.e;

        if (debug) {
          printf("LD B, E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x44:
      {
        rb.b = rb.h;

        if (debug) {
          printf("LD B, H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x45:
      {
        rb.b = rb.l;

        if (debug) {
          printf("LD B, L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x46:
      {
        rb.b = readByte(rb.hl);

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
    case 0x48:
      {
        rb.c = rb.b;

        if (debug) {
          printf("LD C, B");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x49:
      {
        if (debug) {
          printf("LD C, C");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x4A:
      {
        rb.c = rb.d;

        if (debug) {
          printf("LD C, D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x4B:
      {
        rb.c = rb.e;

        if (debug) {
          printf("LD C, E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x4C:
      {
        rb.c = rb.h;

        if (debug) {
          printf("LD C, H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x4D:
      {
        rb.c = rb.l;

        if (debug) {
          printf("LD C, L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x4E:
      {
        rb.c = readByte(rb.hl);

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
    case 0x50:
      {
        rb.d = rb.b;

        if (debug) {
          printf("LD D, B");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x51:
      {
        rb.d = rb.c;

        if (debug) {
          printf("LD D, C");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x52:
      {
        if (debug) {
          printf("LD D, D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x53:
      {
        rb.d = rb.e;

        if (debug) {
          printf("LD D, E");
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
    case 0x55:
      {
        rb.d = rb.l;

        if (debug) {
          printf("LD D, L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x56:
      {
        rb.d = readByte(rb.hl);

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
    case 0x58:
      {
        rb.e = rb.b;

        if (debug) {
          printf("LD E, B");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x59:
      {
        rb.e = rb.c;

        if (debug) {
          printf("LD E, C");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x5A:
      {
        rb.e = rb.d;

        if (debug) {
          printf("LD E, D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x5B:
      {
        if (debug) {
          printf("LD E, E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x5C:
      {
        rb.e = rb.h;

        if (debug) {
          printf("LD E, H");
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
        rb.e = readByte(rb.hl);

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
    case 0x61:
      {
        rb.h = rb.c;

        if (debug) {
          printf("LD H, C");
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
    case 0x63:
      {
        rb.h = rb.e;

        if (debug) {
          printf("LD H, E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x64:
      {
        if (debug) {
          printf("LD H, H");
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
    case 0x66:
      {
        rb.h = readByte(rb.hl);

        if (debug) {
          printf("LD H, (HL)");
        }
        rb.pc++;

        time = 8;
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
    case 0x68:
      {
        rb.l = rb.b;

        if (debug) {
          printf("LD L, B");
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
    case 0x6A:
      {
        rb.l = rb.d;

        if (debug) {
          printf("LD L, D");
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
    case 0x6D:
      {
        if (debug) {
          printf("LD L, L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x6E:
      {
        rb.l = readByte(rb.hl);

        if (debug) {
          printf("LD L, (HL)");
        }
        rb.pc++;

        time = 8;
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
    case 0x70:
      {
        writeByte(rb.hl, rb.b);

        if (debug) {
          printf("LD (HL), B");
        }
        rb.pc++;

        time = 8;
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
    case 0x74:
      {
        writeByte(rb.hl, rb.h);

        if (debug) {
          printf("LD (HL), H");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x75:
      {
        writeByte(rb.hl, rb.l);

        if (debug) {
          printf("LD (HL), L");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x76:
      {
        if (debug) {
          printf("HALT");
        }
        rb.pc++;

        time = 4;
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
        rb.a = readByte(rb.hl);

        if (debug) {
          printf("LD A, (HL)");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x7F:
      {
        if (debug) {
          printf("LD A, A");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x80:
      {
        if ((rb.a & 0xF) + (rb.b & 0xF) > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.b & 0xFF) > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.b;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADD A, B");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x81:
      {
        if ((rb.a & 0xF) + (rb.c & 0xF) > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.c & 0xFF) > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.c;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADD A, C");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x82:
      {
        if ((rb.a & 0xF) + (rb.d & 0xF) > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.d & 0xFF) > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.d;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADD A, D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x83:
      {
        if ((rb.a & 0xF) + (rb.e & 0xF) > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.e & 0xFF) > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.e;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADD A, E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x84:
      {
        if ((rb.a & 0xF) + (rb.h & 0xF) > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.h & 0xFF) > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.h;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADD A, H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x85:
      {
        if ((rb.a & 0xF) + (rb.l & 0xF) > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.l & 0xFF) > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.l;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADD A, L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x86:
      {
        if ((rb.a & 0xF) + (readByte(rb.hl) & 0xF) > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (readByte(rb.hl) & 0xFF) > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + readByte(rb.hl);

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADD A, (HL)");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x87:
      {
        if ((rb.a & 0xF) + (rb.a & 0xF) > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.a & 0xFF) > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.a;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADD A, A");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x88:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((rb.a & 0xF) + (rb.b & 0xF) + carry > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.b & 0xFF) + carry > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.b + carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADC A, B");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x89:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((rb.a & 0xF) + (rb.c & 0xF) + carry > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.c & 0xFF) + carry > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.c + carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADC A, C");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x8A:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((rb.a & 0xF) + (rb.d & 0xF) + carry > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.d & 0xFF) + carry > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.d + carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADC A, D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x8B:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((rb.a & 0xF) + (rb.e & 0xF) + carry > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.e & 0xFF) + carry > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.e + carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADC A, E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x8C:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((rb.a & 0xF) + (rb.h & 0xF) + carry > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.h & 0xFF) + carry > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.h + carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADC A, H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x8D:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((rb.a & 0xF) + (rb.l & 0xF) + carry > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.l & 0xFF) + carry > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.l + carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADC A, L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x8E:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((rb.a & 0xF) + (readByte(rb.hl) & 0xF) + carry > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (readByte(rb.hl) & 0xFF) + carry > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + readByte(rb.hl) + carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADC A, (HL)");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x8F:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((rb.a & 0xF) + (rb.a & 0xF) + carry > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (rb.a & 0xFF) + carry > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + rb.a + carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADC A, A");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x90:
      {
        if ((int)(rb.a & 0xF) - (int)(rb.b & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.b & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.b;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SUB A, B");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x91:
      {
        if ((int)(rb.a & 0xF) - (int)(rb.c & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.c & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.c;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SUB A, C");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x92:
      {
        if ((int)(rb.a & 0xF) - (int)(rb.d & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.d & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.d;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SUB A, D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x93:
      {
        if ((int)(rb.a & 0xF) - (int)(rb.e & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.e & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.e;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SUB A, E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x94:
      {
        if ((int)(rb.a & 0xF) - (int)(rb.h & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.h & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.h;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SUB A, H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x95:
      {
        if ((int)(rb.a & 0xF) - (int)(rb.l & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.l & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.l;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SUB A, L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x96:
      {
        if ((int)(rb.a & 0xF) - (int)(readByte(rb.hl) & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(readByte(rb.hl) & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - readByte(rb.hl);

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SUB A, (HL)");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x97:
      {
        if ((int)(rb.a & 0xF) - (int)(rb.a & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.a & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.a;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SUB A, A");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x98:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((int)(rb.a & 0xF) - (int)(rb.b & 0xF) - (int)carry < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.b & 0xFF) - (int)carry < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.b - carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SBC A, B");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x99:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((int)(rb.a & 0xF) - (int)(rb.c & 0xF) - (int)carry < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.c & 0xFF) - (int)carry < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.c - carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SBC A, C");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x9A:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((int)(rb.a & 0xF) - (int)(rb.d & 0xF) - (int)carry < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.d & 0xFF) - (int)carry < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.d - carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SBC A, D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x9B:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((int)(rb.a & 0xF) - (int)(rb.e & 0xF) - (int)carry < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.e & 0xFF) - (int)carry < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.e - carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SBC A, E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x9C:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((int)(rb.a & 0xF) - (int)(rb.h & 0xF) - (int)carry < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.h & 0xFF) - (int)carry < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.h - carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SBC A, H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x9D:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((int)(rb.a & 0xF) - (int)(rb.l & 0xF) - (int)carry < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.l & 0xFF) - (int)carry < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.l - carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SBC A, L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0x9E:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((int)(rb.a & 0xF) - (int)(readByte(rb.hl) & 0xF) - (int)carry < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(readByte(rb.hl) & 0xFF) - (int)carry < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - readByte(rb.hl) - carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SBC A, (HL)");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0x9F:
      {
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((int)(rb.a & 0xF) - (int)(rb.a & 0xF) - (int)carry < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.a & 0xFF) - (int)carry < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - rb.a - carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SBC A, A");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xA0:
      {
        rb.a = rb.a & rb.b;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b00100000;
        rb.f &= 0b10101111;

        if (debug) {
          printf("AND B");
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
    case 0xA2:
      {
        rb.a = rb.a & rb.d;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b00100000;
        rb.f &= 0b10101111;

        if (debug) {
          printf("AND D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xA3:
      {
        rb.a = rb.a & rb.e;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b00100000;
        rb.f &= 0b10101111;

        if (debug) {
          printf("AND E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xA4:
      {
        rb.a = rb.a & rb.h;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b00100000;
        rb.f &= 0b10101111;

        if (debug) {
          printf("AND H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xA5:
      {
        rb.a = rb.a & rb.l;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b00100000;
        rb.f &= 0b10101111;

        if (debug) {
          printf("AND L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xA6:
      {
        rb.a = rb.a & readByte(rb.hl);

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b00100000;
        rb.f &= 0b10101111;

        if (debug) {
          printf("AND (HL)");
        }
        rb.pc++;

        time = 8;
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
    case 0xA8:
      {
        rb.a = rb.a ^ rb.b;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("XOR B");
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
    case 0xAA:
      {
        rb.a = rb.a ^ rb.d;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("XOR D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xAB:
      {
        rb.a = rb.a ^ rb.e;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("XOR E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xAC:
      {
        rb.a = rb.a ^ rb.h;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("XOR H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xAD:
      {
        rb.a = rb.a ^ rb.l;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("XOR L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xAE:
      {
        rb.a = rb.a ^ readByte(rb.hl);

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("XOR (HL)");
        }
        rb.pc++;

        time = 8;
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
    case 0xB2:
      {
        rb.a = rb.a | rb.d;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("OR D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xB3:
      {
        rb.a = rb.a | rb.e;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("OR E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xB4:
      {
        rb.a = rb.a | rb.h;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("OR H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xB5:
      {
        rb.a = rb.a | rb.l;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("OR L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xB6:
      {
        rb.a = rb.a | readByte(rb.hl);

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("OR (HL)");
        }
        rb.pc++;

        time = 8;
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
    case 0xB8:
      {
        uint16_t temp = rb.a - rb.b;
        if (temp == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;
        if ((int)(rb.a & 0xF) - (int)(rb.b & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.b & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= 0b11101111;

        if (debug) {
          printf("CP B");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xB9:
      {
        uint16_t temp = rb.a - rb.c;
        if (temp == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;
        if ((int)(rb.a & 0xF) - (int)(rb.c & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.c & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= 0b11101111;

        if (debug) {
          printf("CP C");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xBA:
      {
        uint16_t temp = rb.a - rb.d;
        if (temp == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;
        if ((int)(rb.a & 0xF) - (int)(rb.d & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.d & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= 0b11101111;

        if (debug) {
          printf("CP D");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xBB:
      {
        uint16_t temp = rb.a - rb.e;
        if (temp == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;
        if ((int)(rb.a & 0xF) - (int)(rb.e & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.e & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= 0b11101111;

        if (debug) {
          printf("CP E");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xBC:
      {
        uint16_t temp = rb.a - rb.h;
        if (temp == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;
        if ((int)(rb.a & 0xF) - (int)(rb.h & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.h & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= 0b11101111;

        if (debug) {
          printf("CP H");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xBD:
      {
        uint16_t temp = rb.a - rb.l;
        if (temp == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;
        if ((int)(rb.a & 0xF) - (int)(rb.l & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.l & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= 0b11101111;

        if (debug) {
          printf("CP L");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xBE:
      {
        uint16_t temp = rb.a - readByte(rb.hl);
        if (temp == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;
        if ((int)(rb.a & 0xF) - (int)(readByte(rb.hl) & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(readByte(rb.hl) & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= 0b11101111;

        if (debug) {
          printf("CP (HL)");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0xBF:
      {
        uint16_t temp = rb.a - rb.a;
        if (temp == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;
        if ((int)(rb.a & 0xF) - (int)(rb.a & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(rb.a & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= 0b11101111;

        if (debug) {
          printf("CP A");
        }
        rb.pc++;

        time = 4;
      }
      break;
    case 0xC0:
      {
        if (!(rb.f & 0b10000000)) {
          rb.pc = readByte(rb.sp) | readByte(rb.sp+1) << 8;
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
        rb.c = readByte(rb.sp);
        rb.b = readByte(rb.sp+1);
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
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);

        if (!(rb.f & 0b10000000)) {
          rb.pc = addr;

          time = 16;
        }
        else {
          rb.pc += 3;

          time = 12;
        }

        if (debug) {
          printf("JP NZ, 0x%04X", addr);
        }
      }
      break;
    case 0xC3:
      {
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);

        if (debug) {
          printf("JP 0x%04X", addr);
        }
        rb.pc = addr;

        time = 16;
      }
      break;
    case 0xC4:
      {
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);
        rb.pc += 3;

        if (!(rb.f & 0b10000000)) {
          writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
          writeByte(rb.sp -2, (rb.pc & 0xFF));
          rb.pc = addr;
          rb.sp -= 2;

          time = 24;
        }
        else {
          time = 12;
        }

        if (debug) {
          printf("CALL NZ, 0x%04X", addr);
        }
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
        uint8_t imm = readByte(rb.pc+1);
        if ((rb.a & 0xF) + (imm & 0xF) > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (imm & 0xFF) > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + imm;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADD A, 0x%02X", imm);
        }
        rb.pc += 2;

        time = 8;
      }
      break;
    case 0xC7:
      {
        rb.pc++;
        writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
        writeByte(rb.sp -2, (rb.pc & 0xFF));
        rb.pc = 0x0;
        rb.sp = rb.sp-2;

        if (debug) {
          printf("RST 0x0");
        }

        time = 16;
      }
      break;
    case 0xC8:
      {
        if (rb.f & 0b10000000) {
          rb.pc = readByte(rb.sp) | readByte(rb.sp+1) << 8;
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
        rb.pc = readByte(rb.sp) | readByte(rb.sp+1) << 8;
        rb.sp += 2;

        if (debug) {
          printf("RET");
        }

        time = 16;
      }
      break;
    case 0xCA:
      {
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);

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
        uint8_t byte = readByte(rb.pc);
        switch (byte) {
          case 0x00:
            {
              bool carry = rb.b & 0b10000000;

              rb.b <<= 1;
              rb.b |= (carry >> 7);

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.b == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RLC B");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x01:
            {
              bool carry = rb.c & 0b10000000;

              rb.c <<= 1;
              rb.c |= (carry >> 7);

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.c == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RLC C");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x02:
            {
              bool carry = rb.d & 0b10000000;

              rb.d <<= 1;
              rb.d |= (carry >> 7);

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.d == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RLC D");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x03:
            {
              bool carry = rb.e & 0b10000000;

              rb.e <<= 1;
              rb.e |= (carry >> 7);

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.e == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RLC E");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x04:
            {
              bool carry = rb.h & 0b10000000;

              rb.h <<= 1;
              rb.h |= (carry >> 7);

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.h == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RLC H");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x05:
            {
              bool carry = rb.l & 0b10000000;

              rb.l <<= 1;
              rb.l |= (carry >> 7);

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.l == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RLC L");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x06:
            {
              bool carry = readByte(rb.hl) & 0b10000000;

              writeByte(rb.hl, readByte(rb.hl) << 1);
              writeByte(rb.hl, readByte(rb.hl) | (carry >> 7));

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (readByte(rb.hl) == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RLC (HL)");
              }

              rb.pc++;

              time = 16;
            }
            break;
          case 0x07:
            {
              bool carry = rb.a & 0b10000000;

              rb.a <<= 1;
              rb.a |= (carry >> 7);

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.a == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RLC A");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x08:
            {
              bool carry = rb.b & 0b1;

              rb.b >>= 1;
              rb.b |= (carry << 7);
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.b == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RRC B");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x09:
            {
              bool carry = rb.c & 0b1;

              rb.c >>= 1;
              rb.c |= (carry << 7);
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.c == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RRC C");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x0A:
            {
              bool carry = rb.d & 0b1;

              rb.d >>= 1;
              rb.d |= (carry << 7);
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.d == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RRC D");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x0B:
            {
              bool carry = rb.e & 0b1;

              rb.e >>= 1;
              rb.e |= (carry << 7);
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.e == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RRC E");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x0C:
            {
              bool carry = rb.h & 0b1;

              rb.h >>= 1;
              rb.h |= (carry << 7);
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.h == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RRC H");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x0D:
            {
              bool carry = rb.l & 0b1;

              rb.l >>= 1;
              rb.l |= (carry << 7);
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.l == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RRC L");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x0E:
            {
              bool carry = readByte(rb.hl) & 0b1;

              writeByte(rb.hl, readByte(rb.hl) >> 1);
              writeByte(rb.hl, readByte(rb.hl) | (carry << 7));

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (readByte(rb.hl) == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RRC (HL)");
              }

              rb.pc++;

              time = 16;
            }
            break;
          case 0x0F:
            {
              bool carry = rb.a & 0b1;

              rb.a >>= 1;
              rb.a |= (carry << 7);
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              if (rb.a == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RRC A");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x10:
            {
              bool carry = rb.b & 0b10000000;

              rb.b <<= 1;
              rb.b |= (rb.f >> 4) & 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.b == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RL B");
              }
              rb.pc++;

              time = 8;
            }
            break;
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
          case 0x12:
            {
              bool carry = rb.d & 0b10000000;

              rb.d <<= 1;
              rb.d |= (rb.f >> 4) & 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.d == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RL D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x13:
            {
              bool carry = rb.e & 0b10000000;

              rb.e <<= 1;
              rb.e |= (rb.f >> 4) & 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.e == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RL E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x14:
            {
              bool carry = rb.h & 0b10000000;

              rb.h <<= 1;
              rb.h |= (rb.f >> 4) & 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.h == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RL H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x15:
            {
              bool carry = rb.l & 0b10000000;

              rb.l <<= 1;
              rb.l |= (rb.f >> 4) & 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.l == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RL L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x16:
            {
              bool carry = readByte(rb.hl) & 0b10000000;

              writeByte(rb.hl, readByte(rb.hl) << 1);
              writeByte(rb.hl, readByte(rb.hl) | ((rb.f >> 4) & 1));

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (readByte(rb.hl) == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RL (HL)");
              }
              rb.pc++;

              time = 16;
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
              bool carry = rb.b & 0b1;

              rb.b >>= 1;
              rb.b |= ((rb.f >> 4) & 1) << 7;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.b == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RR B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x19:
            {
              bool carry = rb.c & 0b1;

              rb.c >>= 1;
              rb.c |= ((rb.f >> 4) & 1) << 7;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.c == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RR C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x1A:
            {
              bool carry = rb.d & 0b1;

              rb.d >>= 1;
              rb.d |= ((rb.f >> 4) & 1) << 7;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.d == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RR D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x1B:
            {
              bool carry = rb.e & 0b1;

              rb.e >>= 1;
              rb.e |= ((rb.f >> 4) & 1) << 7;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.e == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RR E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x1C:
            {
              bool carry = rb.h & 0b1;

              rb.h >>= 1;
              rb.h |= ((rb.f >> 4) & 1) << 7;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.h == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RR H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x1D:
            {
              bool carry = rb.l & 0b1;

              rb.l  >>= 1;
              rb.l  |= ((rb.f >> 4) & 1) << 7;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.l  == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RR L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x1E:
            {
              bool carry = readByte(rb.hl) & 0b1;

              writeByte(rb.hl, readByte(rb.hl)  >> 1);
              writeByte(rb.hl, readByte(rb.hl)  | (((rb.f >> 4) & 1) << 7));
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (readByte(rb.hl)  == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RR (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0x1F:
            {
              bool carry = rb.a & 0b1;

              rb.a  >>= 1;
              rb.a  |= ((rb.f >> 4) & 1) << 7;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.a  == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("RR A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x20:
            {
              bool carry = rb.b & 0b10000000;

              rb.b <<= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.b == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SLA B");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x21:
            {
              bool carry = rb.c & 0b10000000;

              rb.c <<= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.c == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SLA C");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x22:
            {
              bool carry = rb.d & 0b10000000;

              rb.d <<= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.d == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SLA D");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x23:
            {
              bool carry = rb.e & 0b10000000;

              rb.e <<= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.e == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SLA E");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x24:
            {
              bool carry = rb.h & 0b10000000;

              rb.h <<= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.h == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SLA H");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x25:
            {
              bool carry = rb.l & 0b10000000;

              rb.l <<= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.l == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SLA L");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x26:
            {
              bool carry = readByte(rb.hl) & 0b10000000;

              writeByte(rb.hl, readByte(rb.hl) << 1);

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (readByte(rb.hl) == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SLA (HL)");
              }

              rb.pc++;

              time = 16;
            }
            break;
          case 0x27:
            {
              bool carry = rb.a & 0b10000000;

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
          case 0x28:
            {
              bool carry = rb.b & 0b1;

              rb.b >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              rb.b |= (rb.b & 0b01000000) << 1;

              if (rb.b == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRA B");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x29:
            {
              bool carry = rb.c & 0b1;

              rb.c >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              rb.c |= (rb.c & 0b01000000) << 1;

              if (rb.c == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRA C");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x2A:
            {
              bool carry = rb.d & 0b1;

              rb.d >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              rb.d |= (rb.d & 0b01000000) << 1;

              if (rb.d == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRA D");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x2B:
            {
              bool carry = rb.e & 0b1;

              rb.e >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              rb.e |= (rb.e & 0b01000000) << 1;

              if (rb.e == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRA E");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x2C:
            {
              bool carry = rb.h & 0b1;

              rb.h >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              rb.h |= (rb.h & 0b01000000) << 1;

              if (rb.h == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRA H");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x2D:
            {
              bool carry = rb.l & 0b1;

              rb.l >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              rb.l |= (rb.l & 0b01000000) << 1;

              if (rb.l == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRA L");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x2E:
            {
              bool carry = readByte(rb.hl) & 0b1;

              writeByte(rb.hl, readByte(rb.hl) >> 1);

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              writeByte(rb.hl, readByte(rb.hl) | ((readByte(rb.hl) & 0b01000000) << 1));

              if (readByte(rb.hl) == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRA (HL)");
              }

              rb.pc++;

              time = 16;
            }
            break;
          case 0x2F:
            {
              bool carry = rb.a & 0b1;

              rb.a >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;
              rb.a |= (rb.a & 0b01000000) << 1;

              if (rb.a == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRA A");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x30:
            {
              rb.b = (rb.b >> 4) | (rb.b << 4);

              if (rb.b == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10001111;

              if (debug) {
                printf("SWAP B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x31:
            {
              rb.c = (rb.c >> 4) | (rb.c << 4);

              if (rb.c == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10001111;

              if (debug) {
                printf("SWAP C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x32:
            {
              rb.d = (rb.d >> 4) | (rb.d << 4);

              if (rb.d == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10001111;

              if (debug) {
                printf("SWAP D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x33:
            {
              rb.e = (rb.e >> 4) | (rb.e << 4);

              if (rb.e == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10001111;

              if (debug) {
                printf("SWAP E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x34:
            {
              rb.h = (rb.h >> 4) | (rb.h << 4);

              if (rb.h == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10001111;

              if (debug) {
                printf("SWAP H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x35:
            {
              rb.l = (rb.l >> 4) | (rb.l << 4);

              if (rb.l == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10001111;

              if (debug) {
                printf("SWAP L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x36:
            {
              writeByte(rb.hl, (readByte(rb.hl) >> 4) | (readByte(rb.hl) << 4));

              if (readByte(rb.hl) == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10001111;

              if (debug) {
                printf("SWAP (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0x37:
            {
              rb.a = (rb.a >> 4) | (rb.a << 4);

              if (rb.a == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10001111;

              if (debug) {
                printf("SWAP A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x38:
            {
              bool carry = rb.b & 0b1;

              rb.b >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.b == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRL B");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x39:
            {
              bool carry = rb.c & 0b1;

              rb.c >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.c == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRL C");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x3A:
            {
              bool carry = rb.d & 0b1;

              rb.d >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.d == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRL D");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x3B:
            {
              bool carry = rb.e & 0b1;

              rb.e >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.e == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRL E");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x3C:
            {
              bool carry = rb.h & 0b1;

              rb.h >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.h == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRL H");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x3D:
            {
              bool carry = rb.l & 0b1;

              rb.l >>= 1;
              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (rb.l == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRL L");
              }

              rb.pc++;

              time = 8;
            }
            break;
          case 0x3E:
            {
              bool carry = readByte(rb.hl) & 0b1;

              writeByte(rb.hl, readByte(rb.hl) >> 1);

              if (carry) rb.f |= 0b00010000;
              else rb.f &= 0b11101111;

              if (readByte(rb.hl) == 0) rb.f |= 0b10000000;
              else rb.f &= 0b01111111;
              rb.f &= 0b10011111;

              if (debug) {
                printf("SRL (HL)");
              }

              rb.pc++;

              time = 16;
            }
            break;
          case 0x3F:
            {
              bool carry = rb.a & 0b1;

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
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 0, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x41:
            {
              if (rb.c & 0b1) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 0, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x42:
            {
              if (rb.d & 0b1) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 0, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x43:
            {
              if (rb.e & 0b1) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 0, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x44:
            {
              if (rb.h & 0b1) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 0, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x45:
            {
              if (rb.l & 0b1) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 0, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x46:
            {
              if (readByte(rb.hl) & 0b1) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 0, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0x47:
            {
              if (rb.a & 0b1) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 0, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x48:
            {
              if (rb.b & 0b10) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 1, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x49:
            {
              if (rb.c & 0b10) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 1, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x4A:
            {
              if (rb.d & 0b10) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 1, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x4B:
            {
              if (rb.e & 0b10) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 1, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x4C:
            {
              if (rb.h & 0b10) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 1, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x4D:
            {
              if (rb.l & 0b10) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 1, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x4E:
            {
              if (readByte(rb.hl) & 0b10) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 1, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0x4F:
            {
              if (rb.a & 0b10) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 1, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x50:
            {
              if (rb.b & 0b100) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 2, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x51:
            {
              if (rb.c & 0b100) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 2, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x52:
            {
              if (rb.d & 0b100) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 2, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x53:
            {
              if (rb.e & 0b100) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 2, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x54:
            {
              if (rb.h & 0b100) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 2, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x55:
            {
              if (rb.l & 0b100) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 2, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x56:
            {
              if (readByte(rb.hl) & 0b100) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 2, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0x57:
            {
              if (rb.a & 0b100) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 2, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x58:
            {
              if (rb.b & 0b1000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 3, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x59:
            {
              if (rb.c & 0b1000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 3, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x5A:
            {
              if (rb.d & 0b1000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 3, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x5B:
            {
              if (rb.e & 0b1000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 3, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x5C:
            {
              if (rb.h & 0b1000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 3, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x5D:
            {
              if (rb.l & 0b1000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 3, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x5E:
            {
              if (readByte(rb.hl) & 0b1000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 3, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0x5F:
            {
              if (rb.a & 0b1000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

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
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 4, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x61:
            {
              if (rb.c & 0b10000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 4, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x62:
            {
              if (rb.d & 0b10000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 4, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x63:
            {
              if (rb.e & 0b10000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 4, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x64:
            {
              if (rb.h & 0b10000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 4, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x65:
            {
              if (rb.l & 0b10000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 4, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x66:
            {
              if (readByte(rb.hl) & 0b10000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 4, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0x67:
            {
              if (rb.a & 0b10000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 4, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x68:
            {
              if (rb.b & 0b100000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 5, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x69:
            {
              if (rb.c & 0b100000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 5, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x6A:
            {
              if (rb.d & 0b100000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 5, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x6B:
            {
              if (rb.e & 0b100000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 5, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x6C:
            {
              if (rb.h & 0b100000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 5, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x6D:
            {
              if (rb.l & 0b100000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 5, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x6E:
            {
              if (readByte(rb.hl) & 0b100000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 5, (HL)");
              }
              rb.pc++;

              time = 12;
            }
            break;
          case 0x6F:
            {
              if (rb.a & 0b100000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 5, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x70:
            {
              if (rb.b & 0b1000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 6, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x71:
            {
              if (rb.c & 0b1000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 6, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x72:
            {
              if (rb.d & 0b1000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 6, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x73:
            {
              if (rb.e & 0b1000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 6, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x74:
            {
              if (rb.h & 0b1000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 6, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x75:
            {
              if (rb.l & 0b1000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 6, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x76:
            {
              if (readByte(rb.hl) & 0b1000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 6, (HL)");
              }
              rb.pc++;

              time = 12;
            }
            break;
          case 0x77:
            {
              if (rb.a & 0b1000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 6, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x78:
            {
              if (rb.b & 0b10000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 7, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x79:
            {
              if (rb.c & 0b10000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 7, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x7A:
            {
              if (rb.d & 0b10000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 7, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x7B:
            {
              if (rb.e & 0b10000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 7, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x7C:
            {
              if (rb.h & 0b10000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 7, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x7D:
            {
              if (rb.l & 0b10000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 7, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x7E:
            {
              if (readByte(rb.hl) & 0b10000000) rb.f &= 0b01111111;
              else rb.f |= 0b10000000;
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

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
              rb.f &= 0b10111111;
              rb.f |= 0b00100000;

              if (debug) {
                printf("BIT 7, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x80:
            {
              rb.b &= ~0b1;

              if (debug) {
                printf("RES 0, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x81:
            {
              rb.c &= ~0b1;

              if (debug) {
                printf("RES 0, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x82:
            {
              rb.d &= ~0b1;

              if (debug) {
                printf("RES 0, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x83:
            {
              rb.e &= ~0b1;

              if (debug) {
                printf("RES 0, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x84:
            {
              rb.h &= ~0b1;

              if (debug) {
                printf("RES 0, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x85:
            {
              rb.l &= ~0b1;

              if (debug) {
                printf("RES 0, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x86:
            {
              writeByte(rb.hl, readByte(rb.hl) & ~0b1);

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
          case 0x88:
            {
              rb.b &= ~0b10;

              if (debug) {
                printf("RES 1, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x89:
            {
              rb.c &= ~0b10;

              if (debug) {
                printf("RES 1, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x8A:
            {
              rb.d &= ~0b10;

              if (debug) {
                printf("RES 1, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x8B:
            {
              rb.e &= ~0b10;

              if (debug) {
                printf("RES 1, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x8C:
            {
              rb.h &= ~0b10;

              if (debug) {
                printf("RES 1, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x8D:
            {
              rb.l &= ~0b10;

              if (debug) {
                printf("RES 1, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x8E:
            {
              writeByte(rb.hl, readByte(rb.hl) & ~0b10);

              if (debug) {
                printf("RES 1, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0x8F:
            {
              rb.a &= ~0b10;

              if (debug) {
                printf("RES 1, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x90:
            {
              rb.b &= ~0b100;

              if (debug) {
                printf("RES 2, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x91:
            {
              rb.c &= ~0b100;

              if (debug) {
                printf("RES 2, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x92:
            {
              rb.d &= ~0b100;

              if (debug) {
                printf("RES 2, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x93:
            {
              rb.e &= ~0b100;

              if (debug) {
                printf("RES 2, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x94:
            {
              rb.h &= ~0b100;

              if (debug) {
                printf("RES 2, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x95:
            {
              rb.l &= ~0b100;

              if (debug) {
                printf("RES 2, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x96:
            {
              writeByte(rb.hl, readByte(rb.hl) & ~0b100);

              if (debug) {
                printf("RES 2, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0x97:
            {
              rb.a &= ~0b100;

              if (debug) {
                printf("RES 2, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x98:
            {
              rb.b &= ~0b1000;

              if (debug) {
                printf("RES 3, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x99:
            {
              rb.c &= ~0b1000;

              if (debug) {
                printf("RES 3, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x9A:
            {
              rb.d &= ~0b1000;

              if (debug) {
                printf("RES 3, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x9B:
            {
              rb.e &= ~0b1000;

              if (debug) {
                printf("RES 3, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x9C:
            {
              rb.h &= ~0b1000;

              if (debug) {
                printf("RES 3, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x9D:
            {
              rb.l &= ~0b1000;

              if (debug) {
                printf("RES 3, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0x9E:
            {
              writeByte(rb.hl, readByte(rb.hl) & ~0b1000);

              if (debug) {
                printf("RES 3, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0x9F:
            {
              rb.a &= ~0b1000;

              if (debug) {
                printf("RES 3, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xA0:
            {
              rb.b &= ~0b10000;

              if (debug) {
                printf("RES 4, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xA1:
            {
              rb.c &= ~0b10000;

              if (debug) {
                printf("RES 4, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xA2:
            {
              rb.d &= ~0b10000;

              if (debug) {
                printf("RES 4, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xA3:
            {
              rb.e &= ~0b10000;

              if (debug) {
                printf("RES 4, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xA4:
            {
              rb.h &= ~0b10000;

              if (debug) {
                printf("RES 4, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xA5:
            {
              rb.l &= ~0b10000;

              if (debug) {
                printf("RES 4, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xA6:
            {
              writeByte(rb.hl, readByte(rb.hl) & ~0b10000);

              if (debug) {
                printf("RES 4, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xA7:
            {
              rb.a &= ~0b10000;

              if (debug) {
                printf("RES 4, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xA8:
            {
              rb.b &= ~0b100000;

              if (debug) {
                printf("RES 5, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xA9:
            {
              rb.c &= ~0b100000;

              if (debug) {
                printf("RES 5, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xAA:
            {
              rb.d &= ~0b100000;

              if (debug) {
                printf("RES 5, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xAB:
            {
              rb.e &= ~0b100000;

              if (debug) {
                printf("RES 5, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xAC:
            {
              rb.h &= ~0b100000;

              if (debug) {
                printf("RES 5, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xAD:
            {
              rb.l &= ~0b100000;

              if (debug) {
                printf("RES 5, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xAE:
            {
              writeByte(rb.hl, readByte(rb.hl) & ~0b100000);

              if (debug) {
                printf("RES 5, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xAF:
            {
              rb.a &= ~0b100000;

              if (debug) {
                printf("RES 5, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xB0:
            {
              rb.b &= ~0b1000000;

              if (debug) {
                printf("RES 6, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xB1:
            {
              rb.c &= ~0b1000000;

              if (debug) {
                printf("RES 6, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xB2:
            {
              rb.d &= ~0b1000000;

              if (debug) {
                printf("RES 6, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xB3:
            {
              rb.e &= ~0b1000000;

              if (debug) {
                printf("RES 6, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xB4:
            {
              rb.h &= ~0b1000000;

              if (debug) {
                printf("RES 6, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xB5:
            {
              rb.l &= ~0b1000000;

              if (debug) {
                printf("RES 6, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xB6:
            {
              writeByte(rb.hl, readByte(rb.hl) & ~0b1000000);

              if (debug) {
                printf("RES 6, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xB7:
            {
              rb.a &= ~0b1000000;

              if (debug) {
                printf("RES 6, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xB8:
            {
              rb.b &= ~0b10000000;

              if (debug) {
                printf("RES 7, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xB9:
            {
              rb.c &= ~0b10000000;

              if (debug) {
                printf("RES 7, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xBA:
            {
              rb.d &= ~0b10000000;

              if (debug) {
                printf("RES 7, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xBB:
            {
              rb.e &= ~0b10000000;

              if (debug) {
                printf("RES 7, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xBC:
            {
              rb.h &= ~0b10000000;

              if (debug) {
                printf("RES 7, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xBD:
            {
              rb.l &= ~0b10000000;

              if (debug) {
                printf("RES 7, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xBE:
            {
              writeByte(rb.hl, readByte(rb.hl) & ~0b10000000);

              if (debug) {
                printf("RES 7, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xBF:
            {
              rb.a &= ~0b10000000;

              if (debug) {
                printf("RES 7, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xC0:
            {
              rb.b |= 0b1;

              if (debug) {
                printf("SET 0, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xC1:
            {
              rb.c |= 0b1;

              if (debug) {
                printf("SET 0, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xC2:
            {
              rb.d |= 0b1;

              if (debug) {
                printf("SET 0, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xC3:
            {
              rb.e |= 0b1;

              if (debug) {
                printf("SET 0, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xC4:
            {
              rb.h |= 0b1;

              if (debug) {
                printf("SET 0, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xC5:
            {
              rb.l |= 0b1;

              if (debug) {
                printf("SET 0, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xC6:
            {
              writeByte(rb.hl, readByte(rb.hl) | 0b1);

              if (debug) {
                printf("SET 0, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xC7:
            {
              rb.a = rb.a | 0b1;

              if (debug) {
                printf("SET 0, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xC8:
            {
              rb.b |= 0b10;

              if (debug) {
                printf("SET 1, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xC9:
            {
              rb.c |= 0b10;

              if (debug) {
                printf("SET 1, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xCA:
            {
              rb.d |= 0b10;

              if (debug) {
                printf("SET 1, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xCB:
            {
              rb.e |= 0b10;

              if (debug) {
                printf("SET 1, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xCC:
            {
              rb.h |= 0b10;

              if (debug) {
                printf("SET 1, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xCD:
            {
              rb.l |= 0b10;

              if (debug) {
                printf("SET 1, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xCE:
            {
              writeByte(rb.hl, readByte(rb.hl) | 0b10);

              if (debug) {
                printf("SET 1, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xCF:
            {
              rb.a = rb.a | 0b10;

              if (debug) {
                printf("SET 1, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xD0:
            {
              rb.b |= 0b100;

              if (debug) {
                printf("SET 2, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xD1:
            {
              rb.c |= 0b100;

              if (debug) {
                printf("SET 2, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xD2:
            {
              rb.d |= 0b100;

              if (debug) {
                printf("SET 2, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xD3:
            {
              rb.e |= 0b100;

              if (debug) {
                printf("SET 2, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xD4:
            {
              rb.h |= 0b100;

              if (debug) {
                printf("SET 2, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xD5:
            {
              rb.l |= 0b100;

              if (debug) {
                printf("SET 2, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xD6:
            {
              writeByte(rb.hl, readByte(rb.hl) | 0b100);

              if (debug) {
                printf("SET 2, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xD7:
            {
              rb.a = rb.a | 0b100;

              if (debug) {
                printf("SET 2, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xD8:
            {
              rb.b |= 0b1000;

              if (debug) {
                printf("SET 3, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xD9:
            {
              rb.c |= 0b1000;

              if (debug) {
                printf("SET 3, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xDA:
            {
              rb.d |= 0b1000;

              if (debug) {
                printf("SET 3, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xDB:
            {
              rb.e |= 0b1000;

              if (debug) {
                printf("SET 3, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xDC:
            {
              rb.h |= 0b1000;

              if (debug) {
                printf("SET 3, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xDD:
            {
              rb.l |= 0b1000;

              if (debug) {
                printf("SET 3, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xDE:
            {
              writeByte(rb.hl, readByte(rb.hl) | 0b1000);

              if (debug) {
                printf("SET 3, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xDF:
            {
              rb.a |= 0b1000;

              if (debug) {
                printf("SET 3, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xE0:
            {
              rb.b |= 0b10000;

              if (debug) {
                printf("SET 4, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xE1:
            {
              rb.c |= 0b10000;

              if (debug) {
                printf("SET 4, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xE2:
            {
              rb.d |= 0b10000;

              if (debug) {
                printf("SET 4, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xE3:
            {
              rb.e |= 0b10000;

              if (debug) {
                printf("SET 4, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xE4:
            {
              rb.h |= 0b10000;

              if (debug) {
                printf("SET 4, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xE5:
            {
              rb.l |= 0b10000;

              if (debug) {
                printf("SET 4, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xE6:
            {
              writeByte(rb.hl, readByte(rb.hl) | 0b10000);

              if (debug) {
                printf("SET 4, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xE7:
            {
              rb.a |= 0b10000;

              if (debug) {
                printf("SET 4, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xE8:
            {
              rb.b |= 0b100000;

              if (debug) {
                printf("SET 5, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xE9:
            {
              rb.c |= 0b100000;

              if (debug) {
                printf("SET 5, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xEA:
            {
              rb.d |= 0b100000;

              if (debug) {
                printf("SET 5, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xEB:
            {
              rb.e |= 0b100000;

              if (debug) {
                printf("SET 5, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xEC:
            {
              rb.h |= 0b100000;

              if (debug) {
                printf("SET 5, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xED:
            {
              rb.l |= 0b100000;

              if (debug) {
                printf("SET 5, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xEE:
            {
              writeByte(rb.hl, readByte(rb.hl) | 0b100000);

              if (debug) {
                printf("SET 5, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xEF:
            {
              rb.a = rb.a | 0b100000;

              if (debug) {
                printf("SET 5, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xF0:
            {
              rb.b |= 0b1000000;

              if (debug) {
                printf("SET 6, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xF1:
            {
              rb.c |= 0b1000000;

              if (debug) {
                printf("SET 6, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xF2:
            {
              rb.d |= 0b1000000;

              if (debug) {
                printf("SET 6, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xF3:
            {
              rb.e |= 0b1000000;

              if (debug) {
                printf("SET 6, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xF4:
            {
              rb.h |= 0b1000000;

              if (debug) {
                printf("SET 6, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xF5:
            {
              rb.l |= 0b1000000;

              if (debug) {
                printf("SET 6, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xF6:
            {
              writeByte(rb.hl, readByte(rb.hl) | 0b1000000);

              if (debug) {
                printf("SET 6, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xF7:
            {
              rb.a = rb.a | 0b1000000;

              if (debug) {
                printf("SET 6, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xF8:
            {
              rb.b |= 0b10000000;

              if (debug) {
                printf("SET 7, B");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xF9:
            {
              rb.c |= 0b10000000;

              if (debug) {
                printf("SET 7, C");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xFA:
            {
              rb.d |= 0b10000000;

              if (debug) {
                printf("SET 7, D");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xFB:
            {
              rb.e |= 0b10000000;

              if (debug) {
                printf("SET 7, E");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xFC:
            {
              rb.h |= 0b10000000;

              if (debug) {
                printf("SET 7, H");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xFD:
            {
              rb.l |= 0b10000000;

              if (debug) {
                printf("SET 7, L");
              }
              rb.pc++;

              time = 8;
            }
            break;
          case 0xFE:
            {
              writeByte(rb.hl, readByte(rb.hl) | 0b10000000);

              if (debug) {
                printf("SET 7, (HL)");
              }
              rb.pc++;

              time = 16;
            }
            break;
          case 0xFF:
            {
              rb.a = rb.a | 0b10000000;

              if (debug) {
                printf("SET 7, A");
              }
              rb.pc++;

              time = 8;
            }
            break;
          default:
            printf("\nUnimplemented extended instruction: CB%02X\n", byte);
            exit(1);
        }
      }
      break;
    case 0xCC:
      {
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);
        rb.pc += 3;

        if (rb.f & 0b10000000) {
          writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
          writeByte(rb.sp -2, (rb.pc & 0xFF));
          rb.pc = addr;
          rb.sp -= 2;

          time = 24;
        }
        else {
          time = 12;
        }

        if (debug) {
          printf("CALL Z, 0x%04X", addr);
        }
      }
      break;
    case 0xCD:
      {
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);
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
    case 0xCE:
      {
        uint8_t imm = readByte(rb.pc+1);
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((rb.a & 0xF) + (imm & 0xF) + carry > 0xF) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((rb.a & 0xFF) + (imm & 0xFF) + carry > 0xFF) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a + imm + carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10111111;

        if (debug) {
          printf("ADC A, 0x%02X", imm);
        }
        rb.pc += 2;

        time = 8;
      }
      break;
    case 0xCF:
      {
        rb.pc++;
        writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
        writeByte(rb.sp -2, (rb.pc & 0xFF));
        rb.pc = 0x8;
        rb.sp = rb.sp-2;

        if (debug) {
          printf("RST 0x8");
        }

        time = 16;
      }
      break;
    case 0xD0:
      {
        if (!(rb.f & 0b00010000)) {
          rb.pc = readByte(rb.sp) | readByte(rb.sp+1) << 8;
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
        rb.e = readByte(rb.sp);
        rb.d = readByte(rb.sp+1);
        rb.sp += 2;

        if (debug) {
          printf("POP DE");
        }
        rb.pc++;

        time = 12;
      }
      break;
    case 0xD2:
      {
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);

        if (!(rb.f & 0b00010000)) {
          rb.pc = addr;

          time = 16;
        }
        else {
          rb.pc += 3;

          time = 12;
        }

        if (debug) {
          printf("JP NC, 0x%04X", addr);
        }
      }
      break;
    case 0xD4:
      {
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);
        rb.pc += 3;

        if (!(rb.f & 0b00010000)) {
          writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
          writeByte(rb.sp -2, (rb.pc & 0xFF));
          rb.pc = addr;
          rb.sp -= 2;

          time = 24;
        }
        else {
          time = 12;
        }

        if (debug) {
          printf("CALL NC, 0x%04X", addr);
        }
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
        uint8_t imm = readByte(rb.pc+1);

        if (((int)(rb.a & 0xF) - (int)(imm & 0xF)) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(imm & 0xFF) < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - imm;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SUB A, 0x%02X", imm);
        }
        rb.pc += 2;

        time = 8;
      }
      break;
    case 0xD7:
      {
        rb.pc++;
        writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
        writeByte(rb.sp -2, (rb.pc & 0xFF));
        rb.pc = 0x10;
        rb.sp = rb.sp-2;

        if (debug) {
          printf("RST 0x10");
        }

        time = 16;
      }
      break;
    case 0xD8:
      {
        if (rb.f & 0b00010000) {
          rb.pc = readByte(rb.sp) | readByte(rb.sp+1) << 8;
          rb.sp += 2;

          time = 20;
        }
        else {
          rb.pc++;

          time = 8;
        }

        if (debug) {
          printf("RET C");
        }
      }
      break;
    case 0xD9:
      {
        rb.pc = readByte(rb.sp) | readByte(rb.sp+1) << 8;
        rb.sp += 2;
        IME = true;

        if (debug) {
          printf("RETI");
        }

        time = 16;
      }
      break;
    case 0xDA:
      {
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);

        if (rb.f & 0b00010000) {
          rb.pc = addr;

          time = 16;
        }
        else {
          rb.pc += 3;

          time = 12;
        }

        if (debug) {
          printf("JP C, 0x%04X", addr);
        }
      }
      break;
    case 0xDC:
      {
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);
        rb.pc += 3;

        if (rb.f & 0b00010000) {
          writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
          writeByte(rb.sp -2, (rb.pc & 0xFF));
          rb.pc = addr;
          rb.sp -= 2;

          time = 24;
        }
        else {
          time = 12;
        }

        if (debug) {
          printf("CALL C, 0x%04X", addr);
        }
      }
      break;
    case 0xDE:
      {
        uint8_t imm = readByte(rb.pc+1);
        uint8_t carry = ((rb.f >> 4) & 1);

        if ((int)(rb.a & 0xF) - (int)(imm & 0xF) - (int)carry < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(imm & 0xFF) - (int)carry < 0) rb.f |= 0b00010000;
        else rb.f &= ~0b00010000;

        rb.a = rb.a - imm - carry;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;

        if (debug) {
          printf("SBC A, 0x%02X", imm);
        }
        rb.pc += 2;

        time = 8;
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
        uint16_t imm = readByte(rb.pc+1);
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
        rb.l = readByte(rb.sp);
        rb.h = readByte(rb.sp+1);
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
          printf("LDH (C), A");
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
        uint8_t imm = readByte(rb.pc+1);
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
    case 0xE7:
      {
        rb.pc++;
        writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
        writeByte(rb.sp -2, (rb.pc & 0xFF));
        rb.pc = 0x20;
        rb.sp = rb.sp-2;

        if (debug) {
          printf("RST 0x20");
        }

        time = 16;
      }
      break;
    case 0xE8:
      {
        int8_t imm = readByte(rb.pc+1);

        if (imm >= 0) {
          if ((rb.sp & 0xF) + (imm & 0xF) > 0xF) rb.f |= 0b00100000;
          else rb.f &= ~0b00100000;
          if ((rb.sp & 0xFF) + (imm & 0xFF) > 0xFF) rb.f |= 0b00010000;
          else rb.f &= ~0b00010000;
        }
        else {
          int temp = (int)rb.sp + imm;
          if ((temp & 0xF) <= (rb.sp & 0xF)) rb.f |= 0b00100000;
          else rb.f &= ~0b00100000;
          if ((temp & 0xFF) <= (rb.sp & 0xFF)) rb.f |= 0b00010000;
          else rb.f &= ~0b00010000;
        }

        rb.sp = rb.sp + imm;

        rb.f &= 0b00111111;

        if (debug) {
          printf("ADD SP, 0x%02X", imm);
        }
        rb.pc += 2;

        time = 16;
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
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);
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
        uint8_t imm = readByte(rb.pc+1);

        rb.a = rb.a ^ imm;

        if (rb.a == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f &= 0b10001111;

        if (debug) {
          printf("XOR 0x%02X", imm);
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
        uint16_t imm = readByte(rb.pc+1);
        imm += 0xFF00;
        rb.a = readByte(imm);

        if (debug) {
          printf("LDH A, (0x%04X)", imm);
        }
        rb.pc += 2;

        time = 12;
      }
      break;
    case 0xF1:
      {
        rb.f = readByte(rb.sp);
        rb.a = readByte(rb.sp+1);
        rb.sp += 2;

        if (debug) {
          printf("POP AF");
        }
        rb.pc++;

        time = 12;
      }
      break;
    case 0xF2:
      {
        uint16_t imm = rb.c;
        imm += 0xFF00;
        rb.a = readByte(imm);

        if (debug) {
          printf("LDH A, (C)");
        }
        rb.pc++;

        time = 8;
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
        uint8_t imm = readByte(rb.pc+1);

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
    case 0xF7:
      {
        rb.pc++;
        writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
        writeByte(rb.sp -2, (rb.pc & 0xFF));
        rb.pc = 0x30;
        rb.sp = rb.sp-2;

        if (debug) {
          printf("RST 0x30");
        }

        time = 16;
      }
      break;
    case 0xF8:
      {
        int8_t imm = readByte(rb.pc+1);

        if (imm >= 0) {
          if ((rb.sp & 0xF) + (imm & 0xF) > 0xF) rb.f |= 0b00100000;
          else rb.f &= ~0b00100000;
          if ((rb.sp & 0xFF) + (imm & 0xFF) > 0xFF) rb.f |= 0b00010000;
          else rb.f &= ~0b00010000;
        }
        else {
          int temp = (int)rb.sp + imm;
          if ((temp & 0xF) <= (rb.sp & 0xF)) rb.f |= 0b00100000;
          else rb.f &= ~0b00100000;
          if ((temp & 0xFF) <= (rb.sp & 0xFF)) rb.f |= 0b00010000;
          else rb.f &= ~0b00010000;
        }
        rb.f &= 0b00111111;

        rb.hl = rb.sp + imm;

        if (debug) {
          printf("LD HL, (SP %s 0x%02X)", imm < 0 ? "-" : "+", imm < 0 ? -imm : imm);
        }
        rb.pc += 2;

        time = 12;
      }
      break;
    case 0xF9:
      {
        rb.sp = rb.hl;

        if (debug) {
          printf("LD SP, HL");
        }
        rb.pc++;

        time = 8;
      }
      break;
    case 0xFA:
      {
        uint16_t addr = readByte(rb.pc+1) | (readByte(rb.pc+2) << 8);

        rb.a = readByte(addr);

        if (debug) {
          printf("LD A, (0x%04X)", addr);
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
        uint8_t imm = readByte(rb.pc+1);

        uint16_t temp = rb.a - imm;
        if (temp == 0) rb.f |= 0b10000000;
        else rb.f &= 0b01111111;
        rb.f |= 0b01000000;
        if ((int)(rb.a & 0xF) - (int)(imm & 0xF) < 0) rb.f |= 0b00100000;
        else rb.f &= ~0b00100000;
        if ((int)(rb.a & 0xFF) - (int)(imm & 0xFF) < 0) rb.f |= 0b00010000;
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
