#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "../lib/utility.h"
#include "../lib/instruction.h"

riscv::Instruction ins;

int main() {
// freopen("../data/sample/sample.data", "r", stdin);
freopen("../data/testcases/qsort.data", "r", stdin);
freopen("../test/std.out", "w", stdout);
    std::vector<riscv::inst_t> array;
    std::string buff;
    riscv::addr_t offset;
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