#include "qtree.h"
#include <string.h>

QNode *add_next_level (QNode *current, Node *list_node) {
	int str_len;
	
	str_len = strlen (list_node->str);
	current = (QNode *) calloc (1, sizeof(QNode));

	current->question =  (char *) calloc (str_len +1, sizeof(char));
	strncpy ( current->question, list_node->str, str_len );
	current->question [str_len] = '\0';  
	current->node_type = REGULAR;
	
	if (list_node->next == NULL) {
		current->node_type = LEAF;
		return current;
	}
	
	current->children[0].qchild = add_next_level ( current->children[0].qchild, list_node->next);
	current->children[1].qchild = add_next_level ( current->children[1].qchild, list_node->next);

	return current;
}

void print_qtree (QNode *parent, int level) {
	int i;
	for (i=0; i<level; i++)
		printf("\t");
	
	printf ("%s type:%d\n", parent->question, parent->node_type);
	if(parent->node_type == REGULAR) {
		print_qtree (parent->children[0].qchild, level+1);
		print_qtree (parent->children[1].qchild, level+1);
	}
	else { //leaf node
		for (i=0; i<(level+1); i++)
			printf("\t");
		print_users (parent->children[0].fchild);
		for (i=0; i<(level+1); i++)
			printf("\t");
		print_users (parent->children[1].fchild);
	}
}

void print_users (Node *parent) {
	if (parent == NULL)
		printf("NULL\n");
	else {
        if(parent->str != NULL){
            printf("%s, ", parent->str);
        }
		while (parent->next != NULL) {
			parent = parent->next;
            if (parent->str != NULL){
    			printf("%s, ", parent->str);
            }
        }
		printf ("\n");
	}
}

void free_tree (QNode *tree){
    if(tree!=NULL){
        free(tree->question);
        if(tree->node_type==LEAF){
            free_list(tree->children[0].fchild);
            free_list(tree->children[1].fchild);
        }else if(tree->node_type==REGULAR){
            free_tree(tree->children[0].qchild);
            free_tree(tree->children[1].qchild);
        }
        free(tree);
    }else{
        return;
    }
}

Node *user_search(QNode *tree, char* username){
    QNode *curr=tree;
    Node* result;
    if(curr->node_type==LEAF){
        if(list_search(curr->children[0].fchild,username)==1){
            return curr->children[0].fchild;
        }
        if(list_search(curr->children[1].fchild,username)==1){
            return curr->children[1].fchild;
        }
    }else{
        if(curr!=NULL){
            result = user_search(curr->children[0].qchild, username);
            if (result != NULL){
                return result;
            }
            result= user_search(curr->children[1].qchild, username);
            if (result != NULL){
                return result;
            }
        }
    }
    return NULL;
}

int user_search_path(QNode *tree, char* username){
    QNode *curr=tree;
    int result;
    if(curr->node_type==LEAF){
        if(list_search(curr->children[0].fchild,username)==1){
            return 0;
        }
        if(list_search(curr->children[1].fchild,username)==1){
            return 1;
        }
    }else{
        if(curr!=NULL){
            result = user_search_path(curr->children[0].qchild, username);
            if (result != -1){
                return 2*result;
            }
            result= user_search_path(curr->children[1].qchild, username);
            if (result != -1){
                return 2*result+1;
            }
        }
    }
    return -1;
}

int list_search(Node* list, char* username){
    Node *curr=list;
    while(curr!=NULL){
        if(strcmp(curr->str,username)!=0){
            curr=curr->next;
        }else{
            return 1;
        }
    }
    return 0;
}

Node *reverse_user_search(QNode *tree,int *answers, int q_size) {
    QNode *curr=tree;
    int i=0;
    for(i=0;i<q_size;i++){
        if(curr->node_type==LEAF){
            return curr->children[1 - answers[i]].fchild;
        }
        else{
            if(curr){
                curr=curr->children[1 - answers[i]].qchild;
            }
        }
    }
    return NULL;
}
int add_user(QNode *tree,int *answers, char* username, int q_size) {
    QNode *temp=tree;
    int i=0;
    for(i=0;i<q_size;i++){
        temp=add_user_per_step(temp,answers[i],username);
    }
    if(temp==NULL){
        return 0;
    }
    return 1;
}

QNode *add_user_per_step(QNode *tree, int ans, char* username){ // ans 0 = no; ans 1 =yes
    QNode *curr=tree;
    if(curr->node_type==LEAF){ // already reaches leaf -> add user to the linkedlist
        Node *group=curr->children[ans].fchild;
        Node * new_node = (Node*) malloc(sizeof(Node));
        new_node->str = (char *) malloc(strlen(username)+1);
        strcpy(new_node->str,username);
        new_node->next=NULL;
        if (group != NULL){
            while(group->next != NULL){
                group=group->next;
            }
            group->next=new_node;
        } else {
            curr->children[ans].fchild = new_node;
        }
        return NULL;
    }else{
        QNode *result=curr->children[ans].qchild; // internal node -> traverse
        return result;
    }
}