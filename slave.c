#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>
#include <string.h>

#define COMMAND_MAX PATH_MAX + 255
#define OUTPUT_MAX 1024

int main(int argc, const char* argv[]){
    if(argc < 2){
        return -1; //ver si hay que refinar el tratamiento de errores aca
    }
    
    FILE *minisatStream;
    char cmdBuffer[COMMAND_MAX]; //cota superior para tamaño del string del comando
    char minisatOutput[OUTPUT_MAX];
    
    int i;
    for(i = 1; i < argc; i++){
        strcpy(cmdBuffer, "minisat ");
        strcat(cmdBuffer, argv[i]);
        strcat(cmdBuffer, " | ");
        strcat(cmdBuffer, "grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"");

        minisatStream = popen(cmdBuffer, "r");

        if(minisatStream == NULL){
            //validacion de errores
        }

        int len = fread(minisatOutput, 1, OUTPUT_MAX, minisatStream);
        write(1, minisatOutput, len);
        pclose(minisatStream);
    }

    return 0;
}
