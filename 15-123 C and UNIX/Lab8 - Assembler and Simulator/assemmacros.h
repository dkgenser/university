/*allows for simpler "switch" statement
 *defines all bit shifting that needs to occur*/

#ifndef MACROS_H
#define	MACROS_H

/*errors*/
#define INVALID_PARAM 1
#define INVALID_INST 2
#define INVALID_NUM_PARAM 3


/*register codes*/
#define Z_REG 0x0
#define A_REG 0x1
#define B_REG 0x2
#define C_REG 0x3
#define D_REG 0x4
#define E_REG 0x5
#define F_REG 0x6
#define G_REG 0x7

/*port codes*/
#define INPUT_PORT 0
#define OUTPUT_PORT 15
#define INPUT_PORT_CODE 0x00
#define OUTPUT_PORT_CODE 0x0f

/*op codes*/
#define HLT_OP 0x0
#define JMP_OP 0x1
#define CJMP_OP 0x2
#define OJMP_OP 0x3

#define LOAD_OP 0x4
#define STORE_OP 0x5
#define LOADI_OP 0x6
#define NOP_OP 0x7

#define ADD_OP 0x8
#define SUB_OP 0x9

#define IN_OP 0xa
#define OUT_OP 0xb

#define EQU_OP 0xc
#define LT_OP 0xd
#define LTE_OP 0xe
#define NOT_OP 0xf

/*bit shift codes*/
#define OP_SHIFT 28
#define REG0_SHIFT 24
#define REG1_SHIFT 20
#define REG2_SHIFT 16

/*functions to shift information into single 32bit integer*/
#define HLT() 0
#define JMP(op1) ((JMP_OP<<OP_SHIFT) | op1)
#define CJMP(op1) ((CJMP_OP<<OP_SHIFT) | op1)
#define OJMP(op1) ((OJMP_OP<<OP_SHIFT) | op1)

#define LOAD(dest, op1) ((LOAD_OP<<OP_SHIFT) | op1)
#define STORE(dest, op1) ((STORE_OP<<OP_SHIFT) | ((dest)<<REG0_SHIFT) | op1)
#define LOADI(dest, op1)  ((LOADI_OP<<OP_SHIFT) | ((dest)<<REG0_SHIFT) | op1)
#define NOP() (NOP_OP<<OP_SHIFT)

#define ADD(dest,op1,op2) ((ADD_OP<<OP_SHIFT) | ((dest)<<REG0_SHIFT) | ((op1)<<REG1_SHIFT) | ((op2)<<REG2_SHIFT))
#define SUB(dest,op1,op2) ((SUB_OP<<OP_SHIFT) | ((dest)<<REG0_SHIFT) | ((op1)<<REG1_SHIFT) | ((op2)<<REG2_SHIFT))

#define IN(dest,port) ((IN_OP<<OP_SHIFT) | ((dest)<<REG0_SHIFT) | port)
#define OUT(dest,port) ((OUT_OP<<OP_SHIFT) | ((dest)<<REG0_SHIFT) | port)

#define EQU(op1,op2) ((EQU_OP<<OP_SHIFT) | ((op1)<<REG0_SHIFT) | ((op2)<<REG1_SHIFT))
#define LT(op1,op2) ((LT_OP<<OP_SHIFT) | ((op1)<<REG0_SHIFT) | ((op2)<<REG1_SHIFT))
#define LTE(op1,op2) ((LTE_OP<<OP_SHIFT) | ((op1)<<REG0_SHIFT) | ((op2)<<REG1_SHIFT))
#define NOT() (NOT_OP<<OP_SHIFT)

/*stor stands for string to register code
 *returns -1 if the value passed was invalid*/
int stor(const char *str){
    if(0==strcmp(str,"Z"))
        return Z_REG;
    else if(0==strcmp(str,"A"))
        return A_REG;
    else if(0==strcmp(str,"B"))
        return B_REG;
    else if(0==strcmp(str,"C"))
        return C_REG;
    else if(0==strcmp(str,"D"))
        return D_REG;
    else if(0==strcmp(str,"E"))
        return E_REG;
    else if(0==strcmp(str,"F"))
        return F_REG;
    else if(0==strcmp(str,"G"))
        return G_REG;
    else
        return -1;
}

/*itop stands for integer to port code
 *returns -1 if the value passed was invalid*/
int itop(const int port){
    if(INPUT_PORT==port)
        return INPUT_PORT_CODE;
    else if(OUTPUT_PORT==port)
        return OUTPUT_PORT_CODE;
    else
        return -1;
}

#endif	/* MACROS_H */

