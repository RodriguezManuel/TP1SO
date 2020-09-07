#ifndef LIBINFO_H_

#define LIBINFO_H_

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

#define SLEEPY_TIME 2

//	Info para la creacion de esclavos
#define MAX_SLAVE 10
#define FILECOUNT 5
#define DONE_CHAR 3         //  Char de comunicacion para pipes

#define COMMAND_MAX (PATH_MAX + 255)	// Tamaño max para popen
#define OUTPUT_MAX 1024					// Tamaño de la info que devuelve minisat

//	Info para buffer de memoria compartida
#define SHM_NAME "/cnfResults"
#define BLOCK_SIZE 1500

#define AVAIL_SEM "/availBlocks"		//Semaforo que cuenta bloques disponibles para lectura

#endif
