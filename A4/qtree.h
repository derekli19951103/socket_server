#ifndef QTREE_H
#define QTREE_H
#include "questions.h"

typedef enum {
    REGULAR, LEAF
} NodeType;

union Child {
	struct str_node *fchild;
	struct QNode *qchild;
} Child;

typedef struct QNode {
	char *question;
	NodeType node_type;
	union Child children[2];
} QNode;

QNode *add_next_level (QNode *current, Node * list_node);

void print_qtree (QNode *parent, int level);
void print_users (Node *parent);

void free_tree (QNode *tree);

Node *user_search(QNode *tree, char* username);
int list_search(Node* list, char* username);
Node *reverse_user_search(QNode *tree,int *answers, int q_size);
int add_user(QNode *tree, int* ans, char* username, int q_size);
QNode *add_user_per_step(QNode *tree, int ans, char* username);

int user_search_path(QNode *tree, char* username);
#endif
