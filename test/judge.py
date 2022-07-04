import os
import subprocess

data_path = '../testcases_for_riscv/testcases/'
testcases = [
    ('array_test1', 123),
    ('array_test2', 43),
    ('basicopt1', 88),
    ('bulgarian', 159),
    ('expr', 58),
    ('gcd', 178),
    ('hanoi', 20),
    ('lvalue2', 175),
    ('magic', 106),
    ('manyarguments', 40),
    ('multiarray', 115),
    ('naive', 94),
    ('qsort', 105),
    ('queens', 171),
    ('statement_test', 50),
    ('superloop', 134),
    ('tak', 186)
]

def printCtrl(msg, line=True):
    print(f'\033[1;36m{msg}\033[0;0m', end = '\n' if line else ' ')
def printInfo(msg, line=True):
    print(f'\033[1;32m{msg}\033[0;0m', end = '\n' if line else ' ')
def printWarn(msg, line=True):
    print(f'\033[0;31m{msg}\033[0;0m', end = '\n' if line else ' ')

def compile():
    subprocess.getoutput('g++ ../src/main.cpp -o t')

def execute(input):
    if not os.path.exists(input):
        printWarn('input not found')
        quit()
    p = subprocess.Popen(
        f'./t <{input}', shell=True,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    return p.communicate()


def main():
    compile()
    printCtrl('compile done')
    
    for i, case in enumerate(testcases):
        name = case[0]
        ans = case[1]
        printCtrl('run test case', False)
        printInfo(name)
        out, err = execute(data_path + name + '.data')
        # print(out, err)
        out = out.decode().splitlines()[0]
        err = err.decode().splitlines()
        [ins, cyc, acc, tot] = err
        if int(out) != ans:
            printWarn('Wrong Answer')
            quit()
        print(f'judge result: {out}')
        print(f'commit number: {ins}')
        print(f'cycles: {cyc}')
        print(f'accuracy: {acc} among {tot}')
    
    printCtrl('execute done')

if __name__ == '__main__':
    main()
    
