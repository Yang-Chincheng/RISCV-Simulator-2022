#include "simulator.h"

int main() {
// freopen("../data/testcases/qsort.data", "r", stdin);
// freopen("../test/tmp.out", "w", stdout);
    riscv::simulator sim;
    sim.scan();
    sim.run();
    return 0;
}