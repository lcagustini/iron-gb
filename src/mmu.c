void mem_dump(uint16_t start, uint16_t end) {
    int i, j;
    for (i = start; i <= end; i += 16) {
        printf("%04X  ", i);
        for (j = 0; j < 16; j++) {
            if (i + j <= end) {
                printf("%02x ", ram[i + j]);
            } else {
                printf("   ");
            }
            if (j == 7) {
                printf(" ");
            }
        }
        printf(" |");
        for (j = 0; j < 16; j++) {
            if (i + j <= end) {
                printf("%c",
                        isprint(ram[i + j]) ?
                        ram[i + j] : '.');
            }
        }
        printf("|\n");
    }
}

void DMATransfer() {
    uint16_t source = ram[DMA] * 0x100;
    for (int i = 0; i < 0xA0; i++) {
        ram[0xFE00 + i] = ram[source + i];
    }
}

void writeByte(uint16_t addr, uint8_t value) {
    if (addr <= 0x7FFF) return; //Can't write to ROM (no banking support yet)
    if (addr >= 0xFEA0 && addr <= 0xFEFF) return;

    switch (addr) {
        case LY:
            break;
        case DIV:
            ram[addr] = 0;
            break;
        case DMA:
            ram[addr] = value;
            DMATransfer();
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

    //STAT
    if ((ram[IE] & 0b10) && (ram[IF] & 0b10)) {
        IME = false;
        ram[IF] &= ~0b10;

        writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
        writeByte(rb.sp -2, (rb.pc & 0xFF));
        rb.pc = 0x48;
        rb.sp -= 2;

        time = 20;
    }

    //Timer Overflow
    if ((ram[IE] & 0b100) && (ram[IF] & 0b100)) {
        IME = false;
        ram[IF] &= ~0b100;

        writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
        writeByte(rb.sp -2, (rb.pc & 0xFF));
        rb.pc = 0x50;
        rb.sp -= 2;

        time = 20;
    }

    //Serial
    if ((ram[IE] & 0b1000) && (ram[IF] & 0b1000)) {
        IME = false;
        ram[IF] &= ~0b1000;

        writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
        writeByte(rb.sp -2, (rb.pc & 0xFF));
        rb.pc = 0x58;
        rb.sp -= 2;

        time = 20;
    }

    //Joypad
    if ((ram[IE] & 0b10000) && (ram[IF] & 0b10000)) {
        IME = false;
        ram[IF] &= ~0b10000;

        writeByte(rb.sp -1, (rb.pc & 0xFF00) >> 8);
        writeByte(rb.sp -2, (rb.pc & 0xFF));
        rb.pc = 0x60;
        rb.sp -= 2;

        time = 20;
    }

    cpu_clock += time;
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

    if (~ram[P1] & 0b1111) {
        ram[IF] |= 0b10000;
    }
}

void updateRegisters() {
    if (ram[LYC] == ram[LY]) {
        ram[STAT] |= 0b100;
        if (ram[STAT] & 0b1000000) ram[IF] |= 0b10;
    }
    else {
        ram[STAT] &= ~0b100;
    }

    static uint64_t last_clock = 0;
    if (cpu_clock - last_clock >= 256) {
        ram[DIV]++;
        last_clock += 256;
    }

    int rate = ram[TAC] & 0b11;

    static bool last_test_bit = true;
    switch (rate) {
        case 0:
            rate = ram[DIV] & 0b10;
            break;
        case 1:
            rate = ram[DIV] & 0b10000000;
            break;
        case 2:
            rate = ram[DIV] & 0b100000;
            break;
        case 3:
            rate = ram[DIV] & 0b1000;
            break;
    }

    bool test_bit = rate && (ram[TAC] & 0b100);

    if (last_test_bit && !test_bit) {
        if (ram[TIMA] == 0) {
            ram[TIMA] = ram[TMA];

            ram[IF] |= 0b100;
        }
        else {
            ram[TIMA]++;
        }
    }
    last_test_bit = test_bit;
}
