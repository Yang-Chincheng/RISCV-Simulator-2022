#ifndef __RISCV_SIMULATOR_MEMUNIT_H__
#define __RISCV_SIMULATOR_MEMUNIT_H__

#include "utility.h"
#include <cstring>

namespace riscv {

class Register {
private:
    int id;
    Word value;
public:
    Register() {
        value = 0x0; 
    }
    void setid(int rid) {
        id = rid;
    }
    Word read() {
        return value;
    }
    void write(Word new_val) {
        if(id == 0) return ;
        value = new_val;
    }
};

template <size_t MEM_SIZE>
class Memory {
private:
    Byte mem[MEM_SIZE];
public:
    Memory() {
        memset(mem, 0, sizeof(mem));
    }

    Byte read_byte(addr_t addr) {
        return mem[addr];
    }
    Hfword read_hfword(addr_t addr) {
        return Hfword(mem[addr]) | (Hfword(mem[addr+1]) << 8);
    }
    Word read_word(addr_t addr) {
        return Word(mem[addr]) | (Word(mem[addr+1]) << 8) | (Word(mem[addr+2]) << 16) | (Word(mem[addr+3]) << 24);
    }

    void write_byte(addr_t addr, Byte val) {
        mem[addr] = val;
    }
    void write_hfword(addr_t addr, Hfword val) {
        mem[addr + 0] = val >> 0 & 255u;
        mem[addr + 1] = val >> 8 & 255u;
    }
    void write_word(addr_t addr, Word val) {
        mem[addr + 0] = val >>  0 & 255u;
        mem[addr + 1] = val >>  8 & 255u;
        mem[addr + 2] = val >> 16 & 255u;
        mem[addr + 3] = val >> 24 & 255u;
    }
};

}

#endif