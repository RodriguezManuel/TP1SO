#include "libinfo.h"

int processCNF(const char *path);

int main(int argc, const char* argv[]){
    if(argc < 2){
        perror("Cantidad insuficiente de argumentos");
        exit(1);
    }

    int len;
    char done = DONE_CHAR, pathBuffer[PATH_MAX];
   
    setvbuf(stdout, NULL, _IONBF, 0);

    for(int i = 1; i < argc; i++){
        processCNF(argv[i]);
    }
    
    //Los primeros programas los lee por parametro, el resto les llega por pipe
    do{
        write(1, &done, 1);
        len = read(0, pathBuffer, PATH_MAX);
        if(len > 0)
            processCNF(pathBuffer);

    } while (len > 0);
    
    exit(0);
}

int processCNF(const char *path){
    int length, ret = 0;
    char result[4096];
    if(access(path, R_OK) == -1){
        ret = 1;
        length = sprintf(result, "Error al querer acceder archivo CNF: %s\nQuizas no existe o no tiene permisos de lectura (processCNF)", path);
        //si no existe o no tiene permisos, lo ignoro; no valdria la pena abortar
    }else{
        FILE *minisatStream;
        char cmdBuffer[COMMAND_MAX]; //cota superior para tamaño del string del comando
        char minisatOutput[OUTPUT_MAX+1];

        strcpy(cmdBuffer, "minisat ");
        strcat(cmdBuffer, path);
        strcat(cmdBuffer, " | ");
        strcat(cmdBuffer, "grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"");
    
        minisatStream = popen(cmdBuffer, "r");

        if(minisatStream == NULL){
            perror("Error al iniciar proceso de minisat y grep (processCNF)");
            exit(1);
        }

        length = fread(minisatOutput, 1, OUTPUT_MAX, minisatStream);
        pclose(minisatStream);
        minisatOutput[length] = 0;

        length = sprintf(result, "SlavePID = %d\n%s%s", getpid(), minisatOutput, path);
    }
    
    write(1, result, length);

    //Espero que master me diga que puedo seguir
    while(getchar() != DONE_CHAR);

    return ret;
}
