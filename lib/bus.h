#ifndef __RISCV_SIMULATOR_BUS_H__
#define __RISCV_SIMULATOR_BUS_H__

#include "utils.h"

namespace riscv {

template <typename T>
struct Bus_stat {
    T data;
    bool flag;
};

template <typename T>
class Bus: public Sequential< Bus_stat<T> > {
public:
    void send(const T &dat) {
        if(this->nex_stat().flag) return ;
        this->nex_stat().data = dat;
        this->nex_stat().flag = 1;
    }

    bool traffic() {
        return this->cur_stat().flag;
    }

    T recv() {
        this->nex_stat().flag = 0;    
        return this->cur_stat().data;
    }

    void flush() {
        this->nex_stat().flag = 0;
    }
    
};

}

#endif