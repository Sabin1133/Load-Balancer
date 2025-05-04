#include <stdint.h>


struct Endpoint {
    uint32_t address;
    uint16_t port;

    bool operator==(const Endpoint &other) const
    {
        return this->address == other.address && this->port == other.port;
    }
};

struct EndpointHasher {
    std::size_t operator()(const Endpoint &endpoint) const
    {
        std::size_t ulong_a = endpoint.address;

        ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
        ulong_a = (ulong_a >> 16u) ^ endpoint.port;
        ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
        ulong_a = (ulong_a >> 16u) ^ ulong_a;

        return ulong_a;
    }
};

struct HashNode {
    unsigned int id;
    std::size_t hash;

    HashNode(unsigned int id, std::size_t hash): id(id), hash(hash) {}
};

class HashRing {
private:
    unsigned int n;
    unsigned int size;

    std::unordered_map<Endpoint, unsigned int, EndpointHasher> ids;
    std::unordered_map<unsigned int, Endpoint> endpoints;
    std::vector<HashNode> nodes;

public:
    HashRing(unsigned int servers_n);

private:
    unsigned int _find_smallest_bigger(std::size_t hash);

public:
    void add(Endpoint server_endpoint);
    void remove(Endpoint server_endpoint);

    Endpoint distribute(Endpoint client_endpoint);

    void print_ring();
};
