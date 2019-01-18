#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "questions.h"

Node * get_list_from_file (char *input_file_name) {
	FILE *fp;
	fp=fopen(input_file_name,"r");
	Node * head = NULL;
    Node * tail = NULL;
	char line[MAX_LINE] = "";
	while (fgets (line, MAX_LINE, fp)!=NULL) {
        line [strcspn (line, "\r\n")] = '\0';
        Node * new_node = (Node*) malloc(sizeof(Node));
        new_node->str = (char*) malloc(MAX_LINE);
        strcpy(new_node->str,line); 
        new_node->next = NULL; //new node has str and next = NULL.
        if(head == NULL){
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = tail->next;
        }
    }
    fclose(fp);
	return head;
}

void print_list (Node *head) {
	Node *curr=head;
    while(curr!=NULL){
        if (curr->str != NULL){
            printf("%s\n",curr->str);
            curr=curr->next;
        }
    }
}

void free_list (Node *head) {
	Node *temp;

    while (head !=NULL) {
       free(head->str);
       temp = head;
       head = head->next;
       free(temp);
    }
}
