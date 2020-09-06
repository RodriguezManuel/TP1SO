#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <stdlib.h>

#define MAX_SLAVE 10
#define FILECOUNT 5

void defineSets(int activePipe[], fd_set *readSet, int pipesSM[][2], int slaveCount);

int main(int argc, const char *argv[]){
    puts(argv[1]);

    if(argc < 2){
        return -1; //ver si hay que refinar el tratamiento de errores aca
    }
    //esperar a proceso vista y compartirle el buffer
    
    pid_t pidVec[MAX_SLAVE];        //Vectores de PIDs
    int argsConsumed = 1;           //Cantidad de argumentos leidos
    int execRet;                    //?????
    
    const char *pathVec[FILECOUNT+2];               //Argumentos para los slaves
    int pipesSM[MAX_SLAVE][2];                    //Pipes para entrada de datos desde los slaves al master
    int pipesMS[MAX_SLAVE][2];                   //Pipes para salida hacia los slaves
    int pipeRet;
    int slaveCount;
    int activePipe[MAX_SLAVE] = {0};
    int maxFD = 0;

    pathVec[0] = "slave";

    for(slaveCount = 0; slaveCount < MAX_SLAVE && argsConsumed < argc; slaveCount++){
        int j;

        for(j = 1; j <= FILECOUNT && argsConsumed < argc; j++)
            pathVec[j] = argv[argsConsumed++];

        pathVec[j] = NULL;

        pipeRet = pipe(pipesSM[slaveCount]);
        if(pipeRet == -1) {}          //tratamiento de errores
        if (pipesSM[slaveCount][0] > maxFD && pipesSM[slaveCount][0] > pipesSM[slaveCount][1])
            maxFD = pipesSM[slaveCount][0];
        else if (pipesSM[slaveCount][1] > maxFD)
            maxFD = pipesSM[slaveCount][1];

        pipeRet = pipe(pipesMS[slaveCount]);
        if(pipeRet == -1) {}          //tratamiento de errores
        if (pipesMS[slaveCount][0] > maxFD && pipesMS[slaveCount][0] > pipesMS[slaveCount][1])
            maxFD = pipesMS[slaveCount][0];
        else if (pipesMS[slaveCount][1] > maxFD)
            maxFD = pipesMS[slaveCount][1];

        pidVec[slaveCount] = fork();

        //tratamiento de errores fork
        if(pidVec[slaveCount] == 0){

            dup2(pipesSM[slaveCount][1], 1);
            
            dup2(pipesMS[slaveCount][0], 0);

            for(int i = 0; i <= slaveCount; i++){
                close(pipesSM[i][0]);
                close(pipesSM[i][1]);

                close(pipesMS[i][0]);
                close(pipesMS[i][1]);    
            }

            execRet = execv("slave", (char * const *) pathVec);
            //tratamiento de errores exec
        }

        close(pipesSM[slaveCount][1]);
        close(pipesMS[slaveCount][0]);

        activePipe[slaveCount] = 1;
    }

    maxFD++;

    fd_set readSet;
    int finishedSlaves = 0;

    setvbuf(stdout, NULL, _IONBF, 0);

    while(finishedSlaves != slaveCount){
        defineSets(activePipe, &readSet, pipesSM, slaveCount);

        int result = select(maxFD, &readSet, NULL, NULL, NULL);
        switch(result){
            case -1:
                    perror("MBEH");
                    exit(1); 
            case 0: //you shouldnt be here; 
                    break;
            default:
                for(int i = 0; i < slaveCount; i++){
                    // printf("%d",i);
                    if(FD_ISSET(pipesSM[i][0], &readSet)){
                        //Entonces tengo que leer del fd
                        char bufre[1000];
                        int n = read(pipesSM[i][0], bufre, 1000);
                        if(n == 0)
                        {
                            // tratamiento de errores
                            close(pipesSM[i][0]);
                            activePipe[i] = 0;
                        }else if(n == -1){
                            //  tratamiento de errores
                            printf("MBEHHHHH");
                        }
                        else{
                            if(bufre[0] == 3){
                                if(argsConsumed == argc){
                                    // no hay mas archivos para parsear, cierro el pipe (y se muere el slave)
                                    close(pipesSM[i][0]);
                                    close(pipesMS[i][1]);
                                    activePipe[i] = 0;
                                    finishedSlaves++;
                                }else{
                                    write(pipesMS[i][1], argv[argsConsumed], strlen(argv[argsConsumed])+1);
                                    argsConsumed++;
                                }
                            }else{
                                bufre[n] = 0;
                                puts(bufre);
                                fflush(stdout);
                                bufre[0] = 0;
                                char done = 03;
                                write(pipesMS[i][1], &done, 1);
                            }
                       }
                    }
                }
        }

    }

    for(int i = 0; i < slaveCount; i++){
        wait(NULL);
    }

    return 0;    
}

void defineSets(int activePipe[], fd_set *readSet, int pipesSM[][2], int slaveCount){
    FD_ZERO(readSet);

    for(int i = 0; i < slaveCount; i++){
        if(activePipe[i] == 1){
            FD_SET(pipesSM[i][0], readSet);
        }
    }
}
