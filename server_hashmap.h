/* Copyright 2023 Padurariu Sabin */
#ifndef SERVER_H_
#define SERVER_H_

#define BASESIZE 128

struct kv_node_t;
typedef struct kv_node_t kv_node_t;

struct server_memory;
typedef struct server_memory server_memory;

unsigned int hash_function_key(void *a);

/** init_server_memory() -  Initializes the memory for a new server struct.
 * 							Make sure to check what is returned by malloc using DIE.
 * 							Use the linked list implementation from the lab.
 *
 * Return: pointer to the allocated server_memory struct.
 */
server_memory *init_server_memory();

/** free_server_memory() - Free the memory used by the server.
 * 						   Make sure to also free the pointer to the server struct.
 * 						   You can use the server_remove() function for this.
 *
 * @arg1: Server to free
 */
void free_server_memory(server_memory *server);

/**
 * server_store() - Stores a key-value pair to the server.
 *
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 */
void server_store(server_memory *server, char *key, char *value);

/**
 * server_remove() - Removes a key-pair value from the server.
 *					 Make sure to free the memory of everything that is related to the entry removed.
 *
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 */
void server_remove(server_memory *server, char *key);

/**
 * server_retrieve() - Gets the value associated with the key.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 *
 * Return: String value associated with the key
 *         or NULL (in case the key does not exist).
 */
char *server_retrieve(server_memory *server, char *key);

/*-------AUX-FUNCTIONS-------*/

/**
 * empty() - checks to see if the hashmap is empty
 * @arg1: hashmap
 *
 */
int empty(server_memory *server);

/**
 * transf_elm() - Transfers all the entries form the SRC hashmap to the DEST
 *                 hashmap that have a smaller or equal hash than the given one
 * @arg1: Destination hashmap.
 * @arg2: Source hashmap.
 * @arg3: give hash.
 *
 */
void transf_elm(server_memory *dest, server_memory *src, unsigned int ref_hash);

#endif /* SERVER_H_ */
