#include <ctype.h>
#include "qtree.h"

void print_greetings();
int validate_user(char *name);
int validate_answer(char *answer);
void wrap_up();
void print_friends(Node *list, char *name);

QNode *root = NULL;    
Node *interests = NULL;

char *user_prompt = "Please enter your name. Type 'q' to quit\n";
char *question_prompt = "Do you like %s? (y/n)\n";
char *neg_result = "Sorry, no users with similar interests joined yet\n\n";
char *pos_result1 = "friend recommendation for user %s:\n";
char *pos_result2 = "You have total %d potential friend(s)!!!\n\n";

int main (int argc, char **argv) {
    
    char answer[MAX_LINE];    
    char name[MAX_LINE];

    if (argc < 2) {
        printf ("To run the program ./categorizer <name of input file>\n");
        return 1;
    }
    
    // read interests from file
    interests = get_list_from_file (argv[1]);
    
    if (interests == NULL)
        return 1;

    // build question tree
    root = add_next_level (root,  interests);

    //main application loop
    for (; ;){
        print_greetings();

        // read user name    
        printf("%s", user_prompt);
        scanf("%s", name);
    
        // user entered "q" instead of a user name
        if (strcmp(name, "q") == 0 || strcmp(name, "Q") == 0) 
            wrap_up();
        
        // if user name is not valid, continue to loop
        if (!validate_user(name))
            continue;
        
        // check whether the user with this name already exists
        Node *user_list = find_user(root, name);
        
        if (!user_list){
            // find the list of friends to which the user should be attached
            QNode *prev, *curr;
            prev = curr = root;
        
            // iterate over list of interests
            Node *i = interests;
            int ans;
            while (i){
                printf(question_prompt, i->str);
                
                // read answer to prompt
                scanf("%s", answer);
                ans = validate_answer(answer);
                
                // if answer if not valid, continue to loop
                if (ans == 2)
                    continue;
                    
                prev = curr;
                curr = find_branch(curr, ans);

                i = i->next;
            }
            
            // add user to the end of the list
            user_list = prev->children[ans].fchild;
            prev->children[ans].fchild = add_user(user_list, name);
        }
        
        // print list of potential friends
        print_friends(user_list, name);  
    }
    
    return 0;
}

void print_greetings () {
    printf ("----------------------------------------------\n");
    printf ("Friend recommender system. Find people who are just like you!\n");
    printf ("CSC209 fall 2016 team. All rights reserved\n");
    printf ("----------------------------------------------\n");
}

void wrap_up(){
    //end of main loop - the user typed "q"
    print_qtree (root, 0);
    
    free_list (interests);
    free_qtree(root);
    
    exit(0);
}

int validate_user(char *name){
    int valid = 1;
    
    int len = strlen(name);
    if (len < 8 || len > 128)
        valid = 0;
    
    for (int i = 0; i < len; i++){
        if (!isalnum(name[i])){
            valid = 0;
            break;
        }
    }

    if (!valid)
        fprintf(stderr, "ERROR: User name must be at least 8 and at most "
            "128 characters, and can only contain alphanumeric "
            "characters.\n\n");

    return valid;
}

int validate_answer(char *answer){
    char *invalid_message = "ERROR: Answer must be one of 'y', 'n', 'q'.\n";
    
    if (strlen(answer) > 3){
        printf("%s", invalid_message);
        return 2;
    }
    
    if (answer[0] == 'q' || answer[0] == 'Q')
        wrap_up();
        
    if (answer[0] == 'n' || answer[0] == 'N')
        return 0;
        
    if (answer[0] == 'y' || answer[0] == 'Y')
        return 1;
        
    printf("%s", invalid_message);
    return 2;
}

// print list of potential friends for user
void print_friends(Node *list, char *name){
    int friends = 0;

    // iterate over user list and count the number of friends
    while (list) {
	// ignore this user
        if (strcmp(list->str, name)) {
            friends++;
             
	    // if this is the first friend found, print successful message    
            if (friends == 1)
                printf(pos_result1, name);
            
	    // if friend was found, print his/her name
            printf("%s, ", list->str);
        }
            
        list = list->next;
    }
    
    // if friends were found, print the number of friends    
    if (friends){
        printf("\n");
        printf(pos_result2, friends);
        
    } else {
        printf("%s", neg_result);    
    }
}
