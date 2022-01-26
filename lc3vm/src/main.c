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


// EXPLAINation of FUNCTIONS PROVIDED IN .H file

#include "vm.h"

uint16_t memory[UINT16_MAX]; //MEMORY STORGE Our VM can store max upto 65536 hence 16-bit MAX

   uint16_t reg[R_COUNT]; // number of registers

  struct termios original_tio;
// main function  starts now

int main(int argc , char* argv[])
{
   

 // check for arguments the binary is gonna recieve

	if(argc < 2){
		// usage"
		printf("%s\n","./vm [path_to_image].....");
		exit(2);
	}


	for(int i = 1; i < argc; i++){
		if(!read_image(argv[i])){
			printf("%s\n", "failed to Load image Try again :("); // checking if the image has loaded properly
		}
	}


signal(SIGINT,handle_interrupt);
disable_input_buffering();

reg[R_COND] = FL_ZRO; // since only one condition flag can be set at a time lets set the zero flag by default

enum{PC_START = 0x3000}; 
/* start of range of address doesnt start at 0 but at 0x3000 because that memory region 
is reserved for system memory traps
*/

//setting the instruction pointer to the starting address

reg[R_PC] = PC_START;

int running = 1;

while(running){
// FETCH THE INSTRUCTIONS
	uint16_t instr = mem_read(reg[R_PC]++); //read the insrtuction from the binary and update the instruction pointer
	uint16_t op = instr >> 12;
	switch(op){
		case OP_ADD: // add instrcution has 2 cases one with register and immediate value embedded in instruction
		// add opcode or instusction
			{
				    //destination register
					uint16_t r0 = (instr >> 9 ) & 0x7;
					//source register
					uint16_t r1 = (instr >> 6 ) & 0x7;
					// to check whether we are in immediate mode or register
					uint16_t imm_flag = (instr >> 5) & 0x1; // in immediate mode the value is embedded in final 5 bits on the instrction

					if(imm_flag){
						uint16_t imm5 = sign_extend(instr & 0x1F , 5);
						reg[r0] = reg[r1] + imm5; // adding register and immediate
					}

					// else normal register mode

					else{
						uint16_t r2 = instr & 0x7;
						reg[r0] = reg[r1] + reg[r2]; // addding two registers
					}

					update_flags(r0); //updating flags based on operation

			} //end of add

			break;

			case OP_AND:
				{

					//similar to that of add instruction but WITH bitwise and
					uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                
                    if (imm_flag)
                    {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[r0] = reg[r1] & imm5;
                    }
                    else
                    {
                        uint16_t r2 = instr & 0x7;
                        reg[r0] = reg[r1] & reg[r2];
                    }
                    update_flags(r0);

				}// end of and

				break;

			case OP_NOT:
				{
					//NOT instrcution
					uint16_t r0 = (instr >> 9) & 0x7;
					uint16_t r1 = (instr >> 6) & 0x7;
					reg[r0] = ~reg[r1];
					update_flags(r0);
				}//end of NOT


				break;


			case OP_BR:
				{
					// brancing instruction
					uint16_t pc_offset = sign_extend(instr & 0x1FF , 9); // since the address is in last 9 bits
					uint16_t cond_flag = (instr >> 9) & 0x7;
					if(cond_flag & reg[R_COND]) // checking for condition before branching
					{
						reg[R_PC] += pc_offset;
					}
				}// end of branch


				break;
			case OP_JMP:
				{
					// uncondiational jump instruction
					uint16_t r1 = (instr >> 6) & 0x7;
					reg[R_PC] = r1;
				}// end of jump

				break;


			case OP_JSR:
				{
					uint16_t long_flag = (instr >> 1) & 1;
					reg[R_R7] = reg[R_PC];
                    if (long_flag)
                    {
                        uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
                        reg[R_PC] += long_pc_offset;  /* JSR */
                    }
                    else
                    {
                        uint16_t r1 = (instr >> 6) & 0x7;
                        reg[R_PC] = reg[r1]; /* JSRR */
                    }
                    

				}// end of jsr

				break;


			case OP_LD:
				{
					//LOAD INSTRUCTION
					uint16_t r0 = (instr >> 9) & 0x7; 
					uint16_t pc_offset = sign_extend(instr & 0x1FF , 9);// getting the pc offset from the leftmost 9 bits
					reg[r0] = mem_read(reg[R_PC] + pc_offset); // reading from the memoru region given by the offset
					update_flags(r0); // updating flags although i dont think we need to to update on read meh just a convention

				} // end of load

				break;

			case OP_LDI:
				{
					//destination register
					uint16_t r0 = (instr >> 9) & 0x7;
					uint16_t pc_offset = sign_extend(instr & 0x1FF , 9);
					reg[r0] = mem_read(reg[R_PC] + pc_offset);
					update_flags(r0);
				}// end of load immediate

				break;

			case OP_LDR:
				{	// loading from another register
					uint16_t r0 =  (instr >> 9) & 0x7;
					uint16_t r1 = (instr >> 6) & 0x7;
					uint16_t offset = sign_extend(instr & 0x3F,6);
					reg[r0] = mem_read(reg[r1] + offset);
					update_flags(r0);
				}// end of load register

				break;


			case OP_LEA:
				{

					// load effective register
					uint16_t r0 = (instr >> 9) & 0x7;
					uint16_t pc_offset = sign_extend(instr & 0x1FF , 9);
					reg[r0] = reg[R_PC] + pc_offset;
					update_flags(r0);
				}// end of load effective address

				break;

			case OP_ST:
				{
					//store instruction

					uint16_t r0 = (instr >> 9) & 0x7;
					uint16_t pc_offset = sign_extend(instr & 0x1FF , 9);
					mem_write(reg[R_PC]+pc_offset , reg[r0]); // storing the value in that address into register
			        update_flags(r0);


				}// end of store

				break;

			case OP_STI:
				{
					uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
				}// end of store


				break;

			case OP_STR:
			{
				//store register instrucitom
				uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F, 6);
                mem_write(reg[r1] + offset, reg[r0]);
			}// end of Store register


			break;


			case OP_TRAP:
				//OS system traps
				switch(instr & 0xFF)
				{
					case TRAP_GETC:
						//reading a single char from keyboard with traps
						reg[R_R0] = (uint16_t)getchar();
						update_flags(R_R0);
						break;

					case TRAP_OUT:
						// priting a single char to std output
						putc((char)reg[R_R0],stdout);
						fflush(stdout);
						break;

					case TRAP_PUTS:
						{
							// priting a whole string char by char
						uint16_t* c = memory + reg[R_R0];
						while (*c){
							putc((char)*c,stdout);
							++c;
						}
						fflush(stdout);
						}

						break;

					case TRAP_IN:
						{

							printf("Enter a character: ");
                            				char c = getchar();
                            				putc(c, stdout);
                            				fflush(stdout);
                            				reg[R_R0] = (uint16_t)c;
                            				update_flags(R_R0);

						}
						
						break;

					case TRAP_PUTSP:
						{
							 /* one char per byte (two bytes per word)
                               here we need to swap back to
                               big endian format */
                            uint16_t* c = memory + reg[R_R0];
                            while (*c)
                            {
                                char char1 = (*c) & 0xFF;
                                putc(char1, stdout);
                                char char2 = (*c) >> 8;
                                if (char2) putc(char2, stdout);
                                ++c;
                            }
                            fflush(stdout);
						}

						break;


					case TRAP_HALT:
						// halting trap terminating the process
						puts("program Terminated *.*\n");
						fflush(stdout);
						running = 0;
						break;



				}// end of inner switch

				break;



			case OP_RES:
			case OP_RTI:
			default:
				//UNRECOGNIZED OR INVALID OPCODE
				abort();

				break;




					}// end of outer switch
						}// end of while






	restore_input_buffering();

	}//end of main
	




