#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_SLAVE 10
#define FILECOUNT 5

int main(int argc, const char *argv[]){
    if(argc < 2){
        return -1; //ver si hay que refinar el tratamiento de errores aca
    }
    //esperar a proceso vista y compartirle el buffer
    
    pid_t pidVec[MAX_SLAVE];
    int argsConsumed = 1;
    int execRet;
    
    const char *pathVec[FILECOUNT+2];
    pathVec[0] = "slave";
    pathVec[FILECOUNT+1] = NULL;

    for(int i = 0; i < MAX_SLAVE; i++){
        for(int j = 1; j < FILECOUNT+1 && argsConsumed < argc; j++)
            pathVec[j] = argv[argsConsumed++];
        
        if(argsConsumed == argc)
            break;

        pidVec[i] = fork();

        //tratamiento de errores fork
        if(pidVec[i] == 0){
            execRet = execv("slave", (char * const *) pathVec);
            //tratamiento de errores exec
        }
    }

    for(int i = 0; i < MAX_SLAVE; i++){
        wait(NULL);
    }

    return 0;    
}