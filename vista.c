#include "libinfo.h"

void readShm(char **currentShm, sem_t *availBlocks);

int main(int argc, const char *argv[]){
    errno = 0;
    int shmFD = -1, fileCount;
    char *shmBase;

    sem_t *availBlocks;

    setvbuf(stdout, NULL, _IONBF, 0);

    if(argc == 1){
        char buffer[32];
        int n = read(0, buffer, 31);
        if (n > 0){
            buffer[n] = 0;
            fileCount = atoi(buffer);
        } else {
            perror("Cantidad incorrecta de argumentos en Vista (main)");
            exit(1);
        }
    } else if(argc == 2){
        fileCount = atoi(argv[1]);
    } else{
        perror("Cantidad incorrecta de argumentos en Vista (main)");
        exit(1);
    }

    shmFD = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);            

    if(shmFD == -1){
        perror("Error en creacion de memoria compartida en Vista (main)");
        exit(1);
    }

    shmBase = mmap(NULL, fileCount*BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
    if(shmBase == MAP_FAILED){
        perror("Error en mmap en Vista (main)");
        exit(1);
    }

    availBlocks = sem_open(AVAIL_SEM, O_CREAT, S_IRUSR | S_IWUSR, 0);
    
    char *currentShm = shmBase; 

    for(int i = 0; i < fileCount; i++){
        readShm(&currentShm, availBlocks);
    }
  
    if(munmap(shmBase, fileCount*BLOCK_SIZE) == -1){
        perror("Error en munmap en Vista (main)");
        exit(1);
    }

    close(shmFD);
    sem_close(availBlocks);

    return 0;
}

void readShm(char **currentShm, sem_t *availBlocks){
    sem_wait(availBlocks);
    
    printf("%s\n", *currentShm);
    
    *currentShm += BLOCK_SIZE; 
}
