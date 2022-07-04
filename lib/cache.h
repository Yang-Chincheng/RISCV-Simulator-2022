#ifndef __RISCV_SIMULATOR_CACHE_H__
#define __RISCV_SIMULATOR_CACHE_H__

#include <cstring>
#include <cassert>
#include "utils.h"
#include "register.h"
#include "ram.h"
#include "inst.h"

namespace riscv {

const int CACHE_SIZE = 32768;
const int LINE_LEN = 64;
const int LINE_NUM = 512;

class Cache {
private:
    RAM<> *mem;
    Queue<std::pair<int, int>> delay;
    
    byte tag[LINE_NUM + 2];
    word ind[LINE_NUM + 2];
    byte val[CACHE_SIZE + 2];

    static void decode(addr_t addr, int &idx, int &num, int &off) {
        idx = Decoder::slice(addr, 15, 32);
        num = Decoder::slice(addr, 6, 15);
        off = Decoder::slice(addr, 0, 6);
    }

public:
    Cache() {
        mem = nullptr;
        memset(tag, 0, sizeof(tag));
        memset(ind, 0, sizeof(ind));
        memset(val, 0, sizeof(val));
    }

    void bind(RAM<> *_mem) {mem = _mem; }

    bool hit_byte(addr_t addr) {
        int idx, num, off;
        decode(addr, idx, num, off);
        if(tag[num] == 1 && ind[num] == idx) return 1;
        if(tag[num] == 0) {
            addr_t addr1 = num << 6;
            addr_t addr2 = addr & ~0b111111u;
            for(int i = 0; i < LINE_LEN; ++i) {
                val[addr1 + i] = mem->read_byte(addr2 + i);
            }
            tag[num] = 2, ind[num] = idx;
            delay.push(std::make_pair(num, 4));
        }
        if(tag[num] == 1) {
            byte tmp[LINE_LEN];
            addr_t addr1 = num << 6;
            addr_t addr2 = (ind[num] << 15) | (num << 6); 
            addr_t addr3 = addr & ~0b111111u;
            for(int i = 0; i < LINE_LEN; ++i) {
                mem->write_byte(addr2 + i, val[addr1 + i]);
                val[addr1 + i] = mem->read_byte(addr3 + i);
            }            
            tag[num] = 2, ind[num] = idx;
            delay.push(std::make_pair(num, 4));
        }
        return 0;
    }
    bool hit_hfword(addr_t addr) {
        return hit_byte(addr) && hit_byte(addr + 1);
    }
    bool hit_word(addr_t addr) {
        bool flag = 1;
        flag &= hit_byte(addr);
        flag &= hit_byte(addr + 1); 
        flag &= hit_byte(addr + 2);
        flag &= hit_byte(addr + 3);
        return flag;
    }

    byte read_byte(addr_t addr) {
        int idx, num, off;
        decode(addr, idx, num, off);
        return val[(num << 6) + off];
    }
    hfword read_hfword(addr_t addr) {
        hfword ret = 0;
        ret |= read_byte(addr + 0) << 0;
        ret |= read_byte(addr + 1) << 8;
        return ret;
    }
    word read_word(addr_t addr) {
        word ret = 0;
        ret |= read_byte(addr + 0) << 0;
        ret |= read_byte(addr + 1) << 8;
        ret |= read_byte(addr + 2) << 16;
        ret |= read_byte(addr + 3) << 24;
        return ret;        
    }

    void write_byte(addr_t addr, byte data) {
        int idx, num, off;
        decode(addr, idx, num, off);
        val[(num << 6) + off] = data;
    }
    void write_hfword(addr_t addr, hfword data) {
        write_byte(addr + 0, data >> 0 & 255);
        write_byte(addr + 1, data >> 8 & 255);
    }
    void write_word(addr_t addr, word data) {
        write_byte(addr + 0, data >>  0 & 255);
        write_byte(addr + 1, data >>  8 & 255);
        write_byte(addr + 2, data >> 16 & 255);
        write_byte(addr + 3, data >> 24 & 255);
    }    

    void tick() {
        for(int i = delay.begin(); i != delay.end(); i = delay.next(i)) {
            if(delay[i].second) delay[i].second--;
        }
        while(!delay.empty() && !delay.front().second) {
            tag[delay.front().first] = 1;
            delay.pop();
        }
    }
};

}

#endif