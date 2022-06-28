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
    bool blocked;
    T cur, nex;
    T& cur_stat() {return cur; }
    T& nex_stat() {return nex; }

public:
    bool busy() {return blocked; }
    bool ready() {return !blocked; }
    void stall() {blocked = 1; }
    void resume() {blocked = 0; }
    void tick() {
        if(!blocked) cur = nex; 
    }

};

template <typename T, size_t MAX_LEN = 32>
class Queue {
protected:
    int len;
    int head, tail;
    T que[MAX_LEN];
    
public:
    Queue(): len(0), head(0), tail(0) {}
    
    void push(const T &ele) {
        tail = (tail + 1) % MAX_LEN;
        que[tail] = ele, len++;
    }
    void pop() {
        head = (head + 1) % MAX_LEN;
        len--;
    }
    T front() {
        return que[(head + 1) % MAX_LEN];
    }

    void clear() {len = head = tail = 0; }
    bool empty() {return len == 0; }
    bool full() {return len == MAX_LEN; }
    int length() {return len; }

    T& at(int idx) {return que[idx]; }
    T& operator [] (int idx) {return que[(head + idx) % MAX_LEN]; }

};

template <typename T, size_t MAX_LEN = 32>
class List {
protected:
    int size;
    int last;
    T list[MAX_LEN];
    int pre[MAX_LEN];

    class iterator;
    friend class iterator;

public:
    List() {
        size = 0, last = -1;
        for(int i = 0; i < MAX_LEN; ++i) pre[i] = -1; 
    }

    int insert(const T &ele) {
        if(size == MAX_LEN) return -1;
        for(int i = 0; i < MAX_LEN; ++i) {
            if(pre[i] == -1) {
                list[i] = ele;
                pre[i] = last, last = i;
                size++;
                return i;
            }
        }
        return -1;
    }

    int length() {return size; }
    bool empty() {return size == 0; }
    bool full() {return size == MAX_LEN; }

    void clear() {
        size = 0, last = -1;
        for(int i = 0; i < MAX_LEN; ++i) pre[i] = -1;
    }

    T& at(int idx) {return list[idx]; }
    T& operator [] (int idx) {
        int pos = last;
        for(int i = 0; i < idx; ++i) pos = pre[pos];
        return list[pos];
    }
    
    class iterator {
    private:
        friend List;
        int idx;
    public:
        iterator(): idx(-1) {}
        iterator(int _idx): idx(_idx) {}
        iterator& operator ++ () {idx = pre[idx]; }
        T& operator * () const {return list[idx]; }
        T* operator -> () const {return list + idx; }
        bool operator == (const iterator &o) const {idx == o.idx; }
        bool operator != (const iterator &o) const {idx != o.idx; }
    };

    iterator begin() {return iterator(last); }
    iterator end() {return iterator(-1); }

    int remove(iterator iter) {
        int idx = iter.idx;
        if(idx == last) {
            size--;
            last = pre[last];
            return last;
        }
        for(int i = last; i != -1; i = pre[i]) {
            if(pre[i] == idx) {
                pre[i] = pre[pre[i]];
                size--;
                return pre[i];
            }
        }
        return -1;
    }

};

}

#endif