#include "header.h"


// NOTE: make sure you compile with -lpthread to allow threads to be used.

//*************************************************************************
// this program queries the user for a number (32-bit integer)
// The program then finds all the factors of that number and 31 other
// numbers derived from bit rotating right.
//*************************************************************************

int main() {
    long num;
    char *user_input = (char *)malloc(sizeof(char) * BUFF_SIZE);
    printf("Either enter a value and number of threads (\"12345678 4\"), or type quit.\n");

    key_t shm_key; // shared memory key
    int shm_id; // shared memory id
    struct Memory *shm_ptr; // pointer to shared memory struct.


    while(1){
        memset(user_input, '\0', sizeof(char) * BUFF_SIZE); // clear user_input
        printf("> "); // user prompt
        fgets(user_input, BUFF_SIZE, stdin); // put user input into user_input
        if(strcmp(user_input, "quit\n") == 0){ // check for quit condition
            free(user_input);
            exit(EXIT_SUCCESS);
        }

        if(format_input(user_input, &num) == 0){
            printf("Value: %ld\n", num);
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
