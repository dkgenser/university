#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifndef SIMMACROS_H
#define	SIMMACROS_H

/*memory*/
unsigned char *cMemory;
unsigned int *iMemory;

/*initializes memory array using function defined later*/
void initops();

void initMemory(int size){
    cMemory = (unsigned char *)malloc(size*sizeof(unsigned char));
    initops();
    iMemory=(unsigned int *)cMemory;
}


/*shifting codes for reading instructions*/
#define REG0 ((IR>>24) & 0x0F) 
#define REG1 ((IR>>20) & 0x00F)
#define REG2 ((IR>>16) & 0x000F)
#define IMMEDIATE (IR & 0x0000FFFF)
#define ADDRESS (IR & 0x00FFFFFF)

/*represent boolean values*/
#define TRUE 1
#define FALSE 0

/*flags*/
int Cflg;
int Oflg;

/*registers*/
unsigned short registers[8];
unsigned int PC;
unsigned int IR;

/*functions*/
void HLT(){
    free(cMemory);
    exit(0);
}

void JMP(){
    PC = (ADDRESS>>2);
}

void CJMP(){
    if(Cflg) PC = (ADDRESS>>2);
}

void OJMP(){
    if(Oflg) PC = (ADDRESS>>2);
}

void LOAD(){
    unsigned short input = cMemory[ADDRESS];
    registers[REG0]=input;
}

void STORE(){
    cMemory[ADDRESS]= registers[REG0];
}

void LOADI(){
    registers[REG0]=IMMEDIATE;
}

void NOP(){

}

void ADD(){
    unsigned short result = registers[REG1] + registers[REG2];
    if(result<registers[REG1]||result<registers[REG2]) Oflg=TRUE;
    registers[REG0]=result;
}

void SUB(){
    unsigned short result = registers[REG1] - registers[REG2];
    if(result>registers[REG1]) Oflg=TRUE;
    registers[REG0]=result;
}

void IN(){
    char in;
    read(0,&in,1);
    registers[REG0]=(unsigned short)in;
}

void OUT(){
    write(1,&(registers[REG0]),2);
}

void EQU(){
    if(registers[REG0]==registers[REG1]) Cflg=TRUE;
    else Cflg = FALSE;
}

void LT(){
    if(registers[REG0]<registers[REG1]) Cflg=TRUE;
    else Cflg=FALSE;
}

void LTE(){
    if(registers[REG0]<=registers[REG1]) Cflg=TRUE;
    else Cflg=FALSE;
}

void NOT(){
    if(Cflg) Cflg=FALSE;
    else Cflg=TRUE;
}

/*function array*/
void (*ops[16])();

void initops(){
    ops[0]=HLT;
    ops[1]=JMP;
    ops[2]=CJMP;
    ops[3]=OJMP;

    ops[4]=LOAD;
    ops[5]=STORE;
    ops[6]=LOADI;
    ops[7]=NOP;

    ops[8]=ADD;
    ops[9]=SUB;

    ops[10]=IN;
    ops[11]=OUT;

    ops[12]=EQU;
    ops[13]=LT;
    ops[14]=LTE;
    ops[15]=NOT;
}


#endif	/* SIMMACROS_H */

