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
    if (addr <= 0x7FFF) return; //Can't write to ROM
    if (addr >= 0xFEA0 && addr <= 0xFEFF) return;

    switch (addr) {
        case DIV:
        case LY:
            ram[addr] = 0;
            break;
        default:
            ram[addr] = value;
    }

    if (addr == DMA) DMATransfer();

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
    ram[DIV] += cpu_clock % 293339;

    if (ram[LYC] == ram[LY]) {
        ram[STAT] |= 0b100;
        if (ram[STAT] & 0b1000000) ram[IF] |= 0b10;
    }
    else {
        ram[STAT] &= ~0b100;
    }

    if (ram[TAC] & 0b100) {
        int rate = ram[TAC] & 0b11;

        switch (rate) {
            case 0:
                rate = cpu_clock % 4096 ? 0 : 1;
                break;
            case 1:
                rate = cpu_clock % 262144 ? 0 : 1;
                break;
            case 2:
                rate = cpu_clock % 65536 ? 0 : 1;
                break;
            case 3:
                rate = cpu_clock % 16384 ? 0 : 1;
                break;
        }

        ram[TIMA] += rate;

        if (ram[TIMA] == 0) {
            ram[TIMA] = ram[TMA];

            ram[IF] |= 0b100;
        }
    }
}
