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
        shm_ptr->server_flag[i] = CLOSE;
    }
    pthread_t tid[NUM_REQUESTS];
    pthread_t bar_t;
    while(1){
        memset(user_input, '\0', sizeof(char) * BUFF_SIZE); // clear user_input
        printf("> "); // user prompt
        fgets(user_input, BUFF_SIZE, stdin); // put user input into user_input
        if(strcmp(user_input, "quit\n") == 0){ // check for quit condition
            free(user_input);
            shm_ptr->client_flag = CLOSE;
            shmdt((void *) shm_ptr);
            printf("--- memory detached ---\n");
            shmctl(shm_id, IPC_RMID, NULL);
            printf("--- memory removed ---\n");
            printf("--- quiting... ---\n");
            exit(EXIT_SUCCESS);
        }

        if(format_input(user_input, &num) == 0){
            //printf("--- requesting from server ---\n");

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
            //printf("--- request sent ---\n");

            // wait for server to acknowledge
            while(shm_ptr->client_flag != EMPTY)
                sleep(1);

            // server reply's with index of slot the slot
            // that num has been put in.
            // (server places it in shm_ptr->number).
            if(shm_ptr->number >= 0){ // -1 is returned if full)
                // create a thread for listening for factors.
                pthread_create(&tid[shm_ptr->number], NULL, listen, (void *) shm_ptr);
                //printf("--- request successful: Num: %ld Slot: %ld ---\n", num, shm_ptr->number);
                pthread_create(&bar_t, NULL, loading_bar, (void *) shm_ptr);
            } else {
                printf("--- request denied: server full ---\n");
            }
        } else {
            printf("Incorrect arguments entered.\n");
        }
    }
}


void *listen(void *arg){
    struct Memory* m = (struct Memory*) arg;
    long slot_num = m->number;
    long original_number = m->original_num[slot_num];
    int length = get_length(original_number);
    struct Node *head = NULL;
    head = (struct Node*)malloc(sizeof(struct Node));
    struct timespec start, end;
    long time_taken;

    clock_gettime( CLOCK_MONOTONIC, &start);

    while(m->server_flag[slot_num] != CLOSE){
        // wait for server to give us a factor.
        while(m->server_flag[slot_num] == EMPTY)
            ;
        // server has given us a factor
        if(m->server_flag[slot_num] == NEW_DATA){
            // add factors to list of factors.
            if(head != NULL){
                push_front(&head, m->slot[slot_num]);
            } else {
                head->factor = m->slot[slot_num];
            }
            m->server_flag[slot_num] = EMPTY;
        }
    }
    // stop timing server
    clock_gettime(CLOCK_MONOTONIC, &end);
    // print output
    print_list(head);
    delete(head);

    // calc time taken
    time_taken = (end.tv_sec - start.tv_sec);
    time_taken += (end.tv_nsec - start.tv_nsec) / 1000000000;
    printf("Time elapsed for number \"%ld\": %ld seconds.\n", original_number, time_taken);
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


void print_list(struct Node* node){
    printf("--- Displaying Factors ---\n\n");
    while (node->next != NULL) {
        printf("%ld ", node->factor);
        node = node->next;
    }
    printf("\n\n");
}


void push_front(struct Node **head, long factor){
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    node->factor = factor;
    node->next = *head;
    *head = node;
}

void delete(struct Node *head){
    struct Node* temp;

    while(head != NULL){
        temp = head;
        head = head->next;
        free(temp);
    }
}


void display_bar(long num, long percentage_complete){
    long i;
    printf("%ld : %ld%% |", num,percentage_complete * 10);
    for(i = 0; i < percentage_complete; i++)
        printf("#");

    for(i = 0; i < SIZE-percentage_complete; i++)
        printf("_");
    printf("| ");
    fflush(stdout);

}


void delete_bar(int length){
    // plus 2 for the | at the start and the end.
    // length is the number of digits the numbers have.
    for (long i = 0; i < (SIZE + length + 32); i++)
        printf("\b");
}

void *loading_bar(void *arg){
    struct Memory* m = (struct Memory *) arg;
    // find percentage complete of current slot.
    while(m->client_flag != CLOSE){
        // find all the slots being used, and print a loading bar for them.
        for(int i = 0; i < NUM_REQUESTS; i++){
            if(m->server_flag[i] != CLOSE){
                long p_complete = (m->threads_finished[i] * SIZE) / NUM_THREADS;
                int length = get_length(m->original_num[i]) + get_length(p_complete * 10);
                delete_bar(length);
            }
        }
        for(int i = 0; i < NUM_REQUESTS; i++){
            if(m->server_flag[i] != CLOSE){
                // find percentage complete of current slot.
                long p_complete = (m->threads_finished[i] * SIZE) / NUM_THREADS;
                display_bar(m->original_num[i], p_complete);
            }
        }
        delay(500);
    }
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

int get_length(long num){
    int length = 0;

    do{
        length++;
        num/=10;
    }while(num > 0);

    return length;
}

int kbhit(void){
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}
