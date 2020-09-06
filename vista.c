#include "libinfo.h"

int main(int argc, const char *argv[]){
    int shmFD = -1;
    char *shmBase;

    char buffer[1024];
    char const *shmName;
    sem_t *emptyBlocks, *fullBlocks;

    if(argc == 1){
        read(0, buffer, 1024);
        shmName = buffer;
    }else if(argc == 2){
        shmName = argv[1];
    }else{
        printf("CANTIDAD INCORRECTA DE PARAMETROS ?");
        exit(1);
    }

    shmFD = shm_open(shmName, O_RDWR, S_IRUSR | S_IWUSR);            

    if(shmFD == -1){
        perror("Error en creacion de memoria compartida.");
        exit(1);
    }

    shmBase = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
    if(shmBase == MAP_FAILED){
        perror("Error en mapeo");
        exit(1);
    }

    sprintf(shmBase, "%s", CODE);
    printf("%s\n", shmBase);

    fullBlocks = sem_open(FULL_SEM, O_CREAT, S_IRUSR | S_IWUSR, 0);
    emptyBlocks = sem_open(EMPTY_SEM, O_CREAT, S_IRUSR | S_IWUSR, SHM_COUNT);

    if(munmap(shmBase, SHM_SIZE) == -1){
        perror("Error en unmap.");
        exit(1);
    }

    if(close(shmFD) == -1){
        perror("Error en cierre de fd.");
        exit(1);
    }

    sem_close(fullBlocks);
    sem_close(emptyBlocks);

    return 0;
}
