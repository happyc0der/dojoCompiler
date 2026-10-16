#include <stdio.h>

int main(int argc, char *argv[]){
    if (argc < 2){
        fprintf(stderr,"usage: mycc [--lex|--parse|--codegen|-S] file.c\n");
        return 1;
    }
    else{
        fprintf(stderr,"not implemented yet\n");
        return 1;
    }
    return 0;
}
