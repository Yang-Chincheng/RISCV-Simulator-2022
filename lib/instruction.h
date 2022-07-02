#ifndef __RISCV_SIMULATROR_INSTRUCTION_H__
#define __RISCV_SIMULATROR_INSTRUCTION_H__

#include <vector>
#include "storeunit.h"
#include "utility.h"
#include <iomanip>

namespace riscv {

const int REG_NUM = 32;
const int MEM_SIZE = 5e5 + 10;

class Instruction {
private:
    Register pc;
    Register x[REG_NUM];
    Memory<MEM_SIZE> mem;

    void next() {
        pc.write(pc.read() + 4);
    }

    // [U] Load Upper Immediate
    // x[rd] = sext(immediate[31:12] << 12)
    void LUI(rd_t rd, imm_t imm) {
        x[rd].write(sext(imm, U_IMM_LEN));
        next();
//std::cerr << "[LUI] " << std::hex << unsigned(rd) << " " << imm << std::endl;
    }
    // [U] Add Upper Immediate to PC
    // x[rd] = pc + sext(immediate[31:12] << 12)
    void AUIPC(rd_t rd, imm_t imm) {
        x[rd].write(pc.read() + sext(imm, U_IMM_LEN));
        next();
//std::cerr << "[AUIPC] " << std::hex << unsigned(rd) << " " << imm << std::endl;
    }
    // [J] Jump and  Link
    // x[rd] = pc+4; pc += sext(offset)
    void JAL(rd_t rd, imm_t imm) {
        Word tmp = pc.read();
        x[rd].write(tmp + 4);
        pc.write(tmp + sext(imm, J_IMM_LEN));
//std::cerr << "[JAL] " << std::hex << unsigned(rd) << " " << imm << std::endl;
    }
    // [I] Jump and Link Register    
    // t = pc+4; pc=(x[rs1]+sext(offset))&~1; x[rd]=t
    void JALR(rd_t rd, rs_t rs1, imm_t imm) {
        Word tmp = pc.read() + 4;
        pc.write((x[rs1].read() + sext(imm, I_IMM_LEN)) & ~1u);
        x[rd].write(tmp);
//std::cerr << "[JALR] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [B] Branch if Equal
    // if (x[rs1] == x[rs2]) pc += sext(offset)
    void BEQ(rs_t rs1, rs_t rs2, imm_t imm) {
        if(x[rs1].read() == x[rs2].read()) {
            Word tmp = pc.read();
            pc.write(tmp + sext(imm, B_IMM_LEN));
        }
        else next();
//std::cerr << "[BEQ] " << std::hex << unsigned(rs1) << " " << unsigned(rs2) << " " << imm << std::endl;
    }
    // [B] Branch if Not Equal
    // if (x[rs1] ≠ x[rs2]) pc += sext(offset)
    void BNE(rs_t rs1, rs_t rs2, imm_t imm) {
        if(x[rs1].read() != x[rs2].read()) {
            Word tmp = pc.read();
            pc.write(tmp + sext(imm, B_IMM_LEN));
        }
        else next();
//std::cerr << "[BNE] " << std::hex << unsigned(rs1) << " " << unsigned(rs2) << " " << imm << std::endl;
    }
    // [B] Branch if Less Than   
    // if (x[rs1] <_{s} x[rs2]) pc += sext(offset)
    void BLT(rs_t rs1, rs_t rs2, imm_t imm) {
        if(int(x[rs1].read()) < int(x[rs2].read())) {
            Word tmp = pc.read();
            pc.write(tmp + sext(imm, B_IMM_LEN));
        }
        else next();
//std::cerr << "[BLT] " << std::hex << unsigned(rs1) << " " << unsigned(rs2) << " " << imm << std::endl;
    }
    // [B] Branch if Greater Than or Equal   
    // if (x[rs1] ≥_{s} x[rs2]) pc += sext(offset)
    void BGE(rs_t rs1, rs_t rs2, imm_t imm) {
        if(int(x[rs1].read()) >= int(x[rs2].read())) {
            Word tmp = pc.read();
            pc.write(tmp + sext(imm, B_IMM_LEN));
        }
        else next();
//std::cerr << "[BGE] " << std::hex << unsigned(rs1) << " " << unsigned(rs2) << " " << imm << std::endl;
    }
    // [B] Branch if Less Than, Unsigned
    // if (x[rs1] <_{u} x[rs2]) pc += sext(offset)
    void BLTU(rs_t rs1, rs_t rs2, imm_t imm) {
        if(x[rs1].read() < x[rs2].read()) {
            Word tmp = pc.read();
            pc.write(tmp + sext(imm, B_IMM_LEN));
        }
        else next();
//std::cerr << "[BLTU] " << std::hex << unsigned(rs1) << " " << unsigned(rs2) << " " << imm << std::endl;
    }
    // [B] Branch if Greater Than or Equal, Unsigned   
    // if (x[rs1] ≥_{u} x[rs2]) pc += sext(offset)
    void BGEU(rs_t rs1, rs_t rs2, imm_t imm) {
        if(x[rs1].read() >= x[rs2].read()) {
            Word tmp = pc.read();
            pc.write(tmp + sext(imm, B_IMM_LEN));
        }
        else next();
//std::cerr << "[BGEU] " << std::hex << unsigned(rs1) << " " << unsigned(rs2) << " " << imm << std::endl;
    }
    
    // [I] Load Byte  
    // x[rd] = sext(M[x[rs1] + sext(offset)][7:0])
    void LB(rd_t rd, rs_t rs1, imm_t imm) {
        Word tmp1 = x[rs1].read();
        Byte tmp2 = mem.read_byte(tmp1 + sext(imm, I_IMM_LEN));
        x[rd].write(sext(tmp2, 8));
        next();
//std::cerr << "[LB] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [I] Load Halfword
    // x[rd] = sext(M[x[rs1] + sext(offset)][15:0])
    void LH(rd_t rd, rs_t rs1, imm_t imm) {       
        Word tmp1 = x[rs1].read();
        Hfword tmp2 = mem.read_hfword(tmp1 + sext(imm, I_IMM_LEN));
        x[rd].write(sext(tmp2, 16));
        next();
//std::cerr << "[LH] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [I] Load Word
    // x[rd] = sext(M[x[rs1] + sext(offset)][31:0])
    void LW(rd_t rd, rs_t rs1, imm_t imm) {        
        Word tmp1 = x[rs1].read();
        Word tmp2 = mem.read_word(tmp1 + sext(imm, I_IMM_LEN));
        x[rd].write(sext(tmp2, 32));
        next();
//std::cerr << "[LW] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [I] Load Byte  
    // x[rd] = sext(M[x[rs1] + sext(offset)][7:0])
    void LBU(rd_t rd, rs_t rs1, imm_t imm) {
        Word tmp1 = x[rs1].read();
        Byte tmp2 = mem.read_byte(tmp1 + sext(imm, I_IMM_LEN));
        x[rd].write(Word(tmp2));
        next();
//std::cerr << "[LBU] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [I] Load Halfword
    // x[rd] = sext(M[x[rs1] + sext(offset)][15:0])
    void LHU(rd_t rd, rs_t rs1, imm_t imm) {        
        Word tmp1 = x[rs1].read();
        Hfword tmp2 = mem.read_hfword(tmp1 + sext(imm, I_IMM_LEN));
        x[rd].write(Word(tmp2));
        next();
//std::cerr << "[LHU] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }

    // [S] Store Byte
    // M[x[rs1] + sext(offset)] = x[rs2][7: 0] 
    void SB(rs_t rs1, rs_t rs2, imm_t imm) {
        Byte tmp = slice(x[rs2].read(), 0, 8);
        mem.write_byte(x[rs1].read() + sext(imm, S_IMM_LEN), tmp);
        next();
//std::cerr << "[SB] " << std::hex << unsigned(rs1) << " " << unsigned(rs2) << " " << imm << std::endl;
    }
    // [S] Store Halfword
    // M[x[rs1] + sext(offset)] = x[rs2][15: 0]
    void SH(rs_t rs1, rs_t rs2, imm_t imm) {       
        Hfword tmp = slice(x[rs2].read(), 0, 16);
        mem.write_hfword(x[rs1].read() + sext(imm, S_IMM_LEN), tmp);
        next();
//std::cerr << "[SH] " << std::hex << unsigned(rs1) << " " << unsigned(rs2) << " " << imm << std::endl;
    }
    // [S] Store Word
    // M[x[rs1] + sext(offset)] = x[rs2][31: 0]
    void SW(rs_t rs1, rs_t rs2, imm_t imm) {       
        Word tmp = x[rs2].read();
        mem.write_word(x[rs1].read() + sext(imm, S_IMM_LEN), tmp);
        next();
//std::cerr << "[SW] " << std::hex << unsigned(rs1) << " " << unsigned(rs2) << " " << imm << std::endl;
    }

    // [I] Add Immediate
    // x[rd] = x[rs1] + sext(immediate)
    void ADDI(rd_t rd, rs_t rs1, imm_t imm) {
        x[rd].write(x[rs1].read() + sext(imm, I_IMM_LEN));
        next();
//std::cerr << "[ADDI] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [S] Set if Less Than Immediate
    // x[rd] = (x[rs1] <_{s} sext(immediate))
    void SLTI(rd_t rd, rs_t rs1, imm_t imm) {
        x[rd].write(int(x[rs1].read()) < sext(imm, S_IMM_LEN));
        next();
//std::cerr << "[SLTI] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [S] Set if Less Than Immediate, Unsigned
    // x[rd] = (x[rs1] <_{u} sext(immediate))
    void SLTIU(rd_t rd, rs_t rs1, imm_t imm) { 
        x[rd].write(x[rs1].read() < sext(imm, S_IMM_LEN));
        next();
//std::cerr << "[SLTIU] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }

    // [I] Exclusive-OR Immediate
    // x[rd] = x[rs1] ^ sext(immediate)
    void XORI(rd_t rd, rs_t rs1, imm_t imm) {
        x[rd].write(x[rs1].read() ^ sext(imm, I_IMM_LEN));
        next();
//std::cerr << "[XORI] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [I] OR Immediate
    // x[rd] = x[rs1] | sext(immediate)
    void ORI(rd_t rd, rs_t rs1, imm_t imm) {       
        x[rd].write(x[rs1].read() | sext(imm, I_IMM_LEN));
        next();
//std::cerr << "[ORI] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [I] And Immediate
    // x[rd] = x[rs1] & sext(immediate)
    void ANDI(rd_t rd, rs_t rs1, imm_t imm) {       
        x[rd].write(x[rs1].read() & sext(imm, I_IMM_LEN));
        next();
//std::cerr << "[ANDI] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [I] Shift Left Logical Immediate
    // x[rd] = x[rs1] << shamt
    void SLLI(rd_t rd, rs_t rs1, imm_t imm) {       
        imm = slice(imm, 0, 5);
        x[rd].write(x[rs1].read() << imm);
        next();
//std::cerr << "[SLLI] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [I] Shift Rights Logical Immediate
    // x[rd] = x[rs1] >> shamt
    void SRLI(rd_t rd, rs_t rs1, imm_t imm) {       
        imm = slice(imm, 0, 5);
        x[rd].write(x[rs1].read() >> imm);
        next();
//std::cerr << "[SRLI] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }
    // [I] Shift Rights Arithmetic Immediate
    // x[rd] = x[rs1] >>_{u} shamt
    void SRAI(rd_t rd, rs_t rs1, imm_t imm) {        
        imm = slice(imm, 0, 5);
        x[rd].write(int(x[rs1].read()) >> imm);
        next();
//std::cerr << "[SRAI] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << imm << std::endl;
    }

    // [R] Add
    // x[rd] = x[rs1] + x[rs2]
    void ADD(rd_t rd, rs_t rs1, rs_t rs2) {
        x[rd].write(x[rs1].read() + x[rs2].read());
        next();
//std::cerr << "[ADD] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << unsigned(rs2) << std::endl;
    }
    // [R] Substract
    // x[rd] = x[rs1] - x[rs2]
    void SUB(rd_t rd, rs_t rs1, rs_t rs2) {
        x[rd].write(x[rs1].read() - x[rs2].read());    
        next();    
//std::cerr << "[SUB] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << unsigned(rs2) << std::endl;
    }
    // [R] Shift Left Logical
    // x[rd] = x[rs1] << x[rs2]
    void SLL(rd_t rd, rs_t rs1, rs_t rs2) {
        Byte tmp = slice(x[rs2].read(), 0, 5);
        x[rd].write(x[rs1].read() << tmp);
        next();
//std::cerr << "[SLL] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << unsigned(rs2) << std::endl;
    }
    // [R] Set if Less Than
    // x[rd] = x[rs1] <_{s} x[rs2]
    void SLT(rd_t rd, rs_t rs1, rs_t rs2) {
        x[rd].write(int(x[rs1].read()) < int(x[rs2].read()));
        next();
//std::cerr << "[SLT] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << unsigned(rs2) << std::endl;
    }
    // [R] Set if Less Than, Unsigned
    // x[rd] = x[rs1] <_{u} x[rs2]
    void SLTU(rd_t rd, rs_t rs1, rs_t rs2) {
        x[rd].write(x[rs1].read() < x[rs2].read());
        next();
//std::cerr << "[SLTU] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << unsigned(rs2) << std::endl;
    }
    // [R] Shift Right Logical
    // x[rd] = x[rs1] >>_{u} x[rs2]
    void SRL(rd_t rd, rs_t rs1, rs_t rs2) {
        Byte tmp = slice(x[rs2].read(), 0, 5);
        x[rd].write(x[rs1].read() >> tmp);
        next();
std::cout << "[SRL] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << unsigned(rs2) << std::endl;
    }
    // [R] Shift Right Arithmetic
    // x[rd] = x[rs1] >>_{s} x[rs2]
    void SRA(rd_t rd, rs_t rs1, rs_t rs2) {
        Byte tmp = slice(x[rs2].read(), 0, 5);
        x[rd].write(int(x[rs1].read()) >> tmp);
        next();
//std::cerr << "[SRA] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << unsigned(rs2) << std::endl;
    }
    // [R] Exclusive-OR
    // x[rd] = x[rs1] ^ x[rs2]    
    void XOR(rd_t rd, rs_t rs1, rs_t rs2) {
        x[rd].write(x[rs1].read() ^ x[rs2].read());
        next();
//std::cerr << "[XOR] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << unsigned(rs2) << std::endl;
    }
    // [R] OR
    // x[rd] = x[rs1] | x[rs2]
    void OR(rd_t rd, rs_t rs1, rs_t rs2) {
        x[rd].write(x[rs1].read() | x[rs2].read());
        next();
//std::cerr << "[OR] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << unsigned(rs2) << std::endl;
    }
    // [R] And
    // x[rd] = x[rs1] & x[rs2]
    void AND(rd_t rd, rs_t rs1, rs_t rs2) {
        x[rd].write(x[rs1].read() & x[rs2].read());
        next();
//std::cerr << "[AND] " << std::hex << unsigned(rd) << " " << unsigned(rs1) << " " << unsigned(rs2) << std::endl;
    }

public:

    Instruction() {
        pc.setid(1);
        for(int i = 0; i < REG_NUM; ++i) x[i].setid(i);
    }

    void init(addr_t addr, std::vector<inst_t> &arr) {
        for(auto ins: arr) {
//std::cerr << "place [" << std::setfill('0') << std::setw(8) << std::hex << ins << "] at ";
//std::cerr << std::setfill('0') << std::setw(8) << std::hex << addr << std::endl;
            mem.write_word(addr, ins);
            addr += 4;
        }
    }
    
    void execute(off_t offset) {
        pc.write(offset);

int cnt = 100;
int tot = 0;
        while(1) {
//std::cerr << ">> now pc: " << pc.read() << std::endl; 
            inst_t inst = mem.read_word(pc.read());
            if(inst == 0x0ff00513) break;
// std::cout << "off: " << offset << std::hex << " " << inst << std::endl;
            opc_t opc = get_opcode(inst);
// std::cout << "opcode: " << std::hex << unsigned(opc) << std::endl;
            if(opc == 0x37) LUI(get_rd(inst), get_imm_U(inst));
            else if(opc == 0x17) AUIPC(get_rd(inst), get_imm_U(inst));
            else if(opc == 0x6F) JAL(get_rd(inst), get_imm_J(inst));
            else if(opc == 0x67) JALR(get_rd(inst), get_rs1(inst), get_imm_I(inst));
            else if(opc == 0x63) {
                rs_t rs1 = get_rs1(inst);
                rs_t rs2 = get_rs2(inst);
                imm_t imm = get_imm_B(inst);
                func_t funct3 = get_funct3(inst);
                if(funct3 == 0x0) BEQ(rs1, rs2, imm);
                else if(funct3 == 0x1) BNE(rs1, rs2, imm);
                else if(funct3 == 0x4) BLT(rs1, rs2, imm);
                else if(funct3 == 0x5) BGE(rs1, rs2, imm);
                else if(funct3 == 0x6) BLTU(rs1, rs2, imm);
                else if(funct3 == 0x7) BGEU(rs1, rs2, imm);
            }
            else if(opc == 0x03) {
                rd_t rd = get_rd(inst);
                rs_t rs1 = get_rs1(inst);
                imm_t imm = get_imm_I(inst);
                func_t funct3 = get_funct3(inst);
                if(funct3 == 0x0) LB(rd, rs1, imm);
                else if(funct3 == 0x1) LH(rd, rs1, imm);
                else if(funct3 == 0x2) LW(rd, rs1, imm);
                else if(funct3 == 0x4) LBU(rd, rs1, imm);
                else if(funct3 == 0x5) LHU(rd, rs1, imm);
            }
            else if(opc == 0x23) {
                rs_t rs1 = get_rs1(inst);
                rs_t rs2 = get_rs2(inst);
                imm_t imm = get_imm_S(inst);
                func_t funct3 = get_funct3(inst);
                if(funct3 == 0x0) SB(rs1, rs2, imm);
                else if(funct3 == 0x1) SH(rs1, rs2, imm);
                else if(funct3 == 0x2) SW(rs1, rs2, imm);            
            }
            else if(opc == 0x13) {
                func_t funct3 = get_funct3(inst);
                if(funct3 == 0x2) SLTI(get_rd(inst), get_rs1(inst), get_imm_S(inst));
                else if(funct3 == 0x3) SLTIU(get_rd(inst), get_rs1(inst), get_imm_S(inst));
                else {
                    rd_t rd = get_rd(inst);
                    rs_t rs1 = get_rs1(inst);
                    imm_t imm = get_imm_I(inst);
                    if(funct3 == 0x0) ADDI(rd, rs1, imm);
                    else if(funct3 == 0x4) XORI(rd, rs1, imm);
                    else if(funct3 == 0x6) ORI(rd, rs1, imm);            
                    else if(funct3 == 0x7) ANDI(rd, rs1, imm);
                    else if(funct3 == 0x1) SLLI(rd, rs1, imm);
                    else if(funct3 == 0x5) {
                        func_t funct7 = get_funct7(inst);
                        if(funct7 == 0x00) SRLI(rd, rs1, imm);
                        else if(funct7 == 0x20) SRAI(rd, rs1, imm);
                    }
                }
            }
            else if(opc == 0x33) {
                rd_t rd = get_rd(inst);
                rs_t rs1 = get_rs1(inst);
                rs_t rs2 = get_rs2(inst);
                func_t funct3 = get_funct3(inst);
                if(funct3 == 0x0) {
                    func_t funct7 = get_funct7(inst);
                    if(funct7 == 0x00) ADD(rd, rs1, rs2);
                    else if(funct7 == 0x20) SUB(rd, rs1, rs2);
                }
                else if(funct3 == 0x1) SLL(rd, rs1, rs2);
                else if(funct3 == 0x2) SLT(rd, rs1, rs2);
                else if(funct3 == 0x3) SLTU(rd, rs1, rs2);
                else if(funct3 == 0x4) XOR(rd, rs1, rs2);            
                else if(funct3 == 0x5) {
                    func_t funct7 = get_funct7(inst);
                    if(funct7 == 0x00) SRL(rd, rs1, rs2);
                    else if(funct7 == 0x20) SRA(rd, rs1, rs2);                
                }
                else if(funct3 == 0x6) OR(rd, rs1, rs2);
                else if(funct3 == 0x7) AND(rd, rs1, rs2);
            }
tot++;
// std::cout << "[commit code] " << std::setw(8) << std::setfill('0') << std::hex << Word(inst) << " #" << std::dec << tot << std::endl;
// // std::cout << "[pc] " << std::setw(8) << std::setfill('0') << std::hex << Word(pc.read()) << std::endl;

// for(int i = 0; i < 4; ++i) {
//     for(int j = 0; j < 8; ++j) std::cout << std::setw(8) << std::setfill('0') << std::hex << Word(x[i*8+j].read()) << " ";
//     std::cout << std::endl;
// }
// std::cout << std::hex << std::setw(8) << std::setfill('0') << mem.read_word(4572) << std::endl;

        }
        std::cout << std::dec << (x[10].read() & 255u) << std::endl;
    }

};

}

#endif