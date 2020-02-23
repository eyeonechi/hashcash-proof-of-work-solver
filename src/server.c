/*
 * COMP30023 Computer Systems
 * Semester 1 2017
 * Project 2     : server.c
 * Student Name  : Ivan Ken Weng Chee
 * Student ID    : 736901
 * Student Email : ichee@student.unimelb.edu.au 
 */

#include "server.h"

// Driver Function
int main(int argc, char **argv) {
	int master_socket, client_socket[100], activity, valread, sd, max_sd, addrlen, new_socket, max_clients = 100, PORT, i, opt = 1;
	char bufferstream[256];
	struct sockaddr_in address;
	struct tm* tm_info;
	time_t timer;
	thread_count = 0;
	bzero(bufferstream,256);
	// Requires port number
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	PORT = atoi(argv[1]);
	// Set of socket descriptors
	fd_set readfds;
	// Initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i ++) {
		client_socket[i] = 0;
	}
	// Create a master socket
	if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("ERROR opening master socket");
		exit(EXIT_FAILURE);
	}
	// Set master socket to allow multiple connections
	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	// Type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	// Bind the socket to localhost port
	if (bind(master_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("ERROR on binding");
		exit(EXIT_FAILURE);
	}
	// Try to specify maximum of n pending connections for the master socket
	if (listen(master_socket, max_clients) < 0) {
		perror("ERROR on listening");
		exit(EXIT_FAILURE);
	}
	// Accept the incomming connection
	addrlen = sizeof(address);
	puts("Waiting for connections ...");
	while (true) {
		// Clear the socket set
		FD_ZERO(&readfds);
		// Add master socket to set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;
		// Add child sockets to set
		for (i = 0; i < max_clients; i ++) {
			// Socket descriptor
			sd = client_socket[i];
			// If valid socket descriptor, add to read list
			if (sd > 0) {
				FD_SET(sd, &readfds);
			}
			// Highest file descriptor number, need it for the select function
			if (sd > max_sd) {
				max_sd = sd;
			}
		}
		// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
		if ((activity < 0) && (errno != EINTR)) {
			printf("Select error");
		}
		// If something happened on the master socket, then it is an incomming connection
		if (FD_ISSET(master_socket, &readfds)) {
			if ((new_socket = accept(master_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
				perror("ERROR on accepting");
				exit(EXIT_FAILURE);
			}
			// Inform user of socket number
			char datetime[26];
			time(&timer);
		    tm_info = localtime(&timer);
		    strftime(datetime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
			char log_connect[256] = {0};
			sprintf(log_connect,
				"Connect:\n    Socket   : %d\n    Timestamp: %s\n    IP       : %s\n    Port     : %d\n",
				new_socket,
				datetime,
				inet_ntoa(address.sin_addr),
				ntohs(address.sin_port));
			log_output(log_connect);
			// Add new socket to array of sockets
			for (i = 0; i < max_clients; i ++) {
				// If position is empty
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					break;
				}
			}
		}
		// Else its some IO operation on some other socket
		for (i = 0; i < max_clients; i ++) {
			sd = client_socket[i];
			if (FD_ISSET(sd, &readfds)) {
				// Check if it was for closing, and also read the incomming message
				if ((valread = read(sd, bufferstream, 255)) == 0) {
					// Somebody disconnected, get details and print
					getpeername(sd, (struct sockaddr *) &address, (socklen_t *) &addrlen);
					char datetime[26];
					time(&timer);
				    tm_info = localtime(&timer);
				    strftime(datetime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
					char log_disconnect[256] = {0};
					sprintf(log_disconnect,
					"Disconnect:\n    Socket   : %d\n    Timestamp: %s\n    IP       : %s\n    Port     : %d\n",
					sd,
					datetime,
					inet_ntoa(address.sin_addr),
					ntohs(address.sin_port));
					log_output(log_disconnect);
					// Close the socket and mark as 0 in list for reuse
					close(sd);
					client_socket[i] = 0;
				}
				// Echo back the message that came in
				else {
					// Delimit buffer on \r\n token
					char *buffer;
					buffer = strtok(bufferstream, "\r\n");
					while (buffer != NULL) {
						// PING
						if (strncmp(buffer, PING, 4) == 0) {
							ping_protocol(sd, buffer);
						}
						// PONG
						else if (strncmp(buffer, PONG, 4) == 0) {
							pong_protocol(sd, buffer);
						}
						// OKAY
						else if (strncmp(buffer, OKAY, 4) == 0) {
							okay_protocol(sd, buffer);
						}
						// ERRO
						else if (strncmp(buffer, ERRO, 4) == 0) {
							erro_protocol(sd, buffer);
						}
						// SOLN
						else if (strncmp(buffer, SOLN, 4) == 0) {
							soln_protocol(sd, buffer);
						}
						// WORK
						else if (strncmp(buffer, WORK, 4) == 0) {
							work_protocol(sd, buffer);
						}
						// ABRT
						else if (strncmp(buffer, ABRT, 4) == 0) {
							abrt_protocol(sd, buffer);
						}
						// MALF
						else {
							malf_protocol(sd, buffer);
						}
						buffer = strtok(NULL, "\r\n");
					}
				}
			}
		}
	}
	// close socket
	close(master_socket);
	return 0;
}

// Handles PING messages
void ping_protocol(int sd, char *buffer) {
	char log_read[256] = {0};
	sprintf(log_read, "Socket %d:\n", sd);
	strcat(log_read, "    Type    : PING\n");
	strcat(log_read, "    Received: ");
	strcat(log_read, buffer);
	strcat(log_read, "\n");
	log_output(log_read);
	char *reply = "PONG\r\n                                ";
	send(sd, reply, 40, 0);
	char log_send[256] = {0};
	strcat(log_send, "    Sent    : ");
	strcat(log_send, reply);
	strcat(log_send, "\n");
	log_output(log_send);
}

// Handles PONG messages
void pong_protocol(int sd, char *buffer) {
	char log_read[256] = {0};
	sprintf(log_read, "Socket %d:\n", sd);
	strcat(log_read, "    Type    : PONG\n");
	strcat(log_read, "    Received: ");
	strcat(log_read, buffer);
	strcat(log_read, "\n");
	log_output(log_read);
	char *reply = "ERRO Strictly for server response   \r\n";
	send(sd, reply, 40, 0);
	char log_send[256] = {0};
	strcat(log_send, "    Sent    : ");
	strcat(log_send, reply);
	strcat(log_send, "\n");
	log_output(log_send);
}

// Handles OKAY messages
void okay_protocol(int sd, char *buffer) {
	char log_read[256] = {0};
	sprintf(log_read, "Socket %d:\n", sd);
	strcat(log_read, "    Type    : OKAY\n");
	strcat(log_read, "    Received: ");
	strcat(log_read, buffer);
	strcat(log_read, "\n");
	log_output(log_read);
	char *reply = "ERRO Not okay to send OKAY messages\r\n ";
	send(sd, reply, 40, 0);
	char log_send[256] = {0};
	strcat(log_send, "    Sent    : ");
	strcat(log_send, reply);
	strcat(log_send, "\n");
	log_output(log_send);
}

// Handles ERRO messages
void erro_protocol(int sd, char *buffer) {
	char log_read[256] = {0};
	sprintf(log_read, "Socket %d:\n", sd);
	strcat(log_read, "    Type    : ERRO\n");
	strcat(log_read, "    Received: ");
	strcat(log_read, buffer);
	strcat(log_read, "\n");
	log_output(log_read);
	char *reply = "ERRO Should not send to the server  \r\n";
	send(sd, reply, 40, 0);
	char log_send[256] = {0};
	strcat(log_send, "    Sent    : ");
	strcat(log_send, reply);
	strcat(log_send, "\n");
	log_output(log_send);
}

// Handles SOLN messages
void soln_protocol(int sd, char *buffer) {
	char log_read[256] = {0};
	sprintf(log_read, "Socket %d:\n", sd);
	strcat(log_read, "    Type    : SOLN\n");
	strcat(log_read, "    Received: ");
	strcat(log_read, buffer);
	strcat(log_read, "\n\n");
	log_output(log_read);
	int res = verify_solution(buffer, false);
	if (res == 1) {
		char *reply = "OKAY\r\n                                ";
		send(sd, reply, 40, 0);
		char log_send[256] = {0};
		sprintf(log_send, "Socket %d:\n", sd);
		strcat(log_send, "    Sent    : ");
		strcat(log_send, reply);
		strcat(log_send, "\n");
		log_output(log_send);
	} else if (res == -2) {
		char *reply = "ERRO Invalid solution message\r\n       ";
		send(sd, reply, 40, 0);
		char log_send[256] = {0};
		sprintf(log_send, "Socket %d:\n", sd);
		strcat(log_send, "    Sent    : ");
		strcat(log_send, reply);
		strcat(log_send, "\n");
		log_output(log_send);
	} else {
		char *reply = "ERRO Solution verification failed\r\n   ";
		send(sd, reply, 40, 0);
		char log_send[256] = {0};
		sprintf(log_send, "Socket %d:\n", sd);
		strcat(log_send, "    Sent    : ");
		strcat(log_send, reply);
		strcat(log_send, "\n");
		log_output(log_send);
	}
}

// Handles WORK messages
void work_protocol(int sd, char *buffer) {
	char log_read[256] = {0};
	sprintf(log_read, "Socket %d:\n", sd);
	strcat(log_read, "    Type    : WORK\n");
	strcat(log_read, "    Received: ");
	strcat(log_read, buffer);
	strcat(log_read, "\n\n");
	log_output(log_read);
	
	Work w;
	if (init_work(&w, buffer) != -2) {
		if (work_queue) {
			if (getSize(work_queue) > MAX_PENDING_JOBS) {
				char *reply = "ERRO Job queue limit reached\r\n        ";
				send(sd, reply, strlen(reply), 0);
				char log_send[256] = {0};
				sprintf(log_send, "Socket %d:\n", sd);
				strcat(log_send, "    Sent    : ");
				strcat(log_send, reply);
				strcat(log_send, "\n");
				log_output(log_send);
			} else {
				append(work_queue->next, work_queue, &w, sd);
				pthread_create(&pid, NULL, compute_work, NULL);
			}
		} else {
			work_queue = initList(&w, sd);
			pthread_create(&pid, NULL, compute_work, NULL);
		}
	} else {
		char *reply = "ERRO Invalid work message\r\n           ";
		send(sd, reply, strlen(reply), 0);
		char log_send[256] = {0};
		sprintf(log_send, "Socket %d:\n", sd);
		strcat(log_send, "    Sent    : ");
		strcat(log_send, reply);
		strcat(log_send, "\n");
		log_output(log_send);
	}
}

// Handles ABRT messages
void abrt_protocol(int sd, char *buffer) {
	char log_read[256] = {0};
	sprintf(log_read, "Socket %d:\n", sd);
	strcat(log_read, "    Type    : ABRT\n");
	strcat(log_read, "    Received: ");
	strcat(log_read, buffer);
	strcat(log_read, "\n");
	log_output(log_read);
	
	// Destroys the work queue
	while (work_queue) {
		popHead(&work_queue, work_queue->next, true);
	}
	pthread_cancel(pid);
	char *reply = "OKAY\r\n                                ";
	send(sd, reply, 40, 0);
	char log_send[256] = {0};
	strcat(log_send, "    Sent    : ");
	strcat(log_send, reply);
	strcat(log_send, "\n");
	log_output(log_send);
}

// Handles malfunctioned messages
void malf_protocol(int sd, char *buffer) {
	char log_read[256] = {0};
	sprintf(log_read, "Socket %d:\n", sd);
	strcat(log_read, "    Type    : MALF\n");
	strcat(log_read, "    Received: ");
	strcat(log_read, buffer);
	strcat(log_read, "\n");
	log_output(log_read);
	char *reply = "ERRO This is a malfunctioned message\r\n";
	send(sd, reply, 40, 0);
	char log_send[256] = {0};
	strcat(log_send, "    Sent    : ");
	strcat(log_send, reply);
	strcat(log_send, "\n");
	log_output(log_send);
}

// Initialises a Solution Message structure
int init_soln(Soln *s, char *buffer) {
	memset(s->diff_str, '\0', sizeof(s->diff_str));
	memset(s->seed_str, '\0', sizeof(s->seed_str));
	memset(s->soln_str, '\0', sizeof(s->soln_str));
	memset(s->targ_str, '\0', sizeof(s->targ_str));
	// Extract msg contents into solution
	uint i;
	for (i = 0; i < strlen(buffer); i ++) {
		if (i < 8 && sscanf(buffer + i + 5, "%c", &(s->diff_str[i])) != 1) {
			return -2;
		} else if (i >= 8 && i < 72 && sscanf(buffer + i + 6, "%c", &(s->seed_str[i - 8])) != 1) {
			return -2;
		} else if (i >= 72 && i < 88 && sscanf(buffer + i + 7, "%c", &(s->soln_str[i - 72])) != 1) {
			return -2;
		}
	}
	
	strncpy(s->x_str, s->seed_str, 64);
	s->x_str[64] = '\0';
	strcat(s->x_str, s->soln_str);
	s->x_str[80] = '\0';
	return 0;
}

// Initialises a Work Message structure
int init_work(Work *w, char *buffer) {
	memset(w->diff_str, '\0', sizeof(w->diff_str));
	memset(w->seed_str, '\0', sizeof(w->seed_str));
	memset(w->start_str, '\0', sizeof(w->start_str));
	memset(w->worker_count_str, '\0', sizeof(w->worker_count_str));
	memset(w->x_str, '\0', sizeof(w->x_str));
	w->nonce = 0;
	// Extract message contents into work
	uint i;
	for (i = 0; i < strlen(buffer); i ++) {
		if (i < 8 && sscanf(buffer + i + 5, "%c", &(w->diff_str[i])) != 1) {
			return -2;
		} else if (i >= 8 && i < 72 && sscanf(buffer + i + 6, "%c", &(w->seed_str[i - 8])) != 1) {
			return -2;
		} else if (i >= 72 && i < 88 && sscanf(buffer + i + 7, "%c", &(w->start_str[i - 72])) != 1) {
			return -2;
		} else if (i >= 88 && i < 90 && sscanf(buffer + i + 8, "%c", &(w->worker_count_str[i - 88])) != 1) {
			return -2;
		}
	}
	w->x_str[64] = '\0';
	strcat(w->x_str, w->start_str);
	w->x_str[80] = '\0';
	return 0;
}

// Stage B
int verify_solution(char *buffer, bool compute) {
	Soln s;
	if (init_soln(&s, buffer) < 0) {
		return -2;
	}
	
	// Convert difficulty and extract alpha and beta
	s.diff = strtol(s.diff_str, NULL, 16);
	s.alpha = s.diff >> 24;
	s.beta = s.diff & 0xFFFFFF;
	
	// Convert beta into a byte array
	BYTE base[32];
	BYTE beta[32];
	uint256_init(base);
	uint256_init(beta);
	base[31] = 0x2;
	beta[28] = (s.beta >> 24) & 0xFF;
	beta[29] = (s.beta >> 16) & 0xFF;
	beta[30] = (s.beta >> 8) & 0xFF;
	beta[31] = s.beta & 0xFF;
	
	// Calculate target
	uint256_init(s.targ);
	uint256_exp(s.targ, base, 8 * (s.alpha - 3));
	uint256_mul(s.targ, s.targ, beta);
	
	// Construct x as the concatenation of seed | solution
	uint256_init(s.seed);
	uint256_init(s.soln);
	uint256_init(s.x);
	uint i;
	for (i = 0; i < (strlen(s.x_str) / 2); i ++) {
		if (i < 32) {
        	sscanf(s.seed_str + 2 * i, "%02x", (unsigned int*) &(s.x[i]));
		} else {
			sscanf(s.soln_str + 2 * (i - 32), "%02x", (unsigned int*) &(s.x[i]));
		}
    }
	
	// Hash x twice and return comparison with target
	BYTE first[32];
	BYTE second[32];
	SHA256_CTX ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, s.x, 40);
	sha256_final(&ctx, first);
	sha256_init(&ctx);
	sha256_update(&ctx, first, 32);
	sha256_final(&ctx, second);
	
	if (!compute) {
		char log_soln[256] = {0};
		sprintf(log_soln,
			"Solution:\n    Difficulty: %s\n    Seed      : %s\n    Solution  : %s\n    x         : %s\n\n",
			s.diff_str,
			s.seed_str,
			s.soln_str,
			s.x_str);
		log_output(log_soln);
	}
	
	return sha256_compare(s.targ, second);
}

// Stage C
void *compute_work() {
	pthread_mutex_lock(&mutex2);
	thread_count ++;
	if (work_queue) {
		Work *w = (Work *) work_queue->data;
		w->worker_threads = 0;
		sscanf(w->worker_count_str, "%" SCNu8, &w->worker_count);
		pthread_t wid[w->worker_count];
		int id[w->worker_count];
		uint i;
		for (i = 0; i < w->worker_count; i ++) {
			id[i] = i;
			pthread_create(&wid[i], NULL, increment_nonce, (void *)&(id[i]));
		}
		for (i = 0; i < w->worker_count; i ++) {
			pthread_join(wid[i], NULL);
		}
		char log_send[256] = {0};
		sprintf(log_send, "Socket %d:\n", work_queue->socket);
		strcat(log_send, "    Sent    : ");
		strcat(log_send, w->solution);
		strcat(log_send, "\n");
		log_output(log_send);
	}
	if (!work_queue->pending) {
		popHead(&work_queue, work_queue->next, true);
	}
	thread_count --;
	pthread_mutex_unlock(&mutex2);
	return NULL;
}

void *increment_nonce(void *id) {
	Work *w = (Work *) work_queue->data;
	w->worker_threads ++;
	fprintf(stdout,
			"Work:\n    Difficulty: %s\n    Seed      : %s\n    Start     : %s\n    Workers   : %s\n    Socket    : %d\n\n",
			w->diff_str,
			w->seed_str,
			w->start_str,
			w->worker_count_str,
			w->sd);
	char *nonce_str = w->start_str;
	uint64_t nonce;
	sscanf(nonce_str, "%" SCNx64, &nonce);
	nonce += *((int *) id);
	// nonce += *((int *) id) * ((UINT64_MAX - nonce) / w->worker_count);
	
	// Increment nonce and try looking for a valid SOLN
	do {
		// Concatenates the new nonce
		memset(w->solution, '\0', sizeof(w->solution));
		strcat(w->solution, "SOLN ");
		strcat(w->solution, w->diff_str);
		strcat(w->solution, " ");
		strcat(w->solution, w->seed_str);
		strcat(w->solution, " ");
		strcat(w->solution, nonce_str);
		strcat(w->solution, "\r\n");
		
		// Checks validity using verify_solution()
		if (verify_solution(w->solution, true) == 1) {
			work_queue->pending = false;
			send(work_queue->socket, w->solution, strlen(w->solution), 0);
			break;
		}
		
		// Increment the nonce
		nonce += w->worker_count;
		bzero(nonce_str, 16);
		sprintf(nonce_str, "%" PRIx64, nonce);
		
	} while (work_queue->pending && nonce < UINT64_MAX);
	worker_count --;
	return NULL;
}

// Prints to a log file
void log_output(char *buffer) {
	pthread_mutex_lock(&mutex1);
	FILE *file;
	if (!log_created) {
		file = fopen(LOGFILE, "w");
		char *header = "COMP30023 Computer Systems\n2017 Semester 1\nProject 2     : log.txt\nStudent Name  : Ivan Ken Weng Chee\nStudent ID    : 736901\nStudent Email : ichee@student.unimelb.edu.au\n\n";
		fputs(header, file);
		log_created = true;
	} else {
		file = fopen(LOGFILE, "a");
	}
	if (file == NULL && log_created) {
		log_created = false;
	} else {
		fputs(buffer, file);
		fprintf(stdout, "%s", buffer);
		fclose(file);
	}
	pthread_mutex_unlock(&mutex1);
}
