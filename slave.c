#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>
#include <string.h>

#define COMMAND_MAX PATH_MAX + 255
#define OUTPUT_MAX 1024

int processCNF(const char *path);

int main(int argc, const char* argv[]){
    if(argc < 2){
        return -1; //ver si hay que refinar el tratamiento de errores aca
    }
    
    int ret;

    for(int i = 1; i < argc; i++){
        ret = processCNF(argv[i]);
        //tratamiento de errores
    }

    //Los primeros programas los lee por parametro, el resto les llega por pipe
    //char pathBuffer[PATH_MAX];

    // int len;
    // while((len = read(0, pathBuffer, PATH_MAX)) != 0){
    //     ret = processCNF(pathBuffer);
    //     //tratamiento de errores del processCNF y del
    // }

    return 0;
}

int processCNF(const char *path){
        FILE *minisatStream;
        char cmdBuffer[COMMAND_MAX]; //cota superior para tamaño del string del comando
        char minisatOutput[OUTPUT_MAX];

        strcpy(cmdBuffer, "minisat ");
        strcat(cmdBuffer, path);
        strcat(cmdBuffer, " | ");
        strcat(cmdBuffer, "grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"");

        //con grep se morfa cuando un archivo es invalido, hay que validarlo aparte
        minisatStream = popen(cmdBuffer, "r");

        if(minisatStream == NULL){
            //validacion de errores
        }
        
        setvbuf(stdout, NULL, _IONBF, 0);

        int len = fread(minisatOutput, 1, OUTPUT_MAX, minisatStream);
        //write(1, minisatOutput, len);
        puts(path);
        pclose(minisatStream);

        return 0;
}
