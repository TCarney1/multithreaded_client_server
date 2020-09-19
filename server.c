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
            long num = shm_ptr->number;
            pthread_t tid[NUM_THREADS];
            long temp = num;
            // num threads is just for 1 request.
            // so overall threads is NUM_THREADS * requests.
            // This just starts 32 threads, 1 for each bit rotated num.
            for(int i = 0; i < NUM_THREADS; i++){
                printf("Num Rotation: %d Value: %ld\n", i, temp);
                pthread_create(&tid[i], NULL, find_factors, (void *) &temp);
                sleep(1);
                temp = bit_rotate_right(temp);
            }
            for(int i = 0; i < NUM_THREADS; i++){
                pthread_join(tid[i], NULL);
            }
        }
        // tell client they can replace the number.
        shm_ptr->client_flag = EMPTY;
    }
}


long bit_rotate_right(long num){
    int dropped = num & 1; // bitwise AND
    int bits = sizeof(int) * 8 - 1;
    num = (num >> 1) & (~(1 << bits)); // ~ = bitwise compliment
    num = num | (dropped << bits);
    return num;
}


void *find_factors(void *arg){
    long *num = (long *) arg;
    printf("Num: %ld\n", *num);

    for(long i = 1; i <= *num; i++){
        if(*num % i == 0){
            //printf("Num: %ld -- Factor: %ld\n", *num, i);
        }
    }

    return (void*) 0;
}
