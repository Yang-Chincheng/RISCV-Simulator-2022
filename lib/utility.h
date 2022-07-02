#ifndef __RISCV_SIMULATER_UTILITY_H__
#define __RISCV_SIMULATER_UTILITY_H__

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

namespace riscv {

using inst_t = u_int32_t;
using Word = u_int32_t;
using Hfword = u_int16_t;
using Byte = u_int8_t;

using rd_t = Byte;
using rs_t = Byte;
using imm_t = Word;
using opc_t = Byte;
using func_t = Byte;
using addr_t = Word;

void append_inst(std::string &line, std::vector<inst_t> &arr) {
    std::stringstream ss(line);
    while(ss.rdbuf()->in_avail() > 1) {
        inst_t inst = 0x0;
        int base = 0;
        for(int i = 0; i < 4; ++i) {
            Word tmp;
            ss >> std::hex >> tmp;
// std::cout << std::hex << "tmp: " < < tmp << std::endl;
            inst |= tmp << base;
            base += 8;
        }
        arr.push_back(inst);
    }
    return ;
}

const int I_IMM_LEN = 12;
const int S_IMM_LEN = 12;
const int B_IMM_LEN = 13;
const int U_IMM_LEN = 32;
const int J_IMM_LEN = 21;
const int OPC_LEN = 7;
const int RD_LEN = 5;
const int RS_LEN = 5;
const int FUN3_LEN = 3;
const int FUN7_LEN = 7;


inline Word sext(Word data, int len = 32) {
    if(len == 32) return data;
    Word mask = 0xffffffff >> len << len;
    Word ext = (data >> (len - 1) & 1? mask: 0);
    return data & (~mask) | ext;
}

inline inst_t slice(inst_t inst, int lb, int rb) {
    if(rb < 32) inst &= (1u << rb) - 1;
    return inst >> lb;
}

inline opc_t get_opcode(inst_t inst) {
    return slice(inst, 0, 7);
}

inline func_t get_funct3(inst_t inst) {
    return slice(inst, 12, 15);
}
inline func_t get_funct7(inst_t inst) {
    return slice(inst, 25, 32);
}

inline rd_t get_rd(inst_t inst) {
    return slice(inst, 7, 12);
}
inline rd_t get_rs1(inst_t inst) {
    return slice(inst, 15, 20);
}
inline rd_t get_rs2(inst_t inst) {
    return slice(inst, 20, 25);
}

inline imm_t get_imm_I(inst_t inst) {
    return slice(inst, 20, 32);
}
inline imm_t get_imm_S(inst_t inst) {
    imm_t imm = 0;
    imm |= slice(inst,  7, 12) << 0;
    imm |= slice(inst, 25, 32) << 5;
    return imm;
}
inline imm_t get_imm_B(inst_t inst) {
    imm_t imm = 0;
    imm |= slice(inst,  8, 12) << 1;
    imm |= slice(inst, 25, 31) << 5;
    imm |= slice(inst,  7,  8) << 11;
    imm |= slice(inst, 31, 32) << 12;
    return imm;
}
inline imm_t get_imm_U(inst_t inst) {
    return slice(inst, 12, 32) << 12;
}
inline imm_t get_imm_J(inst_t inst) {
    imm_t imm = 0;
    imm |= slice(inst, 21, 31) << 1;
    imm |= slice(inst, 20, 21) << 11;
    imm |= slice(inst, 12, 20) << 12;
    imm |= slice(inst, 31, 32) << 20;
    return imm;
}

}



#endif