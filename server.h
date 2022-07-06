/* Copyright 2021 <> */
#ifndef SERVER_H_
#define SERVER_H_

#include "linked_list.h"

#define N_BUCKETS 31

typedef struct server_memory server_memory;
struct server_memory {
	//  array de liste simplu inlantuite
	linked_list_t **buckets;

	//  dimensiunea server-ului
	unsigned int size;

	//  numarul maxim de bucket-uri
	unsigned int hmax;
};

//  pereche cheie-valoare
struct info {
	char* key;
	char* value;
};

server_memory* init_server_memory();

unsigned int hash_function_key(void *a);

void free_server_memory(server_memory* server);

/**
 * server_store() - Stores a key-value pair to the server.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 */
void server_store(server_memory* server, char* key, char* value);

/**
 * server_remove() - Removes a key-pair value from the server.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 */
void server_remove(server_memory* server, char* key);

/**
 * server_remove() - Gets the value associated with the key.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 *
 * Return: String value associated with the key
 *         or NULL (in case the key does not exist).
 */
char* server_retrieve(server_memory* server, char* key);

struct info** server_retrieve_all(server_memory* server);

#endif  // SERVER_H_
