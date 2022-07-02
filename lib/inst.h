#ifndef __RISCV_SIMULATOR_INST_H__
#define __RISCV_SIMULATOR_INST_H__

#include "utils.h"
#include <string>

namespace riscv {

enum RV32I_Opt {
    NONE = 0, HALT,
    
    LUI, AUIPC, 
    
    JUMP_BEG = 16, 
        JAL, JALR,
    JUMP_END,
    
    BRANCH_BEG = 32,
        BEQ, BNE, BLT, BGE, BLTU, BGEU, 
    BRANCH_END,

    LOAD_BEG = 48,
        LB, LH, LW, LBU, LHU,
    LOAD_END,

    STORE_BEG = 64,
        SB, SH, SW,
    STORE_END,

    IMM_BEG = 80,
        ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, 
    IMM_END,

    REG_BE = 96,
        ADD, SUB, SLL, SLT, SLTU, SRL, SRA, XOR, OR, AND,
    REG_END
    
};

std::string opt_to_string(RV32I_Opt opt) {
    switch(opt) {
        case LUI: return "lui";
        case AUIPC: return "auipc";
        case JAL: return "jal";
        case JALR: return "jalr";
        case BEQ: return "beq";
        case BNE: return "bne";
        case BLT: return "blt";
        case BGE: return "bge";
        case BLTU: return "bltu";
        case BGEU: return "bgeu";
        case LB: return "lb";
        case LH: return "lh";
        case LW: return "lw";
        case LBU: return "lbu";
        case LHU: return "lhu";
        case SB: return "sb";
        case SH: return "sh";
        case SW: return "sw";
        case ADDI: return "addi";
        case SLTI: return "slti";
        case SLTIU: return "sltiu";
        case XORI: return "xori";
        case ORI: return "ori";
        case ANDI: return "andi";
        case SLLI: return "slli";
        case SRLI: return "srli";
        case SRAI: return "srai";
        case ADD: return "add";
        case SUB: return "sub";
        case SLL: return "sll";
        case SLT: return "slt";
        case SLTU: return "sltu";
        case SRL: return "srl";
        case SRA: return "sra";
        case XOR: return "xor";
        case OR: return "or";
        case AND: return "and";
        default: return "none"; 
    }
}

class Decoder {
public:
    inst_t org;
    opc_t opc;
    rid_t rd, rs1, rs2;
    imm_t imm;
    func_t funct3, funct7;

    char type;
    RV32I_Opt opt;

    static word sext(word data, int len = 32) {
        if(len == 32) return data;
        word mask = 0xffffffff >> len << len;
        word ext = (data >> (len - 1) & 1? mask: 0);
        return data & (~mask) | ext;
    }

    static inst_t slice(inst_t inst, int lb, int rb) {
        if(rb < 32) inst &= (1u << rb) - 1;
        return inst >> lb;
    }

private:
    const static int I_IMM_LEN = 12;
    const static int S_IMM_LEN = 12;
    const static int B_IMM_LEN = 13;
    const static int U_IMM_LEN = 32;
    const static int J_IMM_LEN = 21;
    const static int OPC_LEN = 7;
    const static int RD_LEN = 5;
    const static int RS_LEN = 5;
    const static int FUN3_LEN = 3;
    const static int FUN7_LEN = 7;


    static opc_t get_opcode(inst_t inst) {
        return slice(inst, 0, 7);
    }

    static func_t get_funct3(inst_t inst) {
        return slice(inst, 12, 15);
    }
    static func_t get_funct7(inst_t inst) {
        return slice(inst, 25, 32);
    }

    static rid_t get_rd(inst_t inst) {
        return slice(inst, 7, 12);
    }
    static rid_t get_rs1(inst_t inst) {
        return slice(inst, 15, 20);
    }
    static rid_t get_rs2(inst_t inst) {
        return slice(inst, 20, 25);
    }

    static imm_t get_imm_I(inst_t inst) {
        return sext(slice(inst, 20, 32), I_IMM_LEN);
    }
    static imm_t get_imm_S(inst_t inst) {
        imm_t imm = 0;
        imm |= slice(inst,  7, 12) << 0;
        imm |= slice(inst, 25, 32) << 5;
        return sext(imm, S_IMM_LEN);
    }
    static imm_t get_imm_B(inst_t inst) {
        imm_t imm = 0;
        imm |= slice(inst,  8, 12) << 1;
        imm |= slice(inst, 25, 31) << 5;
        imm |= slice(inst,  7,  8) << 11;
        imm |= slice(inst, 31, 32) << 12;
        return sext(imm, B_IMM_LEN);
    }
    static imm_t get_imm_U(inst_t inst) {
        return sext(slice(inst, 12, 32) << 12, U_IMM_LEN);
    }
    static imm_t get_imm_J(inst_t inst) {
        imm_t imm = 0;
        imm |= slice(inst, 21, 31) << 1;
        imm |= slice(inst, 20, 21) << 11;
        imm |= slice(inst, 12, 20) << 12;
        imm |= slice(inst, 31, 32) << 20;
        return sext(imm, J_IMM_LEN);
    }

public:
    void decode(inst_t inst) {
        org = inst;
        opc = get_opcode(inst);
        switch(opc) {
            case 0x37:
                opt = LUI, type = 'U';
                rd = get_rd(inst);
                imm = get_imm_U(inst);
                break;
            case 0x17:
                opt = AUIPC, type = 'U';
                rd = get_rd(inst);
                imm = get_imm_U(inst);
                break;
            case 0x6f:
                opt = JAL, type = 'J';
                rd = get_rd(inst);
                imm = get_imm_J(inst);
                break;
            case 0x67:
                opt = JALR, type = 'I';
                rd = get_rd(inst);
                rs1 = get_rs1(inst);
                imm = get_imm_I(inst);
                break;
            case 0x63:
                type = 'B';
                rs1 = get_rs1(inst);
                rs2 = get_rs2(inst);
                imm = get_imm_B(inst);
                funct3 = get_funct3(inst);
                switch(funct3) {
                    case 0x0: opt = BEQ; break;
                    case 0x1: opt = BNE; break;
                    case 0x4: opt = BLT; break;
                    case 0x5: opt = BGE; break;
                    case 0x6: opt = BLTU; break;
                    case 0x7: opt = BGEU; break;
                }
                break;
            case 0x03:
                type = 'I';
                rd = get_rd(inst);
                rs1 = get_rs1(inst);
                imm = get_imm_I(inst);
                funct3 = get_funct3(inst);
                switch(funct3) {
                    case 0x0: opt = LB; break;
                    case 0x1: opt = LH; break;
                    case 0x2: opt = LW; break;
                    case 0x4: opt = LBU; break;
                    case 0x5: opt = LHU; break;
                }
                break;
            case 0x23:
                type = 'S';
                rs1 = get_rs1(inst);
                rs2 = get_rs2(inst);
                imm = get_imm_S(inst);
                funct3 = get_funct3(inst);
                switch(funct3) {
                    case 0x0: opt = SB; break;
                    case 0x1: opt = SH; break;
                    case 0x2: opt = SW; break; 
                }
                break;
            case 0x13:
                funct3 = get_funct3(inst);
                rd = get_rd(inst);
                rs1 = get_rs1(inst);
                switch(funct3) {
                    case 0x0:
                        opt = ADDI, type = 'I';
                        imm = get_imm_I(inst);
                        break;
                    case 0x1:
                        opt = SLLI, type = 'I';
                        imm = get_imm_I(inst);
                        break;
                    case 0x2: 
                        opt = SLTI, type = 'S';
                        imm = get_imm_S(inst);
                        break;
                    case 0x3:
                        opt = SLTIU, type = 'S';
                        imm = get_imm_S(inst);
                        break;
                    case 0x4:
                        opt = XORI, type = 'I';
                        imm = get_imm_I(inst);
                        break;
                    case 0x5:
                        type = 'I';
                        imm = get_imm_I(inst);
                        funct7 = get_funct7(inst);
                        opt = funct7? SRAI: SRLI;
                        break;
                    case 0x6:
                        opt = ORI, type = 'I';
                        imm = get_imm_I(inst);
                        break;
                    case 0x7:
                        opt = ANDI, type = 'I';
                        imm = get_imm_I(inst);
                        break;
                }
                break;
            case 0x33:
                type = 'R';
                rd = get_rd(inst);
                rs1 = get_rs1(inst);
                rs2 = get_rs2(inst);
                funct3 = get_funct3(inst);
                switch(funct3) {
                    case 0x0:
                        funct7 = get_funct7(inst);
                        opt = funct7? SUB: ADD;
                        break;
                    case 0x1: opt = SLL; break;
                    case 0x2: opt = SLT; break;
                    case 0x3: opt = SLTU; break;
                    case 0x4: opt = XOR; break;
                    case 0x5: 
                        funct7 = get_funct7(inst);
                        opt = funct7? SRA: SRL;
                        break;
                    case 0x6: opt = OR; break;
                    case 0x7: opt = AND; break;
                }
                break;
            default:
                opt = NONE, type = 'N';
                break;
        }
    }

};

}

#endif