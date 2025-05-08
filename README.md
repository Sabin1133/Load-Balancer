Copyright (c) 2024 Sabin Padurariu

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
━━━┏━━━┓┏━━━┓┏━━━┓━━━━┏┓━━┏┓┏━━━┓┏┓━┏┓━━━━┏━━━┓┏━━━┓┏━━━┓┏━━━┓┏━━┓┏━┓━┏┓┏━━━┓━━━
━━━┃┏━┓┃┃┏━┓┃┃┏━━┛━━━━┃┗┓┏┛┃┃┏━┓┃┃┃━┃┃━━━━┃┏━┓┃┃┏━━┛┃┏━┓┃┗┓┏┓┃┗┫┣┛┃┃┗┓┃┃┃┏━┓┃━━━
━━━┃┃━┃┃┃┗━┛┃┃┗━━┓━━━━┗┓┗┛┏┛┃┃━┃┃┃┃━┃┃━━━━┃┗━┛┃┃┗━━┓┃┃━┃┃━┃┃┃┃━┃┃━┃┏┓┗┛┃┃┃━┗┛━━━
━━━┃┗━┛┃┃┏┓┏┛┃┏━━┛━━━━━┗┓┏┛━┃┃━┃┃┃┃━┃┃━━━━┃┏┓┏┛┃┏━━┛┃┗━┛┃━┃┃┃┃━┃┃━┃┃┗┓┃┃┃┃┏━┓━━━
━━━┃┏━┓┃┃┃┃┗┓┃┗━━┓━━━━━━┃┃━━┃┗━┛┃┃┗━┛┃━━━━┃┃┃┗┓┃┗━━┓┃┏━┓┃┏┛┗┛┃┏┫┣┓┃┃━┃┃┃┃┗┻━┃━━━
━━━┗┛━┗┛┗┛┗━┛┗━━━┛━━━━━━┗┛━━┗━━━┛┗━━━┛━━━━┗┛┗━┛┗━━━┛┗┛━┗┛┗━━━┛┗━━┛┗┛━┗━┛┗━━━┛━━━
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┏━┓┏━┓┏━━━┓━━━━┏━━━┓━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┃┃┗┛┃┃┃┏━━┛━━━━┃┏━┓┃━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┃┏┓┏┓┃┃┗━━┓━━━━┗┛┏┛┃━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┃┃┃┃┃┃┃┏━━┛━━━━━━┃┏┛━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┃┃┃┃┃┃┃┗━━┓━━━━━━┏┓━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┗┛┗┛┗┛┗━━━┛━━━━━━┗┛━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

# **Transport Layer Load Balancer**

## **General Overview**

Trivial implementation of a load balancer that leverages consistent hashing to
distribute the load to a large number of servers that have limited computing
power.

The balancer runs in the background and is configured using the `balancerctl`
utility.

## Getting Started

To install both the executable and the utility, run the provided installation
script `install.sh`.

## Functionality

The load balancer distributes the load evenly and efficiently across all servers
using a technique called **consistent hashing**.

Consistent hashing is frequently used in distributed systems and has the
advantage of satisfying the **minimal disruption constraint**, meaning it 
reduces the amount of necessary transfers when a server is turned on or off.

When a client connects, the balancer finds a server to establish a connection
with and forwards the traffic to the selected server.

Currently, if a server is removed or shuts down abruptly, all client connections
to that server are closed as well. Normally, the clients should be redistributed
to the other servers as a result of consistent hashing but this feature is yet
to be implemented.

## Usage and API

The load balancer can be controlled using the `balancerctl` utility that
provides five main actions:

1. **Start** the balancer
2. **Stop** the balancer
3. **Add** server to balancer's pool
4. **Remove** server from balancer's pool
5. **List** all servers in balancer's pool

### Start and stop balancer

```bash
user@hostname:~$ balancerctl start <inet_address> <inet_port>
```
```bash
user@hostname:~$ balancerctl stop
```

### Configure servers

```bash
user@hostname:~$ balancerctl add <inet_address> <inet_port>
```
```bash
user@hostname:~$ balancerctl remove <inet_address> <inet_port>
```
```bash
user@hostname:~$ balancerctl list
```

## Implementation

### The Connection Engine

The **connection engine** is the **heart** of the balancer and is resposible
with accepting and distributing incoming connections and forwarding the traffic
from the clients to the servers.

By leveraging the **multiplexing meachanism** provided by `epoll` kernel data
structure, the balancer is able to achieve fast response times and 
high performance.

### The Hash Ring

The theoretical model used in consistent hashing. It is used to distribute
the connections across the servers and has a circular nature.

The hash ring is implemented using an array and stores nodes that represent
the servers.The nodes hold the hash of the server and the server id, a unique
identifier given to the server when first added to the pool.

The nodes array is sorted by the hash in each node to guarantee correct
distribution across all servers. Clients are matched using their hash, using a
simple binary search.
