#ifndef __RISCV_SIMULATOR_H__
#define __RISCV_SIMULATOR_H__

#include "../lib/alu.h"
#include "../lib/bus.h"
#include "../lib/register.h"
#include "../lib/inst.h"
#include "../lib/ram.h"
#include "../lib/utils.h"
#include <tuple>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>

namespace riscv {

using CDB_msg = std::tuple<byte, word, addr_t>;
using CDB_reg = Register<CDB_msg>;
using Store_msg = std::tuple<RV32I_Opt, word, addr_t>;
using Load_msg = std::tuple<RV32I_Opt, word, word>;

struct Buffer_item;
struct ROB_item;
struct InstQue_node;
class RS;
class ROB;
class SLB;

struct Buffer_item {
    byte ROBidx;
    RV32I_Opt opt;
    word val1, val2;
    byte src1, src2;
    imm_t imm;

    bool ready() {
        return !src1 && !src2; 
    }
    bool match(byte tag) {
        return src1 == tag || src2 == tag;
    }
    void update(int tag, word data) {
        if(src1 == tag) src1 = 0, val1 = data;
        if(src2 == tag) src2 = 0, val2 = data;
    }    
};

class RS: public Sequential< List<Buffer_item, 16> > {
public:

    bool empty() {return this->cur_stat().empty(); }
    bool full() {return this->cur_stat().full(); }
    int size() {return this->cur_stat().length(); }

    void issue(const Buffer_item &item) {
        int pos = this->nex_stat().allocate();
        this->nex_stat()[pos] = item;
    }

    const Buffer_item* execute(CDB_reg &out) {
        if(out.pending()) return nullptr;
        auto &clis = this->cur_stat();
        for(int i = clis.next(0); ~i; i = clis.next(i)) {
            if(clis[i].ready()) {
                this->nex_stat().deallocate(i);
                return &clis[i];
            }
        }
        return nullptr;
    }

    void update(byte idx, word val) {
        auto &clis = this->cur_stat();
        auto &nlis = this->nex_stat();
        for(int i = clis.next(0); ~i; i = clis.next(i)) {
            if(clis[i].match(idx)) {
                if(nlis.inlist(i)) nlis[i].update(idx, val);
            }
        }
    }

    void flush() {
        this->nex_stat().clear();
    }

    void print() {
        auto cur = this->cur_stat();
        if(cur.empty()) std::cout << "empty\n";
        for(int i = cur.next(0); i != -1; i = cur.next(i)) {
            auto x = cur[i];
            std::cout << "#" << std::setw(2) << std::setfill('0') << std::dec << word(x.ROBidx) << ' ';
            std::cout << std::setw(5) << std::setfill(' ') << opt_to_string(x.opt) << ' ';
            std::cout << '(';
            std::cout << "#" << std::setw(4) << std::setfill('0') << std::dec << word(x.src1) << ", ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << word(x.val1) << ") ";
            std::cout << '(';
            std::cout << "#" << std::setw(4) << std::setfill('0') << std::dec << word(x.src2) << ", ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << word(x.val2) << ") ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << word(x.imm)<< '\n';
        }
    }

}; 

class SLB: public Sequential< Queue<Buffer_item, 16> > {
public:
    bool full() {return this->cur_stat().full(); }
    bool empty() {return this->cur_stat().empty(); }
    int size() {return this->cur_stat().length(); }
    
    void issue(const Buffer_item &item) {
        this->nex_stat().push(item);
    }

    const Buffer_item* execute(CDB_reg &store, CDB_reg &load, int cnt) {        
        auto &cque = this->cur_stat();
        if(cque.empty()) return nullptr;
        auto &item = cque.front();
        if(item.ready()) {
            if(item.opt > LOAD_BEG && item.opt < LOAD_END) {
                if(!load.pending() && cnt == 0) {
                    this->nex_stat().pop(); return &item;
                }
            }
            else {
                if(!store.pending()) {
                    this->nex_stat().pop(); return &item;
                }
            }
        }
        return nullptr;
    }
    
    void update(byte idx, word data) {
        auto &cque = this->cur_stat();
        auto &nque = this->nex_stat();
        for(int i = cque.begin(); i != cque.end(); i = cque.next(i)) {
            if(cque[i].match(idx)) {
                if(nque.inque(i)) nque[i].update(idx, data);
            }
        }
    }

    void flush() {
        this->nex_stat().clear();
    }

    void print() {
        auto cur = this->cur_stat();
        if(cur.empty()) std::cout << "empty\n";
        for(int i = cur.begin(); i != cur.end(); i = cur.next(i)) {
            auto x = cur[i];
            std::cout << "#" << std::setw(2) << std::setfill('0') << std::dec << word(x.ROBidx) << ' ';
            std::cout << std::setw(5) << std::setfill(' ') << opt_to_string(x.opt) << ' ';
            std::cout << '(';
            std::cout << "#" << std::setw(4) << std::setfill('0') << std::dec << word(x.src1) << ", ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << word(x.val1) << ") ";
            std::cout << '(';
            std::cout << "#" << std::setw(4) << std::setfill('0') << std::dec << word(x.src2) << ", ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << word(x.val2) << ") ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << word(x.imm) << '\n';
        }
    }

};

struct ROB_item {
    byte idx;
    inst_t org;
    RV32I_Opt opt;
    int cnt;
    word dest;
    word data;
    word addr;
    
    addr_t cur_pc;
    addr_t nex_pc, mis_pc;
    bool jump;

};

class ROB: public Sequential< Queue<ROB_item, 16> > {
public:
    bool empty() {return this->cur_stat().empty(); }
    bool full() {return this->cur_stat().full(); }

    int allocate() {
        return this->nex_stat().allocate();
    }

    bool ready(int idx) {
        if(!this->cur_stat().inque(idx - 1)) return 0;
        return this->cur_stat()[idx - 1].cnt == 0;
    }
    word value(int idx) {
        return this->cur_stat()[idx - 1].data;
    }

    void issue(int idx, const ROB_item &item) {
        if(!this->nex_stat().inque(idx - 1)) return ;
        this->nex_stat()[idx - 1] = item;
    }

    const ROB_item* commit() {
        if(this->cur_stat().empty()) return nullptr;
        auto &item = this->cur_stat().front();
        if(item.cnt == 0) {
            this->nex_stat().pop();
            return &item;
        }
        return nullptr;
    }

    void update(byte idx, word data, addr_t addr) {
        if(this->nex_stat().inque(idx - 1)) {
            this->nex_stat()[idx - 1].cnt--;
            this->nex_stat()[idx - 1].data = data;
            this->nex_stat()[idx - 1].addr = addr;
        }
    }

    void flush() {
        this->nex_stat().clear();
    }

    void print() {
        auto cur = this->cur_stat();
        if(cur.empty()) std::cout << "empty\n";
        for(int i = cur.begin(); i != cur.end(); i = cur.next(i)) {
            auto x = cur[i];
            std::cout << "#" << std::setw(2) << std::setfill('0') << std::dec << i + 1 << ' ';
            std::cout << std::setw(8) << std::setfill('0') << std::hex << x.org << ' ';
            std::cout << std::setw(5) << std::setfill(' ') << opt_to_string(x.opt) << ' ';
            std::cout << "#" << std::dec << std::setw(4) << std::setfill('0') << x.dest << ' ';
            std::cout << std::setw(8) << std::setfill('0') << std::hex << x.data << ' ';
            std::cout << std::setw(8) << std::setfill('0') << std::hex << x.addr << ' ';
            std::cout << '(';
            std::cout << std::setw(8) << std::setfill('0') << std::hex << x.cur_pc << ", ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << x.nex_pc << ", ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << x.mis_pc << ", ";
            std::cout << (x.jump? "1": "0") << ") ";
            std::cout << x.cnt << '\n';
        }
    }

};

struct InstQue_node {
    inst_t inst;
    addr_t pc, nex_pc, mis_pc;
    bool jump;
};

class Speculation {
private:
    const static int HASH_SIZE = 4096; 
    byte table[4][HASH_SIZE];
    byte history[HASH_SIZE];
    int total;
    int success;

    word hash(addr_t pc) {
        return ((pc >> 12) ^ (pc >> 2)) & 0xfff;
    }

public:
    Speculation() {
        total = success = 0;
        memset(table, 2, sizeof(table));
        memset(history, 0, sizeof(history));
    }

    bool predict(addr_t pc) {
        byte key = hash(pc);
        return table[history[key]][key] >= 2;
    }

    void feedback(addr_t pc, bool jump, bool mis) {
        if(!mis) success++; total++;
        byte key = hash(pc); 
        byte &tab = table[history[key]][key];
        if(jump) tab < 3? tab++: 0;
        else tab > 0? tab--: 0;
        history[key] = (history[key] << 1 | jump) & 3;
    }

    double success_rate() {
        if(total) return 1.0 * success / total;
        else return 1.0;
    }

};

class simulator {
public:
    const static int REG_NUM = 32;
    const static int MEM_SIZE = 5e5;

private:
    bool halt_flag;
    bool flush_flag;
    addr_t jump_to;
    long long cycle;
    int inst_num;

    Register<word> pc;
    Regfile<REG_NUM> regfile;

    Decoder decoder;
    Bus<CDB_msg> cdb;

    RAM<MEM_SIZE> ram; 
    Delay<CDB_msg, 3> load_delay;
    Delay<Store_msg, 3> store_delay; 
    
    Speculation spec;

    SeqQueue<InstQue_node, 16> inst_que;

    ALU alu;
    Adder addr_adder;
    CDB_reg alu_out;
    CDB_reg store_out;
    // CDB_reg addrout;
    CDB_reg load_out;
    Stall stall;

    SeqQueue<CDB_reg*, 5> send_que;

    RS rs;
    SLB slb;
    ROB rob;
    Counter store_cnt;

    void fetch() {
        if(inst_que.full() || stall.get()) return ;
        addr_t cur_pc = pc.read();
        inst_t inst = ram.read_word(cur_pc);
        // halt instruction
        if(inst == 0x0ff00513) stall.set(1);
        
        Decoder pre_decoder;
        Adder pc_adder;
        pre_decoder.decode(inst);
        if(pre_decoder.opt == JALR) stall.set(1);

// std::cout << ">> fetch inst: " << std::hex << std::setw(8) << std::setfill('0') << word(inst) << " ";
// std::cout << std::setw(5) << std::setfill(' ') << opt_to_string(pre_decoder.opt) << " ";
// std::cout << std::hex << std::setw(8) << std::setfill('0') << word(pre_decoder.imm) << "\n";

        // predict next pc
        bool flag = 0;
        if(pre_decoder.type == 'B') flag = spec.predict(cur_pc);
        if(pre_decoder.type == 'J') flag = 1;
        addr_t nex_pc = pc_adder.calc(cur_pc, flag? pre_decoder.imm: 4);
        // pc when mispredicted
        flag = 0;
        if(pre_decoder.type == 'B') flag = !spec.predict(cur_pc);
        if(pre_decoder.type == 'J') flag = 1;
        addr_t mis_pc = pc_adder.calc(cur_pc, flag? pre_decoder.imm: 4);

// std::cout << "cur_pc: " << std::hex << std::setw(6) << std::setfill('0') << word(cur_pc) << std::endl;
// std::cout << "nex_pc: " << std::hex << std::setw(6) << std::setfill('0') << word(nex_pc) << std::endl;
// std::cout << "mis_pc: " << std::hex << std::setw(6) << std::setfill('0') << word(mis_pc) << std::endl;
        pc.write(nex_pc);
        inst_que.push((InstQue_node) {
            inst, cur_pc, nex_pc, mis_pc, spec.predict(cur_pc)
        });
    }

    void getRegSrc(byte rs, byte &src, word &val) {
        auto ord = regfile.order(rs);
        if(!ord) src = 0, val = regfile.read(rs);
        else if(rob.ready(ord)) src = 0, val = rob.value(ord);
        else src = ord, val = 0;
    }

    Buffer_item getBuffer(const InstQue_node &pc_info, byte ROBidx) {
        Buffer_item ret;
        ret.ROBidx = ROBidx;
        ret.opt = decoder.opt;
        ret.src1 = ret.src2 = 0;
        ret.val1 = ret.val2 = 0;
        ret.imm = 0;
        switch(decoder.type) {
            case 'R': case 'B':
                getRegSrc(decoder.rs1, ret.src1, ret.val1);
                getRegSrc(decoder.rs2, ret.src2, ret.val2);
                break;
            case 'U':
                ret.src1 = ret.src2 = 0;
                ret.val1 = decoder.opt == LUI? 0: pc_info.pc;
                ret.val2 = 0;
                ret.imm = decoder.imm;
                break;
            case 'J':
                ret.src1 = ret.src2 = 0;
                ret.val1 = pc_info.pc, ret.val2 = 4;
                break;
            case 'I':
                getRegSrc(decoder.rs1, ret.src1, ret.val1);
                ret.src2 = ret.val2 = 0;
                ret.imm = decoder.imm;
                break;
            case 'S':
                getRegSrc(decoder.rs1, ret.src1, ret.val1);
                getRegSrc(decoder.rs2, ret.src2, ret.val2);
                ret.imm = decoder.imm;
                break;
        }
        return ret;
    }

    ROB_item getROB(const InstQue_node &pc_info, byte idx) {
        ROB_item ret;
        ret.idx = idx;
        ret.org = decoder.org;
        ret.opt = decoder.opt;
        ret.cur_pc = pc_info.pc;
        ret.nex_pc = pc_info.nex_pc;
        ret.mis_pc = pc_info.mis_pc;
        ret.jump = pc_info.jump;
        ret.dest = 0;
        ret.data = 0;
        ret.addr = 0;
        switch(decoder.type) {
            case 'B': case 'S': break;
            case 'R': case 'J': case 'U': case 'I':
                ret.dest = decoder.rd;
                regfile.rename(decoder.rd, idx);
                break;
        }
        ret.cnt = 1;
        // if(decoder.opt > LOAD_BEG && decoder.opt < LOAD_END) ret.cnt = 2;
        // else ret.cnt = 1;
        return ret;
    }

    void issue() {
        if(inst_que.empty() || rob.full()) return ;

        auto cur_inst = inst_que.front();
// std::cout << ">> issue inst: ";
// std::cout << std::hex << std::setw(8) << std::setfill('0') << word(cur_inst.inst) << " ";
// std::cout << std::hex << std::setw(8) << std::setfill('0') << word(cur_inst.pc) << " ";
// std::cout << std::hex << std::setw(8) << std::setfill('0') << word(cur_inst.nex_pc) << " ";
// std::cout << std::hex << std::setw(8) << std::setfill('0') << word(cur_inst.mis_pc) << "\n";

        decoder.decode(cur_inst.inst);

        bool sltag = 0;
        sltag |= decoder.opt > LOAD_BEG && decoder.opt < LOAD_END;
        sltag |= decoder.opt > STORE_BEG && decoder.opt < STORE_END;
        if(sltag && slb.full()) return ;
        if(!sltag && rs.full()) return ;
        
        inst_que.pop();
        if(decoder.opt == NONE) return ;

        byte ROBidx = rob.allocate() + 1;
        auto item = getBuffer(cur_inst, ROBidx);
        auto ROBitem = getROB(cur_inst, ROBidx);
        
        if(cdb.traffic()) {
            auto msg = cdb.recv();
            item.update(std::get<0>(msg), std::get<1>(msg));            
        }

        rob.issue(ROBidx, ROBitem);
        if(sltag) slb.issue(item);
        else rs.issue(item);
    }

    void execute() {
        if(!rs.empty()) {
            auto *item = rs.execute(alu_out);
            if(item) {
                bool flag = 0;
                flag |= item->opt == LUI || item->opt == AUIPC;
                flag |= item->opt > IMM_BEG && item->opt < IMM_END;
                flag |= item->opt == JALR;
                word opd1 = item->val1;
                word opd2 = flag? item->imm: item->val2;            
                switch(item->opt) {
                    case SLL: case SRL: case SRA:
                    case SLLI: case SRLI: case SRAI:
                    opd2 = Decoder::slice(opd2, 0, 5);
                }
                word res = alu.calc(item->opt, opd1, opd2);
                alu_out.write(CDB_msg(item->ROBidx, res, 0));
                alu_out.pend(1);
                send_que.push(&alu_out);
            }
        }

        if(!slb.empty()) {
            auto *item = slb.execute(store_out, load_out, store_cnt.count());
            // auto *item = slb.execute(addrout, load_out);
            if(item) {
                addr_t addr = addr_adder.calc(item->val1, item->imm);
                // addrout.write(CDB_msg(item->ROBidx, item->val2, addr));
                // addrout.pend(1);
                // send_que.push(&addrout);
                if(item->opt > LOAD_BEG && item->opt < LOAD_END) {
                    word res;
                    switch(item->opt) {
                        case LB: res = 0; break;
                        case LH: res = 1; break;
                        case LW: res = 2; break;
                        case LBU: res = 3; break;
                        case LHU: res = 4; break;
                    }
                    // switch(item->opt) {
                    //     case LB: res = Decoder::sext(ram.read_byte(addr), 8); break;
                    //     case LH: res = Decoder::sext(ram.read_hfword(addr), 16); break;
                    //     case LW: res = ram.read_word(addr); break;
                    //     case LBU: res = ram.read_byte(addr); break;
                    //     case LHU: res = ram.read_hfword(addr); break;
                    // }
                    load_delay.input(Load_msg(item->opt, item->ROBidx, addr));
                    load_out.pend(1);
                }
                else {
                    store_cnt.inc();
                    store_out.write(CDB_msg(item->ROBidx, item->val2, addr));
                    store_out.pend(1);
                    send_que.push(&store_out);
                }
            }
        }
    }

    void write_result() {
        if(cdb.traffic()) {
            auto msg = cdb.recv();
            rob.update(std::get<0>(msg), std::get<1>(msg), std::get<2>(msg));
            rs .update(std::get<0>(msg), std::get<1>(msg));
            slb.update(std::get<0>(msg), std::get<1>(msg));
        }
        else {
            if(!send_que.empty()) {
                CDB_reg *out = send_que.front();
                cdb.send(out->read()), out->pend(0);
                send_que.pop();
            }
        }
        if(store_delay.signaled()) {
            auto out = store_delay.output();
            auto opt = std::get<0>(out);
            auto data = std::get<1>(out);
            auto addr = std::get<2>(out);
            switch(opt) {
                case SB: ram.write_byte(addr, data); break;
                case SH: ram.write_hfword(addr, data); break;
                case SW: ram.write_word(addr, data); break;
            }
        }
        if(load_delay.signaled()) {
            auto out = load_delay.output();
            auto opt = std::get<0>(out);
            auto idx = std::get<1>(out);
            auto addr = std::get<2>(out);
            word data = 0;
            switch(opt) {
                case LB: data = Decoder::sext(ram.read_byte(addr), 8); break;
                case LH: data = Decoder::sext(ram.read_hfword(addr), 16); break;
                case LW: data = ram.read_word(addr); break;
                case LBU: data = ram.read_byte(addr); break;
                case LHU: data = ram.read_hfword(addr); break;
            }
            load_out.write(CDB_msg(idx, data, addr));
            send_que.push(&load_out);
        }
    }

    int commit() {
        if(rob.empty()) return 0;
        auto *item = rob.commit();
        if(!item) return 0;
        inst_t org_inst = item->org;

        // Branch
        if(item->opt > BRANCH_BEG && item->opt < BRANCH_END) {
            bool mis_flag = 0;
            bool act_flag = 0;
            switch(item->opt) {
                case BEQ: act_flag = item->data == 0; break;
                case BNE: act_flag = item->data != 0; break;
                case BLT: case BLTU: act_flag = item->data == 1; break;
                case BGE: case BGEU: act_flag = item->data == 0; break;
            }
            mis_flag = act_flag != item->jump;
            spec.feedback(item->cur_pc, act_flag, mis_flag);
            if(mis_flag) {
                flush_flag = 1;
                jump_to = item->mis_pc;
            }
            return org_inst;
        }
        // Store
        if(item->opt > STORE_BEG && item->opt < STORE_END) {
            store_cnt.dec();
            store_delay.input(Store_msg(item->opt, item->data, item->addr));
            return org_inst;
        }
        // Jump
        word write_data;
        if(item->opt == JALR) {
            pc.write(item->data & ~1u);
            write_data = item->nex_pc;
            stall.set(0);
        }
        else write_data = item->data;
        // Ohters
        auto rd = item->dest;
        regfile.write(rd, write_data);
        regfile.reset(rd, item->idx);
        return org_inst;
    }

    void tick() {
        cycle++;
        if(flush_flag) {
            pc.write(jump_to);
            store_cnt.set(0);
            rs.flush(), slb.flush(), rob.flush();
            regfile.flush(), inst_que.flush(), send_que.flush();
            cdb.flush(), alu_out.flush(), store_out.flush(), load_out.flush();
            // addrout.flush(),
            load_delay.flush();
            stall.set(0);
            flush_flag = 0;
        }
        store_cnt.tick();
        stall.tick();
        pc.tick();
        regfile.tick();
        inst_que.tick();
        send_que.tick();
        rs.tick();
        slb.tick();
        rob.tick();
        cdb.tick();
        alu_out.tick();
        store_out.tick();
        // addrout.tick();
        load_out.tick();
        load_delay.tick();
        store_delay.tick();
    }

    void print() {
        std::cout << "+----------------------------- LOG ---------------------------+\n";
        std::cout << "[pc] " << std::hex << std::setw(8) << std::setfill('0') << pc.read() << '\n';
        std::cout << "[regfile]\n";
        regfile.print();
        std::cout << "[cdb] ";
        if(cdb.traffic()) {
            auto msg = cdb.recv();
            std::cout << std::dec << std::setw(2) << std::setfill('0') << word(std::get<0>(msg)) << " ";
            std::cout << std::hex << std::setw(8) << std::setfill('0') << word(std::get<1>(msg)) << " ";
            std::cout << std::hex << std::setw(8) << std::setfill('0') << word(std::get<2>(msg)) << "\n";
        }
        else std::cout << "no traffic\n";
        std::cout << "[out] ";
        if(alu_out.pending()) std::cout << "alu ";
        if(store_out.pending()) std::cout << "store ";
        // if(addrout.pending()) std::cout << "addr ";
        if(load_out.pending()) std::cout << "load ";
        std::cout << "\n";
        std::cout << "[reservation station]\n";
        rs.print(); 
        std::cout << "[store load buffer] ";
        std::cout << std::dec << store_cnt.count() << "\n";
        slb.print();
        std::cout << "[reorder buffer]\n";
        rob.print();
        std::cout << "[store delay] ";
        if(store_delay.signaled()) {
            auto msg = store_delay.output();
            std::cout << std::setw(5) << std::setfill(' ') << opt_to_string(std::get<0>(msg)) << " ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << word(std::get<1>(msg)) << " ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << word(std::get<2>(msg)) << "\n"; 
        }
        else std::cout << "no signal\n";
        std::cout << "[ load delay] ";
        if(load_delay.signaled()) {
            auto msg = load_delay.output();
            std::cout << "#" << std::setw(4) << std::setfill('0') << std::dec << word(std::get<0>(msg)) << " ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << word(std::get<1>(msg)) << " ";
            std::cout << std::setw(8) << std::setfill('0') << std::hex << word(std::get<2>(msg)) << "\n"; 
        }
        else std::cout << "no signal\n";
        std::cout << std::endl;
    }

    void init() {
        flush_flag = 0;
        cycle = 0, inst_num = 0;
        pc.init(0);
        stall.init(0);
        store_cnt.init(0);
    }

public:

    void scan() {
        std::string buff;
        addr_t addr;
        word data;
        while(std::cin >> buff) {
            if(buff[0] == '@') {
                std::stringstream ss(buff.substr(1));
                ss >> std::hex >> addr;
            }
            else {
                std::stringstream ss(buff);
                ss >> std::hex >> data;
                ram.write_byte(addr, data);
                addr++;
            }
        }
    }

    void run() {
int tot = 0;
int cnt = 10000;
        inst_t code;
        while(1) {
            code = commit();
            write_result();
            execute();
            issue();
            fetch();
            if(code) inst_num++;
            if(code == 0x0ff00513) break;
            tick();
//             if(cnt > 0) {
//                 if(code) tot++;
// // std::cerr << std::dec << tot << " " << std::hex << std::setw(8) << std::setfill('0') << tmp << std::endl;
//                 if(code) {
//                     std::cout << "[commit code] ";
//                     std::cout << std::hex << std::setw(8) << std::setfill('0') << word(code) << " ";
//                     std::cout << std::dec << "#" << tot << std::endl;
//                 }
//                 cnt--;
//                 if(code) {
//                     // std::cout << "[pc] " << std::hex << std::setw(8) << std::setfill('0') << word(pc.read()) << std::endl;
//                     regfile.print();
//                     // std::cout << std::hex << std::setw(8) << std::setfill('0') << ram.read_word(0x12e4) << std::endl;
//                     // print();
//                 }
//             }
        }
        std::cerr << std::dec << std::setprecision(4) << spec.success_rate() << std::endl;
        std::cout << std::dec << (regfile.read(10) & 255u) << std::endl;
    }

};

}

#endif