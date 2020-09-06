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

#define SLEEPY_TIME 10

//	Info para la creacion de esclavos
#define MAX_SLAVE 10
#define FILECOUNT 5
#define DONE_CHAR 3         //  Char de comunicacion para pipes

#define COMMAND_MAX PATH_MAX + 255	// Tamaño max para popen
#define OUTPUT_MAX 1024				// Tamaño de la info que devuelve minisat

//	Info para buffer de memoria compartida
#define SHM_COUNT 50
#define BLOCK_SIZE 2000
#define SHM_SIZE SHM_COUNT*BLOCK_SIZE
#define CODE "MBEH"

//	Nombres de los semaforos
#define FULL_SEM "/fullBlocks"		//Sem que cuenta bloques displonibles para lectura
#define EMPTY_SEM "/emptyBlocks"	//Sem que cuenta bloques displonibles para escritura

#endif
