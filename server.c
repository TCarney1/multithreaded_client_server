#include "header.h"

// NOTE: make sure you compile with -lpthread to allow threads to be used.

//*************************************************************************
// this program queries the user for a number (32-bit integer).
// The program then makes 31 individual numbers, each by bit rotating
// right. The program then finds all the factors of all 32 numbers.
// This is done by given each number its own thread, and solving
// simultaneously. Up to 10 requests can be made at the same time.
// Resulting in up to 320 threads simultaneously solving factors.
//*************************************************************************


int main() {
    key_t shm_key;
    int shm_id;
    struct Memory *shm_ptr;

    shm_key = ftok(".", 'x'); // this has to be the same as client.
    shm_id = shmget(shm_key, sizeof(struct Memory), 0666);
    if(shm_id < 0){
        printf("Error with shmget.\n");
        exit(EXIT_FAILURE);
    }

    printf("--- server received shred memory ---\n");

    shm_ptr = (struct Memory *)shmat(shm_id, NULL, 0);

    if((int) shm_ptr == -1){
        printf("Error with shmat.\n");
        exit(EXIT_FAILURE);
    }
    pthread_t tid[NUM_REQUESTS];
    printf("--- server memory attached ---\n");
    while(1) {
        // what for client to send data.
        while (shm_ptr->client_flag == EMPTY){
            ;
        }
        // quit
        if (shm_ptr->client_flag == CLOSE) {
            shmdt((void *) shm_ptr);
            printf("--- memory detached ---\n");
            printf("--- quiting... ---\n");
            exit(EXIT_SUCCESS);
        }
        // number entered.
        if(shm_ptr->client_flag == NEW_DATA) {

            shm_ptr->current_slot = slot_request(shm_ptr->server_flag); // get index of next avail slot
            shm_ptr->original_num[shm_ptr->current_slot] = shm_ptr->number; // keep track of original number
            shm_ptr->threads_finished[shm_ptr->current_slot] = 0;

            // if we arent full, make threads for client request.
            if(shm_ptr->current_slot >= 0) {
                // for each request
                // create 32 threads to solve simultaneously
                pthread_create(&tid[shm_ptr->current_slot], NULL, solve, (void *) shm_ptr);
            } else {
                printf("--- cannot add request: server full ---\n");
            }
            shm_ptr->number = shm_ptr->current_slot; // tell client index of next avail slot. (-1 if full)
            // tell client they can replace the number.
            shm_ptr->client_flag = EMPTY;
        }
    }
}


// returns index of next available slot, or -1 for full.
int slot_request(int server_flag[]){
    for(int i = 0; i < NUM_REQUESTS; i++){
        if(server_flag[i] == CLOSE){ // if slot not being used.
            server_flag[i] = EMPTY; // use it.
            return i;
        }
    }
    return -1;
}

// main driving function for solution.
// creates 32 threads.
// each thread finds all the factors for a different number.
void *solve(void* arg){
    struct Memory *m = (struct Memory *) arg;
    int slot_num = m->current_slot;
    long loc = m->original_num[slot_num];

    pthread_t tid[NUM_THREADS];
    printf("Started: %ld Slot: %d\n", loc, slot_num);
    // num threads is just for 1 request.
    // so overall threads is NUM_THREADS * requests.
    // This just starts 32 threads, 1 for each bit rotated num.
    for(int i = 0; i < NUM_THREADS; i++){
        m->index = i;
        pthread_create(&tid[i], NULL, find_factors, (void *) m);
        delay(30);
    }
    for(int i = 0; i < NUM_THREADS; i++){
        pthread_join(tid[i], NULL);
    }
    // we are finished with the slot.
    m->server_flag[slot_num] = CLOSE; // set slot flag to CLOSE so it can be reused.
    printf("Finished: %ld\n", loc);
}


// each thread executes this function.
// finds all the factors of its number and then sends/waits
// to send results back to client in the right slot.
void *find_factors(void *arg){
    struct Memory *m = (struct Memory*) arg;
    int slot_num = m->current_slot;
    long num = m->original_num[slot_num];
    int rotations = m->index;
    long original = num;
    // bit rotates num by its index.
    num = bit_rotate_right(num, rotations);
    //printf("Num: %ld\n", num);
    //printf("index: %d\n", rotations);
    //delay(100 * NUM_THREADS); // delay all threads until all threads have started.
    for(long i = 1; i <= num; i++){
        if(num % i == 0){
            while (m->server_flag[slot_num] != EMPTY)
                ;
            m->slot[slot_num] = i;
            m->server_flag[slot_num] = NEW_DATA;
           // printf("OG: %ld Num: %ld -- Factor: %ld\n", original, num, i);
        }
    }
    m->threads_finished[slot_num]++;
    return (void*) 0;
}


// bit rotates right an integer of data type long n number of rotations.
long bit_rotate_right(long num, unsigned int rotations) {
    int dropped;
    int bits = sizeof(int) * 8 - 1;
    while (rotations--) {
        dropped = num & 1; // bitwise AND
        num = (num >> 1) & (~(1 << bits)); // ~ = bitwise compliment
        num = num | (dropped << bits); // bitwise OR
    }
    return num;
}


// delays for n milliseconds.
void delay(int milli){
    long pause;
    clock_t now,then;

    pause = milli*(CLOCKS_PER_SEC/1000);
    then = clock();
    now = then;
    while( (now-then) < pause )
        now = clock();
}




