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
    long num;
    char *user_input = (char *)malloc(sizeof(char) * BUFF_SIZE);
    printf("Either enter a value and number of threads (\"12345678 4\"), or type quit.\n");

    /// stuff for shared memory ///
    key_t shm_key; // shared memory key
    int shm_id; // shared memory id
    struct Memory *shm_ptr; // pointer to shared memory struct.
    shm_key = ftok(".", 'x');
    shm_id = shmget(shm_key, sizeof(struct Memory), IPC_CREAT | 0666);
    if(shm_id < 0){
        printf("Error with shared memory ID.\n");
        exit(EXIT_FAILURE);
    }

    printf("--- shared memory allocated ---\n");

    shm_ptr = (struct Memory *) shmat(shm_id, NULL, 0);
    if((int) shm_ptr == -1){
        printf("Error with shmat.\n");
        exit(EXIT_FAILURE);
    }

    printf("--- memory has been attached ---\n");

    // init shared memory flags
    shm_ptr->client_flag = EMPTY;
    for(int i = 0; i < NUM_REQUESTS; i++){
        shm_ptr->server_flag[i] = EMPTY;
    }
    // so client can keep track of where requests have been slotted.
    long local_slots[NUM_REQUESTS];

    while(1){
        memset(user_input, '\0', sizeof(char) * BUFF_SIZE); // clear user_input
        printf("> "); // user prompt
        fgets(user_input, BUFF_SIZE, stdin); // put user input into user_input
        if(strcmp(user_input, "quit\n") == 0){ // check for quit condition
            free(user_input);
            shm_ptr->client_flag = QUIT;
            shmdt((void *) shm_ptr);
            printf("--- memory detached ---\n");
            shmctl(shm_id, IPC_RMID, NULL);
            printf("--- memory removed ---\n");
            printf("--- quiting... ---\n");
            exit(EXIT_SUCCESS);
        }

        if(format_input(user_input, &num) == 0){
            printf("--- requesting from server ---\n");

            // ensure server has taken data.
            // (shouldn't really even enter this loop.)
            if(shm_ptr->client_flag != EMPTY){
                printf("--- waiting for server ---\n");
            }
            while(shm_ptr->client_flag != EMPTY)
                sleep(1);

            // upload num to shared memory
            // and change flag to tell server
            shm_ptr->number = num;
            shm_ptr->client_flag = NEW_DATA;
            printf("--- request sent ---\n");

            // wait for server to acknowledge
            while(shm_ptr->client_flag != EMPTY)
                sleep(1);

            // server reply's with index of slot the slot
            // that num has been put in.
            // (server places it in shm_ptr->number).
            if(shm_ptr->number >= 0){
                local_slots[shm_ptr->number] = num;
                printf("--- request successful: Num: %ld Slot: %ld ---\n", num, shm_ptr->number);
            } else {
                printf("--- request denied: server full ---\n");
            }
        } else {
            printf("Incorrect arguments entered.\n");
        }
    }
}


// returns 1 if incorrect args are given. returns 0 if all is well.
// splits the input string into long value and long n_threads.
int format_input(char *user_input, long *num_p){
    char *token;

    //First arg is value.
    token = strtok(user_input, " ");
    if(token == NULL) return 1; // error, no args
    *num_p = strtol(token, NULL, 10);

    token = strtok(NULL, " ");

    // too many args given or arg was not a number.
    if(token != NULL || *num_p == 0) return 1;

    return 0;
}
