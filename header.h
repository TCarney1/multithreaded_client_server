//
// Created by Tom Carney on 19/9/20.
//

#ifndef ASSESSMENT2_HEADER_H
#define ASSESSMENT2_HEADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>

int format_input(char *user_input, long *num_p);

#define EMPTY 0
#define QUIT -1
#define NEW_DATA 1
#define BUFF_SIZE 64
#define NUM_REQUESTS 10


// shared memory struct between client and server.
struct Memory{
    int client_flag; // flag for new number
    long number; // new number
    int server_flag[NUM_REQUESTS]; // flags for response slots.
    long slot[NUM_REQUESTS]; // slots for responses.
};

#endif //ASSESSMENT2_HEADER_H
