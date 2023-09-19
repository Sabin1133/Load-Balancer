##### Copyright 2023 Padurariu Sabin

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━┏━━━┓┏━━━┓┏━━━┓┏━━━┓━━━┏━┓┏━┓┏━━━┓━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━┃┏━┓┃┃┏━━┛┃┏━┓┃┗┓┏┓┃━━━┃┃┗┛┃┃┃┏━━┛━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━┃┗━┛┃┃┗━━┓┃┃━┃┃━┃┃┃┃━━━┃┏┓┏┓┃┃┗━━┓━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━┃┏┓┏┛┃┏━━┛┃┗━┛┃━┃┃┃┃━━━┃┃┃┃┃┃┃┏━━┛━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━┃┃┃┗┓┃┗━━┓┃┏━┓┃┏┛┗┛┃━━━┃┃┃┃┃┃┃┗━━┓━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━┗┛┗━┛┗━━━┛┗┛━┗┛┗━━━┛━━━┗┛┗┛┗┛┗━━━┛━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

<br>

# **SERVER LOAD BALANCER**

## Important remarks and notes beforehand:

* This software relies heavily on dynamic allocated memory and it is
recommended that the input may be of manageable size
* The implementation is based on and uses hashmaps and resizable arrays

<br>

## **General Overview**

The programme is a basic implementation of a load balancer, a device used to
redirect the traffic to a large group of servers that a limited computing power.

So for this reason it uses a technique called consistent hashing to evenly 
distribute the incoming requests to the servers.

This technique is frequently used in distributed systems and it has the 
advantage of providing minimal disruption constraint, which means that it 
reduces the amount of necessary transfers when a server is turned on or off.

It makes use of a hash ring to efficiently store the incoming entries.

## **Processing Commands**

The program's purpose is to efficiently store key and value pairs on servers
and for fast searching and retrieving and can identify 4 different commands 
read from the standard input, every single command having its own arguments 
described below (the arguments are marked with <>):

```
add_server <id>
```
* Adds a server with the given id to the main load balancer, and three copies 
to the hashring, and redistributes the stored elements

```
remove_server <id>
```
* Removes the server with the gievn id, and its copies from the hashring, and
redistributes the stored elements

```
store <key> <value>
```
* Stores the pair efficiently on the servers
* The key and value must be strings that begin and end with "

```
retrieve <key>
```
* Retrieves the value associated with the key form the corresponding server
* The key must be a string

---
<br>

## **Other remarks and programming details for geeks :wink:**

* The implementation is linux kernel based and the code format is done 
accordingly hence every variable is declared at the start of every function
so there are no function scoped variables

* Every list/array used is 0-indexed and allocated dynamically on the heap

* The frees and mallocs/callocs are done in cascade so every free function call
calls other function and so on until all the requested memory is freed
* At the end after all the operations the program closes gracefully and `frees`
all memory allocated on the heap so there are no memory leaks

* the server_memory struct is just a hashmap in disguise and the load_balancer
struct just a resizable array that stores hashmaps according to the 
hash ring rule
* the hashmap can only store string key and value pairs
* for the programme to work properly there needs to be at least a server added 
so that the entries can be stored somewhere

<br>
