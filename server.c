#include "header.h"

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

    printf("--- server memory attached ---\n");
    while(1) {
        // what for client to send data.
        while (shm_ptr->client_flag == EMPTY){
            ;
        }
        // quit
        if (shm_ptr->client_flag == QUIT) {
            shmdt((void *) shm_ptr);
            printf("--- memory detached ---\n");
            printf("--- quiting... ---\n");
            exit(EXIT_SUCCESS);
        }
        // number entered.
        if(shm_ptr->client_flag == NEW_DATA) {
            // local data so we can keep track of current vars.
            struct Memory m = *shm_ptr; // copy number locally

            m.current_slot = slot_request(shm_ptr->server_flag); // get index of next aval slot
            shm_ptr->number = m.current_slot; // tell client index of next aval slot. (-1 if full)
            // tell client they can replace the number.
            shm_ptr->client_flag = EMPTY;

            // if we arent full, make threads for client request.
            if(m.current_slot >= 0){
                pthread_t tid;
                // create 32 threads to solve simultaneously
                // for each request
                pthread_create(&tid, NULL, solve, (void *) &m);
            } else {
                printf("--- cannot add request: server full ---\n");
            }
        }
    }
}


// returns index of next available slot, or -1 for full.
int slot_request(int server_flag[]){
    for(int i = 0; i < NUM_REQUESTS; i++){
        if(server_flag[i] == EMPTY){
            server_flag[i] = NEW_DATA;
            return i;
        }
    }
    return -1;
}


// creates 32 threads.
// each thread finds all the factors for a different number.
void *solve(void* arg){
    struct Memory *m = (struct Memory *) arg;
    long loc = m->number;

    printf("Started: %ld\n", loc);
    pthread_t tid[NUM_THREADS];
    // num threads is just for 1 request.
    // so overall threads is NUM_THREADS * requests.
    // This just starts 32 threads, 1 for each bit rotated num.
    for(int i = 0; i < NUM_THREADS; i++){
        m->index = i;
        pthread_create(&tid[i], NULL, find_factors, (void *) m);
        delay(25);
    }
    for(int i = 0; i < NUM_THREADS; i++){
        pthread_join(tid[i], NULL);
    }
    printf("Finished: %ld\n", loc);
}


void *find_factors(void *arg){
    struct Memory *m = (struct Memory*) arg;
    int rotations = m->index;
    long num = m->number;
    long original = num;
    // bit rotates num by its index.
    num = bit_rotate_right(num, rotations);

    //printf("Num: %ld\n", num);
    for(long i = 1; i <= num; i++){
        if(num % i == 0){
            printf("OG: %ld Num: %ld -- Factor: %ld\n", original, num, i);
        }
    }
    return (void*) 0;
}


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


void delay(int milli){
    long pause;
    clock_t now,then;

    pause = milli*(CLOCKS_PER_SEC/1000);
    then = clock();
    now = then;
    while( (now-then) < pause )
        now = clock();
}

