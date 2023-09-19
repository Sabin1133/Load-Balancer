/* Copyright 2023 Padurariu Sabin */
#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include "server_hashmap.h"

struct server_info;
typedef struct server_info server_info;

struct load_balancer;
typedef struct load_balancer load_balancer;

/**
 * init_load_balancer() - initializes the memory for a new load balancer and its
 *                        fields and
 *                        returns a pointer to it
 *
 * Return: pointer to the load balancer struct
 */
load_balancer *init_load_balancer();

/**
 * free_load_balancer() - frees the memory of every field that is related to the
 * load balancer (servers, hashring)
 *
 * @arg1: Load balancer to free
 */
void free_load_balancer(load_balancer *main);

/**
 * load_store() - Stores the key-value pair inside the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 * @arg4: This function will RETURN via this parameter
 *        the server ID which stores the object.
 *
 * The load balancer will use Consistent Hashing to distribute the
 * load across the servers. The chosen server ID will be returned
 * using the last parameter.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where
 * the entry should be stored and call the function to store the entry on the
 * respective server.
 *
 */
void loader_store(load_balancer *main, char *key, char *value, int *server_id);

/**
 * load_retrieve() - Gets a value associated with the key.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: This function will RETURN the server ID
 *        which stores the value via this parameter.
 *
 * The load balancer will search for the server which should posess the
 * value associated to the key. The server will return NULL in case
 * the key does NOT exist in the system.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where
 * the entry should be stored and call the function to store the entry on the
 * respective server.
 */
char *loader_retrieve(load_balancer *main, char *key, int *server_id);

/**
 * load_add_server() - Adds a new server to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * The load balancer will generate 3 replica labels and it will
 * place them inside the hash ring. The neighbor servers will
 * distribute some the objects to the added server.
 *
 * Hint:
 * Resize the servers array to add a new one.
 * Add each label in the hashring in its appropiate position.
 * Do not forget to resize the hashring and redistribute the objects
 * after each label add (the operations will be done 3 times, for each replica).
 */
void loader_add_server(load_balancer *main, int server_id);

/**
 * load_remove_server() - Removes a specific server from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the removed server.
 *
 * The load balancer will distribute ALL objects stored on the
 * removed server and will delete ALL replicas from the hash ring.
 *
 */
void loader_remove_server(load_balancer *main, int server_id);

/*--------AUX-FUNCTIONS--------*/

/**
 * resize_load() - Grows or shrinks the load
 * @arg1: Load balancer to resize
 * @arg2: option
 *
 * If the option is 1 then the load grows, if it is -1 then it shrinks
 */
void resize_load(load_balancer *main, int op);

/**
 * search_server() - Returns the index of the server within the load_balancer
 * @arg1: Load balancer which distributes the work.
 * @arg2: Hash of the server or key.
 *
 * The functions returns the index of the closest server with the hash bigger or
 * equal to the given one.
 */
int search_server(load_balancer *main, unsigned int ref_hash);

/**
 * insert_space() - Inserts a space in the array
 * @arg1: Load balancer in which to insert
 * @arg2: position
 *
 * The function insert a blank space and shifts all the elements to the right
 * by one
 */
void insert_space(load_balancer *main, unsigned int pos);

/**
 * remove_space() - Inserts a space from the array
 * @arg1: Load balancer from which to remove
 * @arg2: position to be removed
 *
 * The function removes a space and shifts all the elements to the left by one
 */
void remove_space(load_balancer *main, unsigned int pos);

#endif /* LOAD_BALANCER_H_ */
