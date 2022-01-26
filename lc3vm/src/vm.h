//PREPROCESSING HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

//unix standard libraries
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/termios.h>
#include <sys/mman.h>

// REGISTERS

 enum {

R_R0 = 0x0,
R_R1,
R_R2,
R_R3,
R_R4,
R_R5,
R_R6,
R_R7,
R_PC, // instrcution pointer
R_COND,
R_COUNT
};


// OP CODES FOr ADD,SUB and other instrcutions


 enum{

    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */

};


//condition flags
// used to determine the result of previous instrcution

enum{

FL_POS = 1<<0,// for setting the flag register as positive
FL_ZRO = 1<<1, // for setti,ng the flag register as zero or not
FL_NEG = 1<<2 // for setting the flag register negative

};



// SPECial memory mapped registers for I/O with hardware
//  NOTE JUSt a convention tbh hardware registers need not be memory mapped

 enum{

MR_KBSR = 0xFE00, // Keyboard status register used for indicate whether key has been pressed
MR_KBDR = 0xFE02 // Keyboard data register used for indicate what key has ben pressed

};


// SYSTEM TRAPS


 enum
{
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};



// MEMORY STORGE Our VM can store max upto 65536 hence 16-bit MAX


  extern uint16_t memory[UINT16_MAX];

// number of registers

 extern uint16_t reg[R_COUNT];

// sign extension to align it with 16 bits standard register size

extern uint16_t sign_extend(uint16_t x , int bit_count);

extern uint16_t swap16(uint16_t x); // since LC3 is big endian we need to swap from little Endian to BIG endian

extern void update_flags(uint16_t r); // updating flags register

extern void read_image_file(FILE* file); // reading image files or binaries to be executed by VM


extern int read_image(char* image_path);


//checking keys
extern uint16_t check_key();

// MEMORY ACCESS FUNCTIONS

extern void mem_write(uint16_t address , uint16_t val); // for writing a specific value in the particualr address


extern uint16_t mem_read(uint16_t address);


//  OOF these sections are apparetnly necessary for accessing the keyboard in unix like systems

// so i have no idea about termious tbh maybe.. one dayy

extern struct termios original_tio;

extern void disable_input_buffering();

extern void restore_input_buffering();

extern void handle_interrupt(int signal);
































