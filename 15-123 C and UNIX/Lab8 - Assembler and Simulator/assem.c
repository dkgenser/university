#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assemmacros.h"

int main(int argc, char *argv[]){
    /*argv[1] is the input file
     *argv[2] is the output file*/

    /*declaration of variables*/
    char mystring[100];
    char *ifile, *ofile;
    FILE *iFile, *oFile;
    int linenum, estat, hlt;

    /*check command line arguments*/
    if(argc!=3){
        printf("Usage: sas input.s output.o\n");
        return -1;
    }

    /*prepare files for reading and writing*/
    ifile = calloc(strlen(argv[1])+1,sizeof(char));
    strcpy(ifile,argv[1]);
    ofile = calloc(strlen(argv[2])+1,sizeof(char));
    strcpy(ofile,argv[2]);

    iFile = fopen(ifile, "r");
    oFile = fopen(ofile, "a");

    /*if successful, parse each line of code in input file*/
    if(NULL!=iFile&&NULL!=oFile){
        linenum=estat=hlt=0;
    while(fgets(mystring,100,iFile)){
        /*declaration of variables*/
        char instruction[10], arg0[20], arg1[20], arg2[20];
        int num, dest, op1, op2;

        /*"remove" comments from readable section of mystring*/
        char * comment = strchr(mystring,'#');
        if(comment) *comment = '\0';

        linenum++;
        
        num = sscanf(mystring,"%s %s %s %s",instruction,arg0,arg1,arg2);
        if(0==num||-1==num){
            continue;
        }

        /*at somepoint do checking if values passed themselves are gross*/
        if(0==strcmp("HLT",instruction)){
            if(1!=num){
                printf("Warning: Invalid Argument after HLT at line:%d\n",linenum);
            }
            fprintf (oFile,"0x%08x\n", HLT());
            hlt=0;
            
        }else if(0==strcmp("JMP",instruction)){
            if(2!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                sscanf(mystring, "%s %i",instruction, &op1);
                fprintf (oFile,"0x%x\n", JMP(op1));
            }
            
        }else if(0==strcmp("CJMP",instruction)){
            if(2!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                sscanf(mystring, "%s %i",instruction, &op1);
                fprintf (oFile,"0x%x\n", CJMP(op1));
            }
            
        }else if(0==strcmp("OJMP",instruction)){
            if(2!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                sscanf(mystring, "%s %i",instruction, &op1);
                fprintf (oFile,"0x%x\n", OJMP(op1));
            }
            
        }else if(0==strcmp("LOAD",instruction)){
            if(3!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                sscanf(arg0, "%i",&op1);
                dest=stor(arg0);
                if((-1)!=(dest))
                    fprintf (oFile,"0x%x\n", LOAD(dest,op1));
                else {
                    printf("Error: Invalid Register, Port, or Argument at line:%d\n",linenum);
                    estat=INVALID_PARAM;
                }
            }
            
        }else if(0==strcmp("STORE",instruction)){
            if(3!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                sscanf(mystring, "%s %s %i",instruction, arg0, &op1);
                dest=stor(arg0);
                if((-1)!=(dest))
                    fprintf (oFile,"0x%x\n", STORE(dest,op1));
                else{
                    printf("Error: Invalid Register, Port, or Argument at line:%d\n",linenum);
                    estat=INVALID_PARAM;
                }
            }
            
        }else if(0==strcmp("LOADI",instruction)){
            if(3!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                sscanf(arg1, "%i",&op1);
                dest=stor(arg0);
                if((-1)!=(dest))
                    fprintf (oFile,"0x%x\n", LOADI(dest,op1));
                else{
                    printf("Error: Invalid Register, Port, or Argument at line:%d\n",linenum);
                    estat=INVALID_PARAM;
                }
            }
            
        }else if(0==strcmp("NOP",instruction)){
            if(1!=num){
                printf("Warning: Invalid Argument after NOP at line:%d\n", linenum);
            }
            fprintf (oFile,"0x%x\n", NOP());
            
        }else if(0==strcmp("ADD",instruction)){
            if(4!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                dest=stor(arg0);
                op1=stor(arg1);
                op2=stor(arg2);
                if((-1)!=(dest)&&(-1)!=(op1)&&(-1)!=(op2))
                    fprintf (oFile,"0x%x\n", ADD(dest,op1,op2));
                else{
                    printf("Error: Invalid Register, Port, or Argument at line:%d\n",linenum);
                    estat=INVALID_PARAM;
                }
            }
            
        }else if(0==strcmp("SUB",instruction)){
            if(4!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                dest=stor(arg0);
                op1=stor(arg1);
                op2=stor(arg2);
                if((-1)!=(dest)&&(-1)!=(op1)&&(-1)!=(op2))
                    fprintf (oFile,"0x%x\n", SUB(dest,op1,op2));
                else{
                    printf("Error: Invalid Register, Port, or Argument at line:%d\n",linenum);
                    estat=INVALID_PARAM;
                }
            }
            
        }else if(0==strcmp("IN",instruction)){
            if(3!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                sscanf(mystring, "%s %s %i",instruction, arg0, &op1);
                dest=stor(arg0);
                op1=itop(op1);
                if((-1)!=(dest)&&(-1)!=(op1))
                    fprintf (oFile,"0x%x\n", IN(dest,op1));
                else{
                    printf("Error: Invalid Register, Port, or Argument at line:%d\n",linenum);
                    estat=INVALID_PARAM;
                }
            }
            
        }else if(0==strcmp("OUT",instruction)){
            if(3!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                sscanf(mystring, "%s %s %i",instruction, arg0, &op1);
                dest=stor(arg0);
                op1=itop(op1);
                if((-1)!=(dest)&&(-1)!=(op1))
                    fprintf (oFile,"0x%x\n", OUT(dest,op1));
                else {
                    printf("Error: Invalid Register, Port, or Argument at line:%d\n",linenum);
                    estat=INVALID_PARAM;
                }
            }
            
        }else if(0==strcmp("EQU",instruction)){
            if(3!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                sscanf(mystring, "%s %s %s",instruction, arg0, arg1);
                op1=stor(arg0);
                op2=stor(arg1);
                if((-1)!=(op1)&&(-1)!=(op2))
                    fprintf (oFile,"0x%x\n", EQU(op1,op2));
                else {
                    printf("Error: Invalid Register, Port, or Argument at line:%d\n",linenum);
                    estat=INVALID_PARAM;
                }
            }
            
        }else if(0==strcmp("LT",instruction)){
            if(3!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                sscanf(mystring, "%s %s %s",instruction, arg0, arg1);
                op1=stor(arg0);
                op2=stor(arg1);
                if((-1)!=(op1)&&(-1)!=(op2))
                    fprintf (oFile,"0x%x\n", LT(op1,op2));
                else{
                    printf("Error: Invalid Register, Port, or Argument at line:%d\n",linenum);
                    estat=INVALID_PARAM;
                }
            }
            
        }else if(0==strcmp("LTE",instruction)){
            if(3!=num){
                printf("Error: Too few or too many arguments at line:%d\n",linenum);
                estat=INVALID_NUM_PARAM;
            }else{
                sscanf(mystring, "%s %s %s",instruction, arg0, arg1);
                op1=stor(arg0);
                op2=stor(arg1);
                if((-1)!=(op1)&&(-1)!=(op2))
                    fprintf (oFile,"0x%x\n", LTE(op1,op2));
                else{
                    printf("Error: Invalid Register, Port, or Argument at line:%d\n",linenum);
                    estat=INVALID_PARAM;
                }
            }
            
        }else if(0==strcmp("NOT",instruction)){
            if(1!=num){
                printf("Warning: Invalid Argument after NOT at line:%d\n",linenum);
            }
            fprintf (oFile,"0x%x\n", NOT());
            
        }else{
            printf("Error:Invalid Instruction\n");
            estat = INVALID_INST;
        }

        

    }

    fclose(iFile);
    fclose(oFile);
    if(0!=estat||hlt){
            if(hlt) printf("Error: End of program without exit");
            remove(ofile);
            return (-1);
        }
    }
    else{
        printf("Input or Output file invalid");
        return (-1);
    }
    
    
    return(EXIT_SUCCESS);
}
