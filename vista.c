#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

#define SHM_SIZE 4096

int main(int argc, const char *argv[]){
    int shm_fd;
    char *shm_base;
    char *ptr;

    char name[1024];
    read(0, name, 1024);

    shm_fd = shm_open(name, O_RDWR, 0666);
    if(shm_fd == -1){
        perror("Error en creacion de memoria compartida.");
        exit(1);
    }

    shm_base = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shm_base == MAP_FAILED){
        perror("Error en mapeo");
        exit(1);
    }

    ptr = shm_base;
    ptr += sprintf(ptr, "%s", "MBEH");


    

    return 0;
}