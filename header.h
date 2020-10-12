#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#define SMALLER		-1
#define EQUAL		0
#define BIGGER		1
#define CONTINUE	2
#define END		    -2

#define DEFAULT_PORT    9999
#define DEFAULT_IP      "127.0.0.1"

#define	MUTEXWRITE 0

int sem_wait(int semid, int sem_num, int val) {
	struct sembuf op;

	op.sem_num = sem_num;
	op.sem_op = -val;
	op.sem_flg = 0;
	return semop(semid, &op, 1);
}

int sem_signal(int semid, int sem_num, int val) {
	struct sembuf op;

	op.sem_num = sem_num;
	op.sem_op = val;
	op.sem_flg = 0;
	return semop(semid, &op, 1);
}

int mutex_wait(int semid, int sem_num) {
	return sem_wait(semid, sem_num, 1);
}

int mutex_signal(int semid, int sem_num) {
	return sem_signal(semid, sem_num, 1);
}

#endif
