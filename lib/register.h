#ifndef __RISCV_SIMULATOR_REGISTER_H__
#define __RISCV_SIMULATOR_REGISTER_H__

#include "utils.h"
#include <iomanip>

namespace riscv {

template <typename T>
struct Reg_stat {
    T data;
    bool flag;
};

template <typename T>
class Register: public Sequential< Reg_stat<T> > {
public: 
    void init(const T &val, bool tag = 0) {
        this->cur_stat().data = this->nex_stat().data = val;
        this->cur_stat().flag = this->nex_stat().flag = tag;
    }
    bool pending() {return this->cur_stat().flag; }
    void pend(bool stat) {this->nex_stat().flag = stat; }
    T read() {return this->cur_stat().data; }
    void write(const T &val) {this->nex_stat().data = val; }

    void print() {
        std::cout << this->cur_stat().data << " " << this->nex_stat().data << std::endl;
    }

    void flush() {
        this->nex_stat().flag = 0;
    }

};
using Reg = Register<word>;

template <size_t REG_NUM>
struct Rf_stat {
    word val[REG_NUM];
    byte ord[REG_NUM];  
};

template <size_t REG_NUM = 32>
class Regfile: public Sequential < Rf_stat<REG_NUM> > {
public:
    word read(int id) {
        if(id == 0) return 0;
        return this->cur_stat().val[id];
    }
    void write(int id, word data) {
        if(id == 0) return ;
        this->nex_stat().val[id] = data;
    }
    byte order(int id) {
        if(id == 0) return 0;
        return this->cur_stat().ord[id];
    }
    void rename(int id, byte idx) {
        if(id == 0) return ;
        this->nex_stat().ord[id] = idx;
    }
    void reset(int id, byte idx) {
        if(id == 0) return ;
        if(this->nex_stat().ord[id] == idx) {
            this->nex_stat().ord[id] = 0;
        }
    }
    
    void flush() {
        for(int i = 0; i < REG_NUM; ++i) this->nex_stat().ord[i] = 0;
    }

    void print() {
        for(int i = 0; i < 4; ++i) {
            for(int j = 0; j < 8; ++j) {
                std::cout << std::setw(8) << std::setfill('0') << std::hex << read(i*8+j);
                // std::cout << "(#";
                // std::cout << std::setw(2) << std::setfill('0') << std::dec << word(order(i*8+j)) << ")";
                std::cout << " ";
            }
            std::cout << '\n';
        }
    }

};

template <typename T, size_t DELAY_TIME>
class Delay {
private:
    struct lag_stat {
        T data; bool signal; 
        lag_stat(): data(), signal(0) {}
    };
    Register<lag_stat> lag[DELAY_TIME];
    
public:

    void input(const T &data) {
        lag_stat stat;
        stat.data = data, stat.signal = 1;
        lag[0].write(stat);
    }

    bool signaled() {
        return lag[DELAY_TIME - 1].read().signal;
    }

    T output() {
        return lag[DELAY_TIME - 1].read().data;
    }
    
    void tick() {
        for(int i = 0; i < DELAY_TIME - 1; ++i) {
            lag[i + 1].write(lag[i].read());
        }
        for(int i = 0; i < DELAY_TIME; ++i) lag[i].tick();
        lag_stat stat;
        stat.signal = 0;
        lag[0].write(stat);
    }

    void flush() {
        lag_stat stat;
        stat.signal = 0;
        for(int i = 0; i < DELAY_TIME; ++i) {
            lag[i].write(stat), lag[i].tick();
        }
    }

};

}

#endif