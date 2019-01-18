#include "mismatch_server.h"

#ifndef PORT
  #define PORT 50540

#endif

int servfd;
int question_count;
int clients;
int question_number;
Client *top=NULL;
QNode *root = NULL;
Node *interests = NULL;


char *question_err = "Invaild question file. \n";
char *user_prompt = "What is your user name?\n";
char *welcome_msg = "Welcome.\nGo ahead and enter user commands>\n";
char *welcome_back_msg = "Welcome back.\n";
char *col_int = "Collecting your interests\n";
char *question_prompt = "Do you like %s? (y/n)\n";
char *invalid_message = "ERROR: Answer must be one of 'y', 'n'. Type quit to exit.\n";
char *test_complete = "Test complete.\n";
char *test_already_done = "You have already done your test.\n";
char *do_test_first = "Type do_test to do test first.\n";
char *neg_result = "No completing personalities found. Please try again later\n";
char *pos_result = "Here are your best mismatches:\n";
char *rec_not_found = "Receiver not online or does not exist.\n";
char *message = "Message from %s: %s";
char *invailid_syn = "Invaild syntax. \n";
char *goodbye_msg = "Thanks for participating! Goodbye and please come back!\n";

char *tokens[3];

void sigFunc(int sig) {
    release_port();
}


int main(int argc, char **argv){
    if(argc!=2){
        fprintf(stderr, "Usage: <interest_file_name>\n");
        exit(1);
    }

    signal(SIGINT, sigFunc);

    // read interests from file
    interests = get_list_from_file (argv[1]);
    if (interests == NULL) {
        error(question_err);
        exit(1);
    }
    // build question tree
    question_count = count_questions(interests);
    root = add_next_level (root,  interests);
    bindandlisten();  /* aborts on error */

    fd_set fdlist;
    int maxfd = servfd;
    Client *p;
    int prev_user_path;
    int read_bytes=0;
    int where;
    int cmd_argc;
    int process_result;
    int val_result;
    int vaild_len;
    int x;

    /* the only way the server exits is by being killed */
    while (1) {
        maxfd = servfd;
        FD_ZERO(&fdlist);
        FD_SET(servfd, &fdlist);
        for (p = top; p; p = p->next) {
            FD_SET(p->fd, &fdlist);
            if (p->fd > maxfd)
                maxfd = p->fd;
        }
        if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
            perror("select");
        } else {
            for (p = top; p; p = p->next){
                if (FD_ISSET(p->fd, &fdlist)){
                    // put processing here?
                    break;
                }
            }
            if (p){
                read_bytes = read(p->fd, (p->buf)+(p->inbuf), BUFFER_SIZE-(p->inbuf));
                if (read_bytes == 0) { // client exited
                    remove_client(p->fd);
                } else if (read_bytes > 0) {
//
                    //printf("Line 87: %s, %d\n", p->buf, read_bytes);
                    p->inbuf += read_bytes;
                    //printf("Line 89: %d\n", p->inbuf);
                    where=find_network_newline(p->buf, p->inbuf);
                    //printf("Line 91: %d\n", where);
                    //fflush(stdout);
                    if (where >= 0){
                        (p->buf)[where] = '\n';
                        (p->buf)[where+1] = '\0';
                        vaild_len = strlen(p->buf);
                        p->inbuf -= 1;
                        //process_args()
                        if (p->state == NO_NAME){
                            (p->buf)[where] = '\0';
                            (p->buf)[127]='\0';

                            prev_user_path = user_search_path(root, p->buf);
                            // printf("%d\n", prev_user_path);
                            p->state = ANSWERED;
                            strncpy(p->name, p->buf, 127);
                            (p->name)[127] = '\0';
                            if (prev_user_path != -1){
                                for (x=0; x<question_count; x++) {
                                    (p->answers)[x] = prev_user_path % 2;
                                    prev_user_path = prev_user_path/2;
                                }
/*                                int m;
                                for (m=0; m<question_count; m++){
                                    printf("%d", (p->answers)[m]);
                                }
                                printf("\n");
                                fflush(stdout);*/

                                if(write(p->fd, welcome_back_msg, strlen(welcome_back_msg)) == -1) {
                                    perror("write");
                                }
                                //restore
                            } else {
                                if(write(p->fd, welcome_msg, strlen(welcome_msg)) == -1) {
                                    perror("write");
                                }
                                p->state = PENDING;
                                //printf("Client name: %s\n", p->name);
                            }
                        } else if (p->state > 0) { //answering
                            // printf("test answer: %s", p->buf);
                            p->buf[where] = '\0';
                            // do test 
                            val_result = validate_answer(p->buf);
                            // printf("validate_ans: %d\n", val_result);
                            if (val_result == -2) {
                                if(write(p->fd, invalid_message, strlen(invalid_message)) == -1){
                                    perror("write");
                                }
                                ask_question(p->fd, p->state);
                            } else if (val_result == -1) {
                                remove_client(p->fd);
                            } else {
                                (p->answers)[question_count-(p->state)]=val_result;
                                p->state --;
                                if (p->state != 0){
                                    ask_question(p->fd, p->state);
                                } else if (p->state == 0){
                                    printf("recorded activity for %s.\n", p->name);
                                    fflush(stdout);
                                    if (write(p->fd, test_complete, strlen(test_complete)) == -1){
                                        perror("write");
                                    }
                                    add_user(root, p->answers, p->name, question_count);
/*                                    int m;
                                    for (m=0; m<question_count; m++){
                                        printf("%d", (p->answers)[m]);
                                    }
                                    printf("\n");
                                    fflush(stdout);*/
                                }
                            }
                        } else {
                            //printf("Else: %s", p->buf);
                            //printf("Line 135: %d\n", strlen(p->buf));

                            cmd_argc = tokenize(p->buf, tokens);
                            //printf("Line 137: %d\n", strlen(p->buf));
                            process_result = process_args(cmd_argc, tokens, p);
                            //printf("Line 139: %d\n", strlen(p->buf));

                            if (process_result == -1) {
                                remove_client(p->fd);
                            }
                            //printf("Line 144: %d\n", strlen(p->buf));
                        }
                        //process end
                        //printf("Line 143: inbuf:%d, strlen:%d\n", p->inbuf, strlen(p->buf));
                        (p->inbuf) -= vaild_len;
                        //printf("Line 145: %d\n------\n", p->inbuf);
                        if ((p->inbuf) > 0) {
                            memmove(p->buf, (p->buf)+vaild_len+1, p->inbuf);
                        }
                    }
//
                } else {
                    perror("read");
                }
            }
            if (FD_ISSET(servfd, &fdlist)) { /*new client connects*/
                newconnection();
            }
        }
    }
    return(0);
}

void bindandlisten() {  /* bind and listen, abort on error */

    struct sockaddr_in serv_addr;

    if ((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&serv_addr, '\0', sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(servfd, (struct sockaddr *)&serv_addr, sizeof serv_addr) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(servfd, 5)) { // -1 is evaluated as true
        perror("listen");
        exit(1);
    }
    printf("Listening on %d\n", ntohs(serv_addr.sin_port));

    fflush(stdout);
}

void newconnection() { /* accept connection, sing to them, get response, update
                        * linked list */

    int clientfd;
    struct sockaddr_in client_addr;
    socklen_t socklen = sizeof client_addr;

    if ((clientfd = accept(servfd, (struct sockaddr *)&client_addr, &socklen)) < 0) {
        perror("accept");
        return;
    }
    char *addr=inet_ntoa(client_addr.sin_addr);
    printf("connection from %s\n", addr);
    fflush(stdout);

    add_client(clientfd, addr);
    if (write(clientfd, user_prompt, strlen(user_prompt)) == -1){
        perror("write");
    }
}

void add_client(int fd, const char* addr) {

    printf("Adding client %s\n", addr);
    Client *p = malloc(sizeof(Client));
    fflush(stdout);
    p->fd = fd;
    p->inbuf = 0;
    p->next = top;
    p->state= NO_NAME;
    p->answers = malloc(question_count*sizeof(int));
    int c;
    for (c=0; c<question_count; c++){
        (p->answers)[c]=0;
    }

    top=p;
    clients++;

    //printf("TOP\n");
    //print_clients(top);

}

void remove_client(int fd){
    Client **p;
    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next)
         ;
    if (*p) {
        Client *t = (*p)->next;
        printf("Removing client %s\n", (*p)->name);
        if (write(fd, goodbye_msg, strlen(goodbye_msg)) == -1){
            perror("write");
        }
        fflush(stdout);
        if (close(fd) == -1){
            perror("close");
        }
        free((*p)->answers);
        free(*p);
        *p = t;
        clients--;
    } else {
        fprintf(stderr, "intend not to remove fd %s\n", (*p)->name);
        fflush(stderr);
    }
    //printf("TOP\n");
    //print_clients(top);
}

int find_network_newline (char *buf, int inbuf) { 
    int i;
    for (i = 0; i < inbuf - 1; i++){
        if ((buf[i] == '\r') && (buf[i + 1] == '\n')){
            return i;
        }
    }
    return -1; 
}

void release_port() {
    //printf("\nReleasing port.\n");
    free_list(interests);
    free_tree(root);
    Client * curr=top;
    while(curr){
        if (close(curr->fd) == -1){
            perror("close");
        }
        free((curr)->answers);
        curr=curr->next;         
        free(curr);
    }

    int on = 1;
    int status = setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR,(const char *) &on, sizeof(on));
    if(status == -1) {
        perror("setsockopt -- REUSEADDR");
    }
    //printf("Port released.\n");
    exit(0);
}

int validate_answer(char *answer){
    if (strlen(answer) > 3){
        if (strcmp(answer, "quit") == 0) {
            return -1;
        }
        return -2;
    }
    if (answer[0] == 'n' || answer[0] == 'N')
        return 0;
    if (answer[0] == 'y' || answer[0] == 'Y')
        return 1;
    return -2;
}

void error(char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    fflush(stderr);
}

/*
 * Read and process commands
 */
int process_args(int cmd_argc, char **cmd_argv, Client *current_client) {
        //printf("%s, %s, %s. %d\n", cmd_argv[0],cmd_argv[1],cmd_argv[2], cmd_argc);
    if (cmd_argc <= 0) {
        return 0;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        /* Return an appropriate value to denote that the specified
         * user is now need to be disconnected. */
        return -1;
    } else if (strcmp(cmd_argv[0], "do_test") == 0 && cmd_argc == 1 ) {
        /* The specified user is ready to start answering questions. You
         * need to make sure that the user answers each question only
         * once.
         */
        if (current_client->state == PENDING){
            current_client->state = question_count;
            if (write(current_client->fd, col_int, strlen(col_int)) == -1) {
                perror("write");
            }
            ask_question(current_client->fd, current_client->state);
        } else if (current_client->state == ANSWERED) {
            if (write(current_client->fd, test_already_done, strlen(test_already_done)) == -1){
                perror("write");
            }
        }
    } else if (strcmp(cmd_argv[0], "get_all") == 0 && cmd_argc == 1) {
        /* Send the list of best mismatches related to the specified
         * user. If the user has not taked the test yet, return the
         * corresponding error value (different than 0 and -1).
         */
        if (current_client->state == PENDING){
            if (write(current_client->fd, do_test_first, strlen(do_test_first)) == -1){
                perror("write");
            }
            // do test first
        } else {
            Node *friends=reverse_user_search(root,current_client->answers, question_count);
            if(friends){
                if (write(current_client->fd,pos_result,strlen(pos_result)) == -1){
                    perror("write");
                }
                Node *curr=friends;
                while(curr!=NULL){
                    if (curr->str != NULL){
                    if (write(current_client->fd, curr->str, strlen(curr->str)) == -1){
                        perror("write");
                    }
                    if (write(current_client->fd, "\n", 1) == -1){
                        perror("write");
                    }
                    curr=curr->next;
                    }
                }
            } else {
                if (write(current_client->fd,neg_result,strlen(neg_result)) == -1){
                    perror("write");
                }
            }
        }
    } else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc == 3) {
        /* Send the specified message stored in cmd_argv[2] to the user
         * stored in cmd_argv[1].
         */
        if (current_client->state == PENDING){
            if (write(current_client->fd,pos_result,strlen(pos_result)) == -1){
                perror("write");
            }
        } else {
            char* receiver = cmd_argv[1];
            Client* rec_client = find_client_by_name(receiver, top);
            if (rec_client){
                char * msg_to_send = malloc(2048);
                sprintf(msg_to_send, message, current_client->name, cmd_argv[2]);
                if (write(rec_client->fd, msg_to_send, strlen(msg_to_send)) == -1){
                    perror("write");
                }
                free (msg_to_send);
            } else {
                if (write(current_client->fd, rec_not_found, strlen(rec_not_found)) == -1){
                    perror("write");
                }
            }
        }
    } else {
        /* The input message is not properly formatted. */
        error("Incorrect syntax");
        if (write(current_client->fd, invailid_syn, strlen(invailid_syn)) == -1){
            perror("write");
        }
        //if (write(current_client->fd, ))
    }
    return 0;
}

/*
 * Tokenize the command stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv) {
    memset(cmd_argv, 0, 3*sizeof(char*));
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);

    while (next_token != NULL) {
        cmd_argv[cmd_argc] = next_token;
        ++cmd_argc;

    if(cmd_argc < (INPUT_ARG_MAX_NUM - 1))
        next_token = strtok(NULL, DELIM);
    else
        break;
    }

    if (cmd_argc == (INPUT_ARG_MAX_NUM - 1)) {
    cmd_argv[cmd_argc] = strtok(NULL, "");
    if(cmd_argv[cmd_argc] != NULL)
        ++cmd_argc;
    }

    return cmd_argc;
}

int count_questions(Node *questions) {
    Node *q = questions;
    int count = 0;
    while (q != NULL){
        count ++;
        q = q->next;
    }
    return count;
}

void ask_question(int fd, int question_num) {
    //printf("asking question%d.\n", question_count - question_num +1);
    fflush(stdout);
    Node *curr_question = interests;
    if (question_num > question_count || question_count <= 0) {
        error("Invaild question_num");
    }
    int k=question_count;
    for (k=question_count; k>question_num; k--){
        curr_question = curr_question->next;
    }
    int length = strlen(question_prompt) + strlen (curr_question->str);
    char *this_question = malloc(length + 50);
    int wc=sprintf(this_question, question_prompt, curr_question->str);
    if (wc > 0){
        int written =write(fd, this_question, wc);
        if (written != wc){
            perror("write");
        }
    } else {
        error("Did not get the question.");
    }
    free(this_question);
}

Client* find_client_by_name(char * name, Client* which_top){
    Client **p;
    for (p = &which_top; *p && strcmp((*p)->name, name) != 0; p = &(*p)->next)
         ;
    return *p;
}

void print_clients(Client * c){
    c=top;
    while (c){
        printf("%s, %d\n", c->name, c->fd);
        c = c->next;
    }
}