#ifndef __RISCV_SIMULATOR_MEMORY_H__
#define __RISCV_SIMULATOR_MEMORY_H__

#include "utils.h"
#include <cstring>

namespace riscv {

const int DEFAULT_MEM_SIZE = 5e5;

template <size_t MEM_SIZE = DEFAULT_MEM_SIZE>
class RAM {
private:
    byte mem[MEM_SIZE];

public:
    RAM() {
        memset(mem, 0, sizeof(mem));
    }

    byte read_byte(addr_t addr) {
        return mem[addr];
    }
    hfword read_hfword(addr_t addr) {
        return mem[addr] | (mem[addr + 1] << 8);
    }
    word read_word(addr_t addr) {
        return mem[addr] | (mem[addr + 1] << 8) | (mem[addr + 2] << 16) | (mem[addr + 3] << 24);
    }

    void write_byte(addr_t addr, word data) {
        mem[addr] = data & 255;
    }
    void write_hfword(addr_t addr, word data) {
        mem[addr + 0] = data >> 0 & 255;
        mem[addr + 1] = data >> 8 & 255;
    }
    void write_word(addr_t addr, word data) {
        mem[addr + 0] = data >>  0 & 255;
        mem[addr + 1] = data >>  8 & 255;
        mem[addr + 2] = data >> 16 & 255;
        mem[addr + 3] = data >> 24 & 255;
    }

    // byte* memptr(addr_t addr) {
    //     return mem + addr;
    // }

};

}

#endif