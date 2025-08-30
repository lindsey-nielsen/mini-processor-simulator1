# Mini Processor Simulator
This project implements a **mini MIPS processor simulator** in C. The simulator models a single-cycle datapath and demonstrates the execution of basic MIPS instructions step by step.

## Features
- Supports 14 MIPS instructions
- Handles 32 general-purpose registers
- 64 KB memory space (0x0000 - 0xFFFF), word aligned
- Big-endian memory storage
- Halts on invalid instructions, misaligned accesses, or out-of-bounds memory

## Implementation
- project.c -> my implementation of the simulator logic
- spimcore.c / spimcore.h -> provided framework files
- input_file.asc / input_file2.asc -> provided input files
- test_results.txt -> provided output file

## Usage
### Compile:
gcc -o spimcore spimcore.c project.c

### Run with input file:
./spimcore <filename>.asc

### Example .asc input:
20010000  # addi $1, $0, 0

200200c8  # addi $2, $0, 200

10220003  # beq $1, $2, 3

## Commands
- r: Show registers
- m: Show memory
- s: Step one instruction
- c: Continue execution
- h: Check halt state
- p: Print input file
- g: Display most recent control signals
- x: Quit
