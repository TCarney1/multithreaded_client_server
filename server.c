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
        sleep(1);
        //printf("Outside loop.\n");
        // what for client to send data.
        while (shm_ptr->client_flag == EMPTY){
            //printf("--- waiting... ---\n");
            sleep(1);
        }
        //printf("looping.\n");
        if (shm_ptr->client_flag == QUIT) {
            shmdt((void *) shm_ptr);
            printf("--- memory detached ---\n");
            printf("--- quiting... ---\n");
            exit(EXIT_SUCCESS);
        }
        if(shm_ptr->client_flag == NEW_DATA) {
            printf("Received: %ld\n", shm_ptr->number);
            shm_ptr->client_flag = EMPTY;
        }
    }
}
