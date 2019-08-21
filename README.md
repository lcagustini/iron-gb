# IronGB

This is a learning exercise on emulators.

It has all Sharp LR35902 instructions implemented and a primitive PPU.  
This means it can already run some simple games, like Tetris and Dr.Mario.

Currently trying to implement other Gameboy subsystems, like the timer, to improve game compatibility, and maybe someday a couple mappers will be supported.

Running (needs SDL2 installed):
```
make
./bin/main ROM
```
