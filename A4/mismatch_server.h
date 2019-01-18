#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include "qtree.h"

#define NO_NAME -2
#define PENDING -1
#define ANSWERED 0

#define MAX_NAME 128
#define BUFFER_SIZE 1160
#define INPUT_ARG_MAX_NUM 3
#define DELIM " \r\n"


void bindandlisten();
void newconnection();
void add_client(int fd, const char* addr);
void remove_client(int fd);
int find_network_newline (char *buf, int inbuf);
void release_port();
/*
 * Τhe definition of the following structure is copied directly from the
 * muffinman server (http://www.cdf.toronto.edu/~ajr/209/a4/muffinman.c).
 * You might need to add more fields - such as a seoarate buffer for each client and 
 * the current position in the buffer - as explained in A 4 handout.
 */

typedef struct client {
    int fd; //file descriptor to write into and to read from int *answers;
    //before user entered a name, he cannot issue commands
    int *answers;
    int state;
    char name [MAX_NAME];
    char buf [BUFFER_SIZE]; // each client has its own buffer int inbuf; // and a pointer to the current end-of-buf position struct 
    int inbuf;
    struct client *next;
} Client;


/* 
 * Print a formatted error message to stderr.
 */
void error(char *);

/* 
 * Read and process commands
 */
int process_args(int cmd_argc, char **cmd_argv, Client *current_client);

/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int validate_answer(char *answer);
int tokenize(char *, char **);
int count_questions(Node *);
void ask_question(int fd, int question_num);
Client* find_client_by_name(char * name, Client *which_top);
void add_back_client(int fd);
void print_clients(Client * c);