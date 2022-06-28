#ifndef __RISCV_SIMULATOR_ALU_H__
#define __RISCV_SIMULATOR_ALU_H__

#include "bus.h"
#include "utils.h"
#include "register.h"

namespace riscv {

class Adder {
public:
    word calc(word opd1, word opd2) {
        return opd1 + opd2;
    }
};

class ALU {
public:
    word calc(RV32I_Opt opt, word opd1, word opd2) {
        switch(opt) {
            case NONE:
                return 0; 
            case ADD: case ADDI:
                return opd1 + opd2;
            case SUB: 
            case BEQ: case BNE:
                return opd1 - opd2;
            case AND: case ANDI:
                return opd1 & opd2;
            case OR: case ORI:
                return opd1 | opd2;
            case XOR: case XORI:
                return opd1 ^ opd2;
            case SLL: case SLLI:
                return opd1 << opd2;
            case SRL: case SRLI:
                return unsigned(opd1) >> opd2;
            case SRA: case SRAI:
                return signed(opd1) >> opd2;
            case SLT: case SLTI: 
            case BLT: case BGE:
                return signed(opd1) < signed(opd2);
            case SLTU: case SLTIU: 
            case BLTU: case BGEU:
                return unsigned(opd1) < unsigned(opd2);
            default:
                return -1;
        }
        return -1;
    }
};

}

#endif