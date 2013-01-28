#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "simmacros.h"

int main(int argc, char *argv[]){
    char *ifile, mystring[256];
    unsigned int instruction, opcode;
    int address, memsize;
    FILE *iFile;

    if(argc!=3){
        printf("Usage: sim membytes input.o\n");
        return -1;
    }
    sscanf(argv[1], "%i",&memsize);
    initMemory(memsize);

    /*load program into memory*/
    ifile = calloc(strlen(argv[2])+1,sizeof(char));
    strcpy(ifile,argv[2]);
    iFile = fopen(ifile, "r");
    free(ifile);

    if(NULL!=iFile){
        address=0;
        while(fgets(mystring,255,iFile)){
            sscanf (mystring,"0x%x", &instruction);
            memcpy (cMemory + address, &instruction, sizeof(instruction));
            address+=4;
        }
        fclose(iFile);
    }else{
        printf("Input file invalid");
        return (-1);
    }

    /*fetch, decode, execute*/
    PC=0;
    Cflg=FALSE;
    Oflg=FALSE;

    while((PC<<2)<(unsigned int)(memsize)){
    memcpy(&IR,&(iMemory[PC]),sizeof(IR));
    PC++;
    opcode=IR>>28;
    (ops[opcode])();
    }

    if((PC<<2)<(unsigned int)(memsize)) printf("Too little memory allocated");
    exit(0);
}


