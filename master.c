#include "libinfo.h"

int createSlaves(int pipesSM[][2], int pipesMS[][2], int *argsConsumed, int argc, const char *argv[]);

void defineSets(int activePipe[], fd_set *readSet, int pipesSM[][2], int slaveCount);

void runSelect(int pipesSM[][2], int pipesMS[][2], int slaveCount, int *argsConsumed, int argc, const char *argv[], 
                sem_t *availBlocks, char *currentShm, FILE* resultFile);

void writeShm(char **currentShm, char *shmBase, char *output, sem_t *availBlocks, int fileCount);

int main(int argc, const char *argv[]){

    if(argc < 2){
        return -1; //ver si hay que refinar el tratamiento de errores aca
    }

    int shmFD, ftruncRet;
    char *shmBase;
    sem_t *availBlocks;

    FILE* resultFile = fopen("resultFile.txt","w");
    if(resultFile == NULL){
        perror("Error en apertura de archivo");
        exit(1);
    }


    shmFD = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
 
    if(shmFD == -1){
        perror("Error en creacion de memoria compartida.");
        exit(1);
    }

    ftruncRet = ftruncate(shmFD, (argc-1)*BLOCK_SIZE);
    if(ftruncRet == -1){
        perror("Error en asignacion de memoria compartida");
        exit(1);
    }

    shmBase = mmap(NULL, (argc-1)*BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
    if(shmBase == MAP_FAILED){
        perror("Error en mapeo");
        exit(1);
    }

    setvbuf(stdout, NULL, _IONBF, 0);

    availBlocks = sem_open(AVAIL_SEM, O_CREAT, S_IRUSR | S_IWUSR, 0);

    printf("%d\n", argc-1);
    sleep(SLEEPY_TIME);
 
    int argsConsumed = 1;           //Cantidad de argumentos leidos
    
    int pipesSM[MAX_SLAVE][2];                    //Pipes para entrada de datos desde los slaves al master
    int pipesMS[MAX_SLAVE][2];                   //Pipes para salida hacia los slaves
    int slaveCount;

    slaveCount = createSlaves(pipesSM, pipesMS, &argsConsumed, argc, argv);

    runSelect(pipesSM, pipesMS, slaveCount, &argsConsumed, argc, argv, availBlocks, shmBase, resultFile);

    for(int i = 0; i < slaveCount; i++){
        wait(NULL);
    }

    if(shmBase != NULL){
        if(munmap(shmBase, (argc-1)*BLOCK_SIZE) == -1){
            //FALTA UNLINK
            perror("Error en unmap.");
            exit(1);
        }

        if(close(shmFD) == -1){
            perror("Error en cierre de fd.");
            exit(1);
        }

        shm_unlink(SHM_NAME);

        sem_close(availBlocks);

        sem_unlink(AVAIL_SEM);
    }

    fclose(resultFile);

    return 0;    

}

int createSlaves(int pipesSM[][2], int pipesMS[][2], int *argsConsumed, int argc, const char *argv[]){
    const char *pathVec[FILECOUNT+2];               //Argumentos para los slaves
    int slaveCount, pipeRet;
    pid_t slavePID;
    int execRet;                    //?????

    pathVec[0] = "slave";

    for(slaveCount = 0; slaveCount < MAX_SLAVE && *argsConsumed < argc; slaveCount++){
        int j;

        for(j = 1; j <= FILECOUNT && *argsConsumed < argc; j++)
            pathVec[j] = argv[(*argsConsumed)++];

        pathVec[j] = NULL;

        pipeRet = pipe(pipesSM[slaveCount]);
        if(pipeRet == -1) {}          //tratamiento de errores

        pipeRet = pipe(pipesMS[slaveCount]);
        if(pipeRet == -1) {}          //tratamiento de errores

        slavePID = fork();

        //tratamiento de errores fork
        if(slavePID == 0){
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
        } else if (slavePID == -1){
            //tratamiento de errores
        }

        close(pipesSM[slaveCount][1]);
        close(pipesMS[slaveCount][0]);
    }

    return slaveCount;
}

void defineSets(int activePipe[], fd_set *readSet, int pipesSM[][2], int slaveCount){
    FD_ZERO(readSet);

    for(int i = 0; i < slaveCount; i++){
        if(activePipe[i] == 1){
            FD_SET(pipesSM[i][0], readSet);
        }
    }
}

void runSelect(int pipesSM[][2], int pipesMS[][2], int slaveCount, int *argsConsumed, int argc, const char *argv[], 
                sem_t *availBlocks, char *currentShm, FILE* resultFile){
    int finishedSlaves = 0;
    fd_set readSet;
    char done = DONE_CHAR;
    char buffer[1000];
    int n, result, maxFD = 0;
    int activePipe[MAX_SLAVE];
    char *shmBase = currentShm;

    for (int i = 0; i < slaveCount; ++i){
        activePipe[i] = 1;
        //  Tengo que buscar el FD maximo para select()
        if (pipesSM[i][0] > maxFD )
            maxFD = pipesSM[i][0];
    }

    while(finishedSlaves != slaveCount){
        defineSets(activePipe, &readSet, pipesSM, slaveCount);

        result = select(maxFD+1, &readSet, NULL, NULL, NULL);
        switch(result){
            case -1:
                    perror("Error en Select");
                    exit(1); 
            case 0: //you shouldnt be here; 
                    break;
            default:
                for(int i = 0; i < slaveCount; i++){
                    if(FD_ISSET(pipesSM[i][0], &readSet)){
                        //Entonces tengo que leer del fd
                        n = read(pipesSM[i][0], buffer, 999);
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
                            if(buffer[0] == DONE_CHAR){
                                if(*argsConsumed == argc){
                                    // no hay mas archivos para parsear, cierro el pipe (y se muere el slave)
                                    close(pipesSM[i][0]);
                                    close(pipesMS[i][1]);
                                    activePipe[i] = 0;
                                    finishedSlaves++;
                                }else{
                                    write(pipesMS[i][1], argv[*argsConsumed], strlen(argv[*argsConsumed])+1);
                                    (*argsConsumed)++;
                                }
                            }else{
                                buffer[n] = 0;
                                writeShm(&currentShm, shmBase, buffer, availBlocks, argc-1);
                                fprintf(resultFile,"%s\n\n", buffer);
                                //y escribirle al archivo
                                buffer[0] = 0;
                                write(pipesMS[i][1], &done, 1);
                            }
                       }
                    }
                }
        }

    }
}

void writeShm(char **currentShm, char *shmBase, char *output, sem_t *availBlocks, int fileCount){
    if(*currentShm - shmBase == (fileCount)*BLOCK_SIZE){
        exit(1);//tratamiento de errores (te quedaste sin memoria)
    }
    
    sprintf(*currentShm, "%s\n", output);

    *currentShm += BLOCK_SIZE; 
    sem_post(availBlocks);
}
