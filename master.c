#include "libinfo.h"

int createSlaves(int pipesSM[][2], int pipesMS[][2], int *argsConsumed, int argc, const char *argv[]);

void defineSets(int activePipe[], fd_set *readSet, int pipesSM[][2], int slaveCount);

void runSelect(int pipesSM[][2], int pipesMS[][2], int slaveCount, int *argsConsumed, int argc, const char *argv[]);

int main(int argc, const char *argv[]){

    if(argc < 2){
        return -1; //ver si hay que refinar el tratamiento de errores aca
    }

    const char *shmName = "/cnfResults";
    int shmFD;
    char *shmBase;
    char *ptr;
    int shmFlag = 1;
    sem_t *emptyBlocks, *fullBlocks;

    shmFD = shm_open(shmName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
 
    if(shmFD == -1){
        perror("Error en creacion de memoria compartida.");
        exit(1);
    }

    ftruncate(shmFD, SHM_SIZE);
    shmBase = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
    if(shmBase == MAP_FAILED){
        perror("Error en mapeo");
        exit(1);
    }

    fullBlocks = sem_open(FULL_SEM, O_CREAT, S_IRUSR | S_IWUSR, 0);
    emptyBlocks = sem_open(EMPTY_SEM, O_CREAT, S_IRUSR | S_IWUSR, SHM_COUNT);

    write(1, shmName, strlen(shmName));
    sleep(SLEEPY_TIME);
    putchar('\n');

    if(strcmp(shmBase, CODE)){
        //no existe vista
        if(munmap(shmBase, SHM_SIZE) == -1){
            perror("Error en unmap.");
            exit(1);
        }

        if(close(shmFD) == -1){
            perror("Error en cierre de fd.");
            exit(1);
        }

        shm_unlink(shmName);

        sem_close(fullBlocks);
        sem_close(emptyBlocks);

        sem_unlink(FULL_SEM);
        sem_unlink(EMPTY_SEM);

        shmFlag = 0;
    }

    /*
    int argsConsumed = 1;           //Cantidad de argumentos leidos
    
    int pipesSM[MAX_SLAVE][2];                    //Pipes para entrada de datos desde los slaves al master
    int pipesMS[MAX_SLAVE][2];                   //Pipes para salida hacia los slaves
    int slaveCount;

    slaveCount = createSlaves(pipesSM, pipesMS, &argsConsumed, argc, argv);

    setvbuf(stdout, NULL, _IONBF, 0);

    runSelect(pipesSM, pipesMS, slaveCount, &argsConsumed, argc, argv);

    for(int i = 0; i < slaveCount; i++){
        wait(NULL);
    }
    */

    if(shmFlag){
        if(munmap(shmBase, SHM_SIZE) == -1){
            //FALTA UNLINK
            perror("Error en unmap.");
            exit(1);
        }

        if(close(shmFD) == -1){
            perror("Error en cierre de fd.");
            exit(1);
        }

        shm_unlink(shmName);

        sem_close(fullBlocks);
        sem_close(emptyBlocks);

        sem_unlink(FULL_SEM);
        sem_unlink(EMPTY_SEM);
    }

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

void runSelect(int pipesSM[][2], int pipesMS[][2], int slaveCount, int *argsConsumed, int argc, const char *argv[]){
    int finishedSlaves = 0;
    fd_set readSet;
    char done = DONE_CHAR;
    char buffer[1000];
    int n, result, maxFD;
    int activePipe[MAX_SLAVE];

    for (int i = 0; i < slaveCount; ++i){
        activePipe[i] = 1;
        //  Tengo que buscar el FD maximo para select()
        if (pipesSM[slaveCount][0] > maxFD && pipesSM[slaveCount][0] > pipesSM[slaveCount][1])
            maxFD = pipesSM[slaveCount][0];
        else if (pipesSM[slaveCount][1] > maxFD)
            maxFD = pipesSM[slaveCount][1];
    }

    while(finishedSlaves != slaveCount){
        defineSets(activePipe, &readSet, pipesSM, slaveCount);

        result = select(maxFD+1, &readSet, NULL, NULL, NULL);
        switch(result){
            case -1:
                    perror("MBEH");
                    exit(1); 
            case 0: //you shouldnt be here; 
                    break;
            default:
                for(int i = 0; i < slaveCount; i++){
                    if(FD_ISSET(pipesSM[i][0], &readSet)){
                        //Entonces tengo que leer del fd
                        n = read(pipesSM[i][0], buffer, 1000);
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
                                puts(buffer);
                                fflush(stdout);
                                buffer[0] = 0;
                                write(pipesMS[i][1], &done, 1);
                            }
                       }
                    }
                }
        }

    }
}
