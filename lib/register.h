#ifndef __RISCV_SIMULATOR_REGISTER_H__
#define __RISCV_SIMULATOR_REGISTER_H__

#include "utils.h"

namespace riscv {

template <typename T>
class Register: public Temporal<T> {
public:
    T read() {return cur_stat(); }
    void write(const T &val) {nex_stat() = val; }

};
using Reg = Register<word>;

template <size_t REG_NUM = 32>
class Regfile {
private:
    Register<word> V[REG_NUM];
    Register<byte> Q[REG_NUM];

public:
    word read(int id) {return V[id].read(); }
    void write(int id, word val) {V[id].write(val); }
    void tick() {
        for(int i = 0; i < REG_NUM; ++i) V[i].tick();
    }

};

}

#endif