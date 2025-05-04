#ifndef HASH_RING_H
#define HASH_RING_H

#include <stdint.h>

#include <vector>
#include <unordered_map>


typedef unsigned long ring_id_t;


struct Endpoint {
    uint32_t address;
    uint16_t port;

    Endpoint();
    Endpoint(uint32_t address, uint16_t port);

    bool operator==(const Endpoint &other) const;
};

namespace std {
    template <>
    struct hash<Endpoint> {
        std::size_t operator()(const Endpoint &endpoint) const;
    };
}

struct HashNode {
    ring_id_t id;
    std::size_t hash;

    HashNode(unsigned int id, std::size_t hash);
};

class HashRing {
private:
    unsigned int n;
    unsigned int size;

    std::unordered_map<ring_id_t, Endpoint> endpoints;
    std::vector<HashNode> nodes;

public:
    HashRing();
    HashRing(unsigned int servers_n);

private:
    unsigned int _find_smallest_bigger(std::size_t hash);

public:
    void add(Endpoint server_endpoint);
    void remove(Endpoint server_endpoint);

    Endpoint distribute(Endpoint client_endpoint);

    /*----testing----*/
    
    void print_ring();
};

#endif // hash_ring.hpp
