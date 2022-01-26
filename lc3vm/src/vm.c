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



// sign extend function

uint16_t sign_extend(uint16_t x , int bit_count){

 if((x >> (bit_count - 1)) & 1){ /*

 	checking if number is positive or negative number by checking the left most bit 
 	its 1 for negative numbers hence bitwise and with 1
 	*/

 	x |= (0xFFFF << bit_count);
 }

 return  x;


}





uint16_t swap16(uint16_t x)
{
	return (x << 8) | (x >> 8); //little endian to BIG endian
}







void update_flags(uint16_t r)
{
	// updating the registeries on the basis of condition flags

	if(reg[r] == 0){
		reg[R_COND] = FL_ZRO;

	}

	else if(reg[r] >> 15){
		reg[R_COND] = FL_NEG;
	}

	else{
		reg[R_COND] = FL_POS;
	}



}// end of update_flags








void read_image_file(FILE* file){

// origin tells us thwere the image lies at memory
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin); //swapping to BIG endian

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = UINT16_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    while (read-- > 0)
    {
        *p = swap16(*p); // again swapping to write to valid location
        ++p;
    }

}


int read_image(char* image_path){
	FILE* file = fopen(image_path , "rb");
	if(file==0){
		return 0;
	}

	read_image_file(file);
	fclose(file);
	return 1;

}


uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}


void mem_write(uint16_t address,uint16_t val){
	memory[address] = val;
}


uint16_t mem_read(uint16_t address){
	if(address == MR_KBSR){ // check if memory is in the memory mapped region
		memory[MR_KBSR] = (1 << 15);
		memory[MR_KBDR] = getchar();
	}

	else{
		memory[MR_KBSR] = 0;
	}

	return memory[address];
}




void disable_input_buffering()
{  
	tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}


void restore_input_buffering()
{
	
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

/* Handle Interrupt */
void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}


