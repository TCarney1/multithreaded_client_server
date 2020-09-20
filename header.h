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
#include <pthread.h>

//client functions
int format_input(char *user_input, long *num_p);




//server functions
long bit_rotate_right(long num, unsigned int rotations);
void *find_factors(void *arg);
void delay(int milli);
void *solve(void* num);
int slot_request(int server_flag[]);

#define EMPTY 0
#define QUIT -1
#define NEW_DATA 1
#define BUFF_SIZE 64
#define NUM_REQUESTS 10 // number of inputted numbers handled simultaneously
#define NUM_THREADS 32 // num threads per inputted number


// shared memory struct between client and server.
struct Memory{
    long number; // new number
    long slot[NUM_REQUESTS]; // slots for responses.
    int client_flag; // flag for new number
    int server_flag[NUM_REQUESTS]; // flags for response slots.
    int index, current_slot;
};





#endif //ASSESSMENT2_HEADER_H
