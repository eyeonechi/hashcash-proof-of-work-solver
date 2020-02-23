/*
 * COMP30023 Computer Systems
 * Semester 1 2017
 * Project 2     : list.h
 * Student Name  : Ivan Ken Weng Chee
 * Student ID    : 736901
 * Student Email : ichee@student.unimelb.edu.au 
 */

#ifndef LIST_H
#define LIST_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define NEWLINE "\n"

typedef void Elem;
typedef struct list_t List;

struct list_t {
	int socket;
	bool pending;
	Elem *data; /* Data reference   */
	List *next; /* Next list node   */
};

/* Initialises a list node */
List* initList(
	Elem *data,
	int socket
);

/* Prepends a new node to the start of the list */
List* prepend(
	List *node,
	Elem *data,
	int socket
);

/* clear = true  : Returns data and updates the head of the list *
 * clear = false : Returns data and leaves the head node intact  */
Elem* popHead(
	List **head,
	List *next,
	bool clear
);

/* Returns data and removes the tail of the list */
Elem* popTail(
	List *curr,
	List *prev
);

/* Removes data from and leaves the node intact */
Elem* removeData(
	List *curr,
	int aim,
	bool (*eq)(int aim, Elem *data)
);

/* Removes a node from the list */
Elem* removeNode(
	List *curr,
	List *prev,
	int aim,
	bool (*eq)(int aim, Elem *data)
);

/* Appends a new node to the end of the list */
void append(
	List *node,
	List *prev,
	Elem *data,
	int socket
);

/* Inserts a new node at a specified location in the list */
void insert(
	List *curr,
	List *prev,
	Elem *data,
	int socket,
	bool (*eq)(Elem *data)
);

/* Traverses a list, applying a function to each node */
void traverseList(
	List *node,
	void (*fn)(List*)
);

/* Prints the list */
void displayList(
	List *list,
	void (*pr)(List*)
);

/* Swaps data of two nodes */
void swapNodes(
	List *n1,
	List *n2
);

/* Returns the number of elements in the list */
int getSize(
	List *node
);

#endif
