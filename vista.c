#include "libinfo.h"

void readShm(char **currentShm, sem_t *availBlocks);

int main(int argc, const char *argv[]){
    int shmFD = -1, fileCount;
    char *shmBase;

    char shmName[1024], buffer[1024];
    sem_t *availBlocks;

    if(argc == 1){
        read(0, buffer, 1024);
        sscanf(buffer, "%s\n%d\n", shmName, &fileCount);
    }else if(argc == 3){
        strcpy(shmName, argv[1]);
        fileCount = atoi(argv[2]);
    }else{
        perror("CANTIDAD INCORRECTA DE PARAMETROS ?");
        exit(1);
    }

    shmFD = shm_open(shmName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);            

    if(shmFD == -1){
        perror("Error en creacion de memoria compartida.");
        exit(1);
    }

    shmBase = mmap(NULL, fileCount*BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
    if(shmBase == MAP_FAILED){
        perror("Error en mapeo");
        exit(1);
    }

    availBlocks = sem_open(AVAIL_SEM, O_CREAT, S_IRUSR | S_IWUSR, 0);
    
    char *currentShm = shmBase; 

    for(int i = 0; i < fileCount; i++){
        readShm(&currentShm, availBlocks);
    }
  
    if(munmap(shmBase, fileCount*BLOCK_SIZE) == -1){
        perror("Error en unmap.");
        exit(1);
    }

    if(close(shmFD) == -1){
        perror("Error en cierre de fd.");
        exit(1);
    }

    sem_close(availBlocks);

    return 0;
}

void readShm(char **currentShm, sem_t *availBlocks){
    sem_wait(availBlocks);
    
    printf("%s\n", *currentShm);
    fflush(stdout);
    *currentShm += BLOCK_SIZE; 
}
