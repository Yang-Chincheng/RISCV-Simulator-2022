# RISCV-Simulator-2022

A single pipeline scheduling RISCV simulator implemented in C++, mainly used as a checker for the dynamic scheduling version.

The simulator is based on RV32I ISA. Instructions supported are listed as follow:

| Instruction | Type | Description                               |
| ----------- | ---- | ----------------------------------------- |
| lui         | U    | Load Upper Immediate                      |
| auipc       | U    | Add Upper Immediate to PC                 |
| jal         | J    | Jump and  Link                            |
| jalr        | I    | Jump and Link Register                    |
| beq         | B    | Branch if Equal                           |
| bne         | B    | Branch if Less Than                       |
| bge         | B    | Branch if Greater Than or Equal           |
| bltu        | B    | Branch if Less Than, Unsigned             |
| bgeu        | B    | Branch if Greater Than or Equal, Unsigned |
| lb          | I    | Load Byte                                 |
| lh          | I    | Load Halfword                             |
| lw          | I    | Load Word                                 |
| lbu         | I    | Load Byte, Unsigned                       |
| lhu         | I    | Load Halfword, Unsigned                   |
| sb          | S    | Store Byte                                |
| sh          | S    | Store Halfword                            |
| sw          | S    | Store Word                                |
| addi        | I    | Add Immediate                             |
| slti        | S    | Set if Less Than Immediate                |
| sltiu       | S    | Set if Less Than Immediate, Unsigned      |
| xori        | I    | Exclusive-OR Immediate                    |
| ori         | I    | OR Immediate                              |
| andi        | I    | And Immediate                             |
| slli        | I    | Shift Left Logical Immediate              |
| srli        | I    | Shift Rights Logical Immediate            |
| srai        | I    | Shift Rights Arithmetic Immediate         |
| add         | R    | Add                                       |
| sub         | R    | Substract                                 |
| sll         | R    | Shift Left Logical                        |
| slt         | R    | Set if Less Than                          |
| sltu        | R    | Set if Less Than, Unsigned                |
| srl         | R    | Shift Right Logical                       |
| sra         | R    | Shift Right Arithmetic                    |
| xor         | R    | Exclusive-OR                              |
| or          | R    | OR                                        |
| and         | R    | And                                       |

All the instructions are 32-bit and little endian.

## About

PPCA 2022 assignment
