/* Copyright 2023 <> */
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "server_hashmap.h"

struct kv_node_t {
	void *key;
	void *value;
	struct kv_node_t *next;
};

struct server_memory {
	// hashmap implementation using chaining

	// variables of the key-value pair buckets array
	// every element of 'buckets' stores the head of the list and not the size
	// as well there is no need to hold the size of every individual bucket
	// because only the overall size is needed and it also costs extra memory
	kv_node_t **buckets;
	unsigned int size;
	unsigned int hmax;

	// key hash and key compare functions
	unsigned int (*hash)(void *a);
	int (*cmp)(void *p1, void *p2);
};

unsigned int hash_function_key(void *a)
{
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

server_memory *init_server_memory()
{
	server_memory *server;

	server = calloc(1, sizeof(*server));
	DIE(!server, "failed");

	// alloc space for the buckets array
	server->buckets = calloc(BASESIZE, sizeof(void *));
	DIE(!server->buckets, "failed");

	// initial values
	server->hmax = BASESIZE;
	server->hash = hash_function_key;
	server->cmp = (void *)strcmp;

	return server;
}

void server_store(server_memory *server, char *key, char *value)
{
	unsigned int hash;
	void *new_key, *new_value;
	kv_node_t *curr, *new_node;

	// memory for the new value
	new_value = calloc(1, strlen(value) + 1);
	DIE(!new_value, "failed");

	memcpy(new_value, value, strlen(value));

	// variable that helps with dereferencing
	hash = server->hash(key) % server->hmax;

	for (curr = server->buckets[hash]; curr; curr = curr->next)
		if (server->cmp(curr->key, key) == 0) {
			free(curr->value);
			curr->value = new_value;

			// if the entry is found than we replace the value and simply return
			return;
		}

	// if there is no entry with given key
	// a node with the given key and value is created and introduced
	new_node = calloc(1, sizeof(*new_node));
	DIE(!new_node, "failed");

	new_key = calloc(1, strlen(key) + 1);
	DIE(!new_key, "failed");

	memcpy(new_key, key, strlen(key));

	new_node->key = new_key;
	new_node->value = new_value;

	// add the node to the bucket
	new_node->next = server->buckets[hash];
	server->buckets[hash] = new_node;

	++(server->size);
}

char *server_retrieve(server_memory *server, char *key)
{
	unsigned int hash;
	kv_node_t *curr;

	// variable that helps with dereferencing
	hash = server->hash(key) % server->hmax;

	for (curr = server->buckets[hash]; curr; curr = curr->next)
		if (server->cmp(curr->key, key) == 0)
			//  if the entry is found than return it
			return curr->value;

	return NULL;
}

void server_remove(server_memory *server, char *key)
{
	unsigned int hash;
	kv_node_t *curr, *back = NULL;

	// variable that helps with dereferencing
	hash = server->hash(key) % server->hmax;

	for (curr = server->buckets[hash]; curr; back = curr, curr = curr->next)
		if (server->cmp(curr->key, key) == 0)
			// if an entry with given key is found then break to remove it
			break;

	// if there is no entry with the given key simply return
	if (!curr)
		return;

	// remove the entry
	if (back)
		back->next = curr->next;
	else
		server->buckets[hash] = curr->next;

	// free the space of the entry
	free(curr->key);
	free(curr->value);
	free(curr);

	--(server->size);
}

void free_server_memory(server_memory *server)
{
	unsigned int i, n, count;
	kv_node_t *curr, *aux;

	// variables that help with dereferencing
	count = server->size;
	n = server->hmax;

	for (i = 0; i < n && count; ++i)
		for (curr = server->buckets[i]; curr; curr = aux) {
			aux = curr->next;

			// free the space of the entry
			free(curr->key);
			free(curr->value);
			free(curr);

			--count;
		}

	// free the buckets vector and the hashmap struct
	free(server->buckets);
	free(server);
}

int empty(server_memory *server)
{
	return server->size == 0;
}

void transf_elm(server_memory *dest, server_memory *src, unsigned int ref_hash)
{
	unsigned int i, count, n;
	kv_node_t *curr, *prev, *aux;

	// variables that help with dereferencing
	count = src->size;
	n = src->hmax;

	for (i = 0; i < n && count; ++i)
		if (src->buckets[i]) {
			for (prev = NULL, curr = src->buckets[i]; curr; curr = aux) {
				aux = curr->next;

				if (src->hash(curr->key) <= ref_hash) {
					// add the entry to the other hashmap
					server_store(dest, curr->key, curr->value);

					// delete it from the list and free the memory
					if (prev)
						prev->next = aux;
					else
						src->buckets[i] = aux;

					free(curr->key);
					free(curr->value);
					free(curr);

					--(src->size);
				} else {
					prev = curr;
				}

				--count;
			}
		}
}
