#include "spimcore.h"
#define MEMSIZE (64 * 1024)
#define HALT_ADDRESS (MEMSIZE / 4)

/* ALU */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult,
         char *Zero) {
  switch (ALUControl) {
  case 0: // Z = A + B
    *ALUresult = A + B;
    break;
  case 1: // Z = A - B
    *ALUresult = A - B;
    break;
  case 2: // Z = if A < B then 1 else 0
    *ALUresult = (A < B) ? 1 : 0;
    break;
  case 3: // Z = if A < B (unsigned) then 1 else 0
    *ALUresult = ((unsigned)A < (unsigned)B) ? 1 : 0;
    break;
  case 4: // Z = A AND B
    *ALUresult = A & B;
    break;
  case 5: // Z = A OR B
    *ALUresult = A | B;
    break;
  case 6: // Z = B shifted left by 16 bits
    *ALUresult = B << 16;
    break;
  case 7: // Z = NOT A
    *ALUresult = ~A;
    break;
  default:
    // Handle unsupported ALU control codes
    break;
  }

  *Zero = (*ALUresult == 0) ? 1 : 0;
}

/* instruction fetch */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction) {
  *instruction = Mem[PC >> 2];

  if (*instruction == 0x00000000) {
    return 1; // Check for halt condition
  }
  return 0; // Successful fetch
}

/* instruction partition */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,
                           unsigned *r2, unsigned *r3, unsigned *funct,
                           unsigned *offset, unsigned *jsec) {
  *op = (instruction >> 26);        // bits 31-26
  *r1 = (instruction >> 21) & 0x1F; // bits 25-21
  *r2 = (instruction >> 16) & 0x1F; // bits 20-16
  *r3 = (instruction >> 11) & 0x1F; // bits 15-11
  *funct = instruction & 0x3F;      // bits 5-0
  *offset = instruction & 0xFFFF;   // bits 15-0
  *jsec = instruction & 0x3FFFFFF;  // bits 25-0

  // slt and sltu instruction
  if (*op == 0x0) {

    // slt
    if (*funct == 0x2A) {
      *op = 0x2A;
    }

    // sltu
    else if (*funct == 0x2B) {
      // changing op code because sw also uses op code 0x2B
      *op = 0x29;
    }
  }
}

/* instruction decode */
int instruction_decode(unsigned op, struct_controls *controls) {

  switch (op) {
  case 0x8: // addi
    controls->RegDst = 0;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->ALUOp = 0;
    controls->MemWrite = 0;
    controls->ALUSrc = 1;
    controls->RegWrite = 1;
    break;
  case 0x0: // add
    controls->RegDst = 1;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->MemWrite = 0;
    controls->RegWrite = 1;
    controls->ALUSrc = 0;
    controls->ALUOp = 0;
    break;
  case 0x2a: // slt
    controls->RegDst = 1;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->MemWrite = 0;
    controls->RegWrite = 1;
    controls->ALUSrc = 0;
    controls->ALUOp = 2;
    break;
  case 0x2b: // sw
    controls->RegDst = 0;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->MemWrite = 1;
    controls->RegWrite = 0;
    controls->ALUSrc = 1;
    controls->ALUOp = 0;
    break;
  case 0x29: // sltu
    controls->RegDst = 1;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->MemWrite = 0;
    controls->RegWrite = 1;
    controls->ALUSrc = 0;
    controls->ALUOp = 3;
    break;
  case 0x23: // lw
    controls->RegDst = 0;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 1;
    controls->MemtoReg = 1;
    controls->ALUOp = 0;
    controls->MemWrite = 0;
    controls->ALUSrc = 1;
    controls->RegWrite = 1;
    break;
  case 0x02: // j
    controls->RegDst = 0;
    controls->Jump = 1;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->ALUOp = 0;
    controls->MemWrite = 0;
    controls->ALUSrc = 0;
    controls->RegWrite = 0;
    break;
  case 0x04: // beq
    controls->RegDst = 0;
    controls->Jump = 0;
    controls->Branch = 1;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->ALUOp = 1;
    controls->MemWrite = 0;
    controls->ALUSrc = 0;
    controls->RegWrite = 0;
    break;
  case 0xf: // lui
    controls->RegDst = 0;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->ALUOp = 6;
    controls->MemWrite = 0;
    controls->ALUSrc = 1;
    controls->RegWrite = 1;
    break;
  default:
    // handle unsupported instruction opcodes
    return 1; // Set halt condition
  }
  return 0; // No halt condition
}

/* Read Register */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1,
                   unsigned *data2) {
  // get data from registers
  *data1 = Reg[r1];
  *data2 = Reg[r2];
}

/* Sign Extend */
void sign_extend(unsigned offset, unsigned *extended_value) {

  // offset is negative so add 1s
  if (offset & 0x00008000) {
    *extended_value = offset | 0xFFFF0000;
  }

  // offset is positive so add 0s
  else {
    *extended_value = offset | 0x00000000;
  }
}

/* ALU operations */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value,
                   unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult,
                   char *Zero) {

  // invalid alu operation
  if (ALUOp > 7 || ALUOp < 0) {
    return 1; // Halt condition
  }

  if (ALUSrc == 1) {
    // Use data1 and extended_value
    ALU(data1, extended_value, ALUOp, ALUresult, Zero);
  } else {
    // Use data1 and data2
    ALU(data1, data2, ALUOp, ALUresult, Zero);
  }

  return 0; // No halt condition
}

/* Read / Write Memory */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead,
              unsigned *memdata, unsigned *Mem) {

  // check if address matches the halt address or is out of bounds
  if (ALUresult >> 2 == HALT_ADDRESS) {
    return 1; // Set halt condition
  }

  // read data from memory
  if (MemRead) {
    *memdata = Mem[ALUresult >> 2];
  }

  // write data to memory
  if (MemWrite) {
    Mem[ALUresult >> 2] = data2;
  }

  return 0; // No halt condition
}

/* Write Register */
void write_register(unsigned r2, unsigned r3, unsigned memdata,
                    unsigned ALUresult, char RegWrite, char RegDst,
                    char MemtoReg, unsigned *Reg) {
  // only do if regwrite is true
  if (RegWrite) {
    // if memtoreg is true, write memdata to r2
    if (MemtoReg) {
      Reg[r2] = memdata;
      // if regdst is true, write ALUresult to r3, if it is false, write memdata
      // to r3
    } else {
      Reg[(RegDst) ? r3 : r2] = ALUresult;
    }
  }
}

/* PC update */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump,
               char Zero, unsigned *PC) {

  // update PC based on branch and jump conditions
  if (Branch && Zero) {
    *PC += (extended_value * 4) + 4;
  } else if (Jump) {
    *PC = (jsec << 2); // Set PC to jump target address directly
  } else {
    // Increment PC by 4
    *PC += 4;
  }
}
