/*
 * COMP30023 Computer Systems
 * Semester 1 2017
 * Project 2     : server.h
 * Student Name  : Ivan Ken Weng Chee
 * Student ID    : 736901
 * Student Email : ichee@student.unimelb.edu.au 
 */
 
#ifndef SERVER_H
#define SERVER_H

// Libraries
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <unistd.h>

// Dependencies
#include "uint256.h"
#include "sha256.h"
#include "list.h"

// Constants
#define LOGFILE "log.txt"

// Limits
#define MAX_CLIENTS 100
#define MAX_PENDING_JOBS 10
#define MAX_PROCESSORS INFINITY
#define MAX_THREADS INFINITY

// SSTP Protocols
#define PING "PING"
#define PONG "PONG"
#define OKAY "OKAY"
#define ERRO "ERRO"
#define SOLN "SOLN"
#define WORK "WORK"
#define ABRT "ABRT"

// Type Defines
typedef struct soln_t Soln;
typedef struct work_t Work;

// Solution Message
struct soln_t {
	char diff_str[9];  // Difficulty String
	char seed_str[65]; // Seed String
	char soln_str[17]; // Solution String
	char targ_str[33]; // Target String
	char x_str[257];   // x String
	uint32_t diff;     // Difficulty
	uint32_t alpha;    // Alpha
	uint32_t beta;     // Beta
	BYTE targ[32];     // Target
	BYTE seed[32];     // Seed
	BYTE soln[8];      // Solution
	BYTE x[40];        // x
};

// Work Message
struct work_t {
	char diff_str[9];         // Difficulty String
	char seed_str[65];        // Seed String
	char start_str[17];       // Nonce String
	char worker_count_str[3]; // Worker Count String
	char x_str[257];          // x String
	char solution[96];        // Solution string
	uint sd;                  // Socket descriptor
	uint worker_threads;      // Number of worker threads
	uint8_t worker_count;     // Worker Count
	uint64_t nonce;           // Nonce
	bool searching;           // Solution is being searched
};

// Global Variables
int thread_count;
int worker_count;
pthread_t pid;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
bool log_created = false;
List *work_queue = NULL;

// Function Prototypes
int init_soln(Soln*, char*);
int init_work(Work*, char*);
void log_output(char*);
int verify_solution(char*, bool);
void *compute_work();
void *increment_nonce(void*);
void ping_protocol(int, char*);
void pong_protocol(int, char*);
void okay_protocol(int, char*);
void erro_protocol(int, char*);
void soln_protocol(int, char*);
void work_protocol(int, char*);
void abrt_protocol(int, char*);
void malf_protocol(int, char*);

#endif
