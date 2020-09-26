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
#include <limits.h>
#include <sys/time.h>
#include <stdio.h>
#include <termios.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>


#define EMPTY 0
#define CLOSE -1
#define NEW_DATA 1
#define BUFF_SIZE 64
#define NUM_REQUESTS 10 // number of inputted numbers handled simultaneously
#define NUM_THREADS 32 // num threads per inputted number
#define SIZE 10 // loading bar size


// shared memory struct between client and server.
struct Memory{
    long number; // new number
    long slot[NUM_REQUESTS]; // slots for responses.
    int client_flag; // flag for new number
    int server_flag[NUM_REQUESTS]; // flags for response slots.
    long original_num[NUM_REQUESTS]; // track the original values.
    int threads_finished[NUM_REQUESTS]; // track the number of threads finished for each req.
    int index, current_slot; // index -> for bit rotation, cur_slot -> for temp slot value.
};

struct Node{
    long factor;
    struct Node* next;
};


//client functions
int format_input(char *user_input, long *num_p);
void *listen(void *arg);
void print_list(struct Node* n);
void push_front(struct Node ** head, long factor);
void delete(struct Node *head);
void delete_bar(int length);
void display_bar(long num, long full);
int get_length(long num);
void *loading_bar(void *arg);





//server functions
long bit_rotate_right(long num, unsigned int rotations);
void *find_factors(void *arg);
void delay(int milli);
void *solve(void* arg);
int slot_request(int server_flag[]);


#endif //ASSESSMENT2_HEADER_H
