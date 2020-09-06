#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>



#define MAX_SLAVE 10
#define FILECOUNT 5
#define DONE_CHAR 3         //  Char que indica que está todo ok
#define SHM_SIZE 4096

int createSlaves(int pipesSM[][2], int pipesMS[][2], int *argsConsumed, int argc, const char *argv[]);

void defineSets(int activePipe[], fd_set *readSet, int pipesSM[][2], int slaveCount);

void runSelect(int pipesSM[][2], int pipesMS[][2], int slaveCount, int *argsConsumed, int argc, const char *argv[]);

extern sem_t *sem_open(const char *__name, int __oflag, ...);

int main(int argc, const char *argv[]){

    if(argc < 2){
        return -1; //ver si hay que refinar el tratamiento de errores aca
    }

    const char *name = "/cnfResults";
    int shm_fd;
    char *shm_base;
    char *ptr;

    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    
    if(shm_fd == -1){
        perror("Error en creacion de memoria compartida.");
        exit(1);
    }

    ftruncate(shm_fd, SHM_SIZE);
    shm_base = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shm_base == MAP_FAILED){
        perror("Error en mapeo");
        exit(1);
    }

    write(1, name, strlen(name));
    //sleep    



    sem_t *empty = sem_open("/empty", O_CREAT);
    sem_t *full = sem_open("/full", O_CREAT);

    if(!strcmp(shm_base, "MBEH"))
        while(1);






    if(munmap(shm_base, SHM_SIZE) == -1){
        //FALTA UNLINK
        perror("Error en unmap.");
        exit(1);
    }

    if(close(shm_fd) == -1){
        perror("Error en cierre de fd.");
        exit(1);
    }

    sem_close(empty);
    sem_close(full);
    //esperar a proceso vista y compartirle el buffer
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
