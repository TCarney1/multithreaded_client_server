//
// Created by Tom Carney on 19/9/20.
//

#ifndef ASSESSMENT2_HEADER_H
#define ASSESSMENT2_HEADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>

int format_input(char *user_input, long *num_p, long *thread_p);

#define NOT_READY -1
#define FULL 0
#define EMPTY 1
#define BUFF_SIZE 64


// shared memory struct between client and server.
struct Memory{
    int status;
    int data[4];
};

#endif //ASSESSMENT2_HEADER_H
