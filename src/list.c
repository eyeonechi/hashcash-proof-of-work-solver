/*
 * COMP30023 Computer Systems
 * Semester 1 2017
 * Project 2     : list.c
 * Student Name  : Ivan Ken Weng Chee
 * Student ID    : 736901
 * Student Email : ichee@student.unimelb.edu.au 
 */

#include "list.h"

/* Initialises a list node */
List* initList(Elem *data, int socket) {
	List *list = (List *) malloc(sizeof(list));
	assert(list);
	list->data = data;
	list->socket = socket;
	list->next = NULL;
	return list;
}

/* Prepends a new node to the start of the list */
List* prepend(List *node, Elem *data, int socket) {
	List *head = initList(data, socket);
	head->next = node;
	return head;
}

/* clear = true  : Returns data and updates the head of the list *
 * clear = false : Returns data and leaves the head node intact  */
Elem* popHead(List **head, List *next, bool clear) {
	if (*head) {
		if (clear) {
			Elem *data = (*head)->data;
			*head = next;
			return data;
		} else {
			Elem *data = (*(head))->data;
			(*(head))->data = NULL;
			return data;
		}
	}
	return NULL;
}

/* Returns data and removes the tail of the list */
Elem* popTail(List *curr, List *prev) {
	if (curr) {
		if (curr->next) {
			return popTail(curr->next, curr);
		} else {
			Elem *data = curr->data;
			free(curr);
			curr = NULL;
			if (prev) {
				prev->next = NULL;
			}
			return data;
		}
	}
	return NULL;
}

/* Removes data from and leaves the node intact */
Elem* removeData(List *curr, int aim, bool (*eq)(int aim, Elem *data)) {
	if (curr) {
		if (eq(aim, curr->data)) {
			Elem *data = curr->data;
			curr->data = NULL;
			return data;
		}
		return removeData(curr->next, aim, eq);
	}
	return NULL;
}

/* Removes a node from the list */
Elem* removeNode(List *curr, List *prev, int aim, 
	bool (*eq)(int aim, Elem *data)) {
	if (curr) {
		if (eq(aim, curr->data)) {
			if (curr->next) {
				Elem *data = curr->data;
				curr->data = curr->next->data;
				curr->next = curr->next->next;
				return data;
			} else {
				Elem *data = curr->data;
				if (prev) {
					prev->next = NULL;
				} else {
					curr = NULL;
				}
				return data;
			}
		}
		return removeNode(curr->next, curr, aim, eq);
	}
	return NULL;
}

/* Appends a new node to the end of the list */
void append(List *curr, List *prev, Elem *data, int socket) {
	if (curr) {
		append(curr->next, curr, data, socket);
	} else {
		List *new = initList(data, socket);
		if (prev) {
			prev->next = new;
		}
	}
}

/* Inserts a new node at a specified location in the list */
void insert(List *curr, List *prev, Elem *data, int socket,
	bool (*eq)(Elem *data)) {
	if (curr) {
		if (eq && eq(data)) {
			List *next = curr->next;
			List *new = initList(data, socket);
			curr->next = new;
			new->next = next;
		} else {
			insert(curr->next, curr, data, socket, eq);
		}
	} else {
		append(curr, prev, data, socket);
	}
}

/* Traverses a list, applying a function to each node */
void traverseList(List *node, void (*fn)(List*)) {
	if (node) {
		fn(node);
		traverseList(node->next, fn);
	}
}

/* Prints the list */
void displayList(List *list, void (*pr)(List*)) {
	traverseList(list, pr);
}

/* Swaps data of two nodes */
void swapNodes(List *n1, List *n2) {
	Elem *d1 = n1->data;
	n1->data = n2->data;
	n2->data = d1;
}

/* Returns the number of elements in the list */
int getSize(List *node) {
	if (node) {
		return 1 + getSize(node->next);
	}
	return 0;
}
