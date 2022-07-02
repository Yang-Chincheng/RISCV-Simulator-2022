#ifndef __RISCV_SIMULATOR_UTILITY_H__
#define __RISCV_SIMULATOR_UTILITY_H__

#include <iostream>

namespace riscv {

using bit = bool;
using byte = u_int8_t;
using word = u_int32_t;
using hfword = u_int16_t;

using off_t = u_int32_t;
using addr_t = u_int32_t;

using inst_t = word;
using imm_t = word;
using opc_t = byte;
using rid_t = byte;
using func_t = byte;

template <typename T>
class Sequential {
protected:
    // bool flag;
    T cur, nex;
    T& cur_stat() {return cur; }
    T& nex_stat() {return nex; }

public:
    Sequential(): cur(), nex() {}
    // bool stalled() {return flag; }
    // void stall(bool stat) {flag = stat; }
    void tick() {cur = nex; }

};

class Stall: public Sequential <bool> {
public:
    void init(bool flag) {
        this->cur_stat() = this->nex_stat() = flag;
    }
    void set(bool flag) {
        if(flag) this->cur_stat() = 1;
        this->nex_stat() = flag;
    }
    bool get() {return this->cur_stat(); }
};

struct Counter: public Sequential<int> {
public:
    void init(int x) {this->cur_stat() = this->nex_stat() = x; }
    void inc() {this->nex_stat()++; }
    void dec() {this->nex_stat()--; }
    void set(int x) {this->nex_stat() = x; }
    int count() {return this->cur_stat(); }
};

template <typename T, size_t MAX_LEN = 32>
class Queue {
protected:
    int len;
    int head, tail;
    T que[MAX_LEN];
    
public:
    Queue(): len(0), head(0), tail(0) {}
    
    int allocate() {
        if(len >= MAX_LEN - 1) return -1;
        tail = (tail + 1) % MAX_LEN;
        len++; return tail;
    }
    void push(const T &ele) {
        int pos = allocate();
        if(~pos) que[pos] = ele;
    }
    void pop() {
        head = (head + 1) % MAX_LEN;
        len--;
    }
    T& front() {
        return que[(head + 1) % MAX_LEN];
    }

    int begin() {return (head + 1) % MAX_LEN; }
    int end() {return (tail + 1) % MAX_LEN; }
    int next(int idx) {return (idx + 1) % MAX_LEN; }

    bool inque(int pos) {
        if(!len) return 0;
        if(tail > head && (pos <= head || pos > tail)) return 0; 
        if(tail < head && (pos > tail && pos <= head)) return 0;
        return 1;
    }
    void clear() {len = head = tail = 0; }
    bool empty() {return len == 0; }
    bool full() {return len >= MAX_LEN - 1; }
    int length() {return len; }

    T& operator [] (int idx) {return que[idx]; }

};

template <typename T, size_t MAX_LEN = 32>
class List {
protected:
    int size;
    T list[MAX_LEN];
    bool flag[MAX_LEN];

public:
    List() {
        size = 0;
        for(int i = 1; i < MAX_LEN; ++i) flag[i] = 0;
    }

    int allocate() {
        for(int i = 1; i < MAX_LEN; ++i) {
            if(!flag[i]) {
                flag[i] = 1; 
                size++; return i;
            }
        }
        return -1;
    }
    int deallocate(int pos) {
        if(!flag[pos]) return 0;
        flag[pos] = 0, size--; return 1;
    }

    int length() {return size; }
    bool empty() {return size == 0; }
    bool full() {return size >= MAX_LEN - 1; }
    void clear() {
        size = 0;
        for(int i = 1; i < MAX_LEN; ++i) flag[i] = 0;
    }

    int next(int pos) {
        for(int i = pos + 1; i < MAX_LEN; ++i) {
            if(flag[i]) return i;
        }
        return -1;
    }

    bool inlist(int pos) {
        return flag[pos];
    }
    T& operator [] (int idx) {return list[idx]; }

};

template <typename T, size_t MAX_LEN = 32>
class SeqQueue: public Sequential< Queue<T, MAX_LEN> > {
public:
    void push(const T &node) {
        this->nex_stat().push(node);
    }
    void pop() {
        this->nex_stat().pop();
    }
    T front() {
        return this->cur_stat().front();
    }
    bool empty() {
        return this->cur_stat().empty();
    }
    bool full() {
        return this->cur_stat().full();
    }
    void flush() {
        this->nex_stat().clear();
    }
};

}

#endif