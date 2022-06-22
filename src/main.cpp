#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "utility.h"
#include "instruction.h"

riscv::Instruction ins;

int main() {
// freopen("./data/sample.data", "r", stdin);
    std::vector<riscv::inst_t> array;
    std::string buff;
    riscv::off_t offset;
    while(!std::cin.eof()) {
        getline(std::cin, buff);
        if(buff.empty()) break;
//std::cerr << ">> buff " << buff << std::endl;
        if(buff[0] == '@') {
            ins.init(offset, array);
            array.clear();
            std::stringstream ss(buff.substr(1));
            ss >> std::hex >> offset;
        }
        else {
            riscv::append_inst(buff, array);
        }
    }
    ins.init(offset, array);
    ins.execute(0);
    return 0;
}