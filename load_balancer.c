/* Copyright 2023 <> */
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "load_balancer.h"

struct server_info {
	// stores the hash of the label (ex. hash(000034), hash(200034))
	unsigned int hash;
	// but only the id 34
	unsigned int id;
	// just a hashmap reference
	server_memory *memory;
};

struct load_balancer {
	// basically a resizable array but with an extra hash function
	server_info *servers;
	unsigned int size;
	unsigned int capacity;

	unsigned int (*hash)(void *a);
};

unsigned int hash_function_servers(void *a)
{
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

load_balancer *init_load_balancer()
{
	load_balancer *balancer;

	// alloc memory for the struct
	balancer = calloc(1, sizeof(*balancer));
	DIE(!balancer, "failed");

	// alloc memory for the array
	balancer->servers = calloc(64, sizeof(server_info));
	DIE(!balancer->servers, "failed");

	// initial values
	balancer->capacity = 64;
	balancer->hash = hash_function_servers;

	return balancer;
}

void resize_load(load_balancer *main, int op)
{
	unsigned int nr;
	void *aux;

	// if op is 1 then it grows by doubling its size
	// if op is -1 then it shrinks, having half the size

	nr = main->capacity;

	if (op == 1) {
		aux = realloc(main->servers, 2 * nr * sizeof(server_info));
		DIE(!aux, "failed");

		main->servers = aux;
		main->capacity = nr * 2;
	} else if (op == -1) {
		aux = realloc(main->servers, (nr / 2) * sizeof(server_info));
		DIE(!aux, "failed");

		main->servers = aux;
		main->capacity = nr / 2;
	}
}

int search_server(load_balancer *main, unsigned int ref_hash)
{
	int l, r, m = 0;
	server_info *v;

	// initial values
	l = 0;
	r = main->size - 1;
	m = (l + r) / 2;
	v = main->servers;

	// binary search but in case of odd number we take the bigger half
	while (l < r && v[m].hash != ref_hash) {
		if (v[m].hash < ref_hash)
			l = m + 1;
		else
			r = m;

		m = (l + r) / 2;
	}

	// if the given hash is bigger than the hash of the index found then by
	// convention the first index is returned to maintain the circular behaviour
	if (v[m].hash < ref_hash)
		return 0;

	return m;
}

void insert_space(load_balancer *main, unsigned int pos)
{
	unsigned int i;
	server_info *sv;

	// variable to help with dereferencing
	sv = main->servers;

	// shift all elements to the right by one to make space
	for (i = main->size; i > pos; --i)
		sv[i] = sv[i - 1];

	// make all fields zero for safety reasons
	sv[pos] = (server_info){0};

	++(main->size);
}

void loader_add_server(load_balancer *main, int server_id)
{
	unsigned int idx, next_idx, k;
	unsigned int label_hash;
	server_info *sv;

	// shrink the size if necessary
	if (main->size + 3 >= main->capacity)
		resize_load(main, 1);

	// variable to help with dereferencing
	sv = main->servers;

	for (k = 0; k < 3; ++k) {
		// compute the hash of the label
		label_hash = main->hash(&(int){k * 100000 + server_id});

		// find the position to insert the server at
		// if the hash is bigger than all the other hashes, add it to the end
		idx = search_server(main, label_hash);
		if (main->servers[idx].hash < label_hash)
			idx = main->size;

		// add space to the array and fill the fields of the struct
		insert_space(main, idx);
		sv[idx].hash = label_hash;
		sv[idx].id = server_id;
		sv[idx].memory = init_server_memory();

		// index of the next server
		next_idx = (idx + 1) % main->size;

		// general case
		// move all the elements, from the next server, with a smaller hash
		// than the newly server
		if (!empty(sv[next_idx].memory))
			transf_elm(sv[idx].memory, sv[next_idx].memory, sv[idx].hash);

		// add server to the end special case
		if (!empty(sv[idx].memory) && next_idx == 0)
			transf_elm(sv[next_idx].memory, sv[idx].memory, sv[next_idx].hash);

		// add server to the front special case
		if (!empty(sv[next_idx].memory) && next_idx == 1) {
			transf_elm(sv[idx].memory, sv[next_idx].memory, __UINT32_MAX__);
			transf_elm(sv[next_idx].memory, sv[idx].memory, sv[next_idx].hash);
			transf_elm(sv[idx].memory, sv[next_idx].memory, sv[idx].hash);
		}
	}
}

void remove_space(load_balancer *main, unsigned int pos)
{
	unsigned int i;
	server_info *sv;

	// variable to help with dereferencing
	sv = main->servers;

	--(main->size);

	// shift all elements to the left by one to remove space
	for (i = pos; i < main->size; ++i)
		sv[i] = sv[i + 1];

	// make all fields zero for safety reasons
	sv[main->size] = (server_info){0};
}

void loader_remove_server(load_balancer *main, int server_id)
{
	unsigned int idx, next_idx, k;
	unsigned int label_hash;
	server_info *sv;

	// variable to help with dereferencing
	sv = main->servers;

	for (k = 0; k < 3; ++k) {
		// compute the hash of the label
		label_hash = main->hash(&(int){k * 100000 + server_id});

		// find the position of the server to be removed
		idx = search_server(main, label_hash);

		// index of the next server
		next_idx = (idx + 1) % main->size;

		// transfer all the entries to the next server regardless of their
		// hash to maintain the circular behaviour
		if (!empty(sv[idx].memory))
			transf_elm(sv[next_idx].memory, sv[idx].memory, __UINT32_MAX__);

		// remove the space from the array and free the corresponding hashmap
		free_server_memory(sv[idx].memory);
		remove_space(main, idx);
	}

	// shrink the size if necessary
	if (main->size <= (main->capacity / 4))
		resize_load(main, -1);
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	int index;

	// find the index of the server where the key needs to be stored
	index = search_server(main, hash_function_key(key));

	*server_id = main->servers[index].id;

	server_store(main->servers[index].memory, key, value);
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	int index;

	// find the index of the server with the key
	index = search_server(main, hash_function_key(key));

	*server_id = main->servers[index].id;

	return server_retrieve(main->servers[index].memory, key);
}

void free_load_balancer(load_balancer *main)
{
	unsigned int i, n;

	n = main->size;

	// free the hashmaps
	for (i = 0; i < n; ++i)
		free_server_memory(main->servers[i].memory);

	// free the array and the loader
	free(main->servers);
	free(main);
}
