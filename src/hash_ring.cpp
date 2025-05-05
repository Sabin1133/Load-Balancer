#include <hash_ring.hpp>

#define MAX_SERVERS_N 1024
#define REPLICAS_N 2


typedef unsigned long ring_label_t;


ring_id_t endpoint_to_ring_id(Endpoint endpoint)
{
    ring_id_t id;

    id = endpoint.address;
    id <<= 16;
    id ^= endpoint.port;

    return id;
}

ring_label_t ring_label_from_ring_id(ring_id_t id, uint16_t tag)
{
    return ((ring_id_t)tag << 48) ^ id;
}

std::size_t ring_label_hash(ring_id_t id)
{
    std::size_t ulong_a = id;

	ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
	ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
	ulong_a = (ulong_a >> 16u) ^ ulong_a;

	return ulong_a;
}

Endpoint::Endpoint(): address(0), port(0) {}
Endpoint::Endpoint(uint32_t address, uint16_t port): address(address), port(port) {}

bool Endpoint::operator==(const Endpoint &other) const
{
    return this->address == other.address && this->port == other.port;
}

std::size_t std::hash<Endpoint>::operator()(const Endpoint &endpoint) const
{
    std::size_t ulong_a = endpoint.address;

    ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
    ulong_a = (ulong_a >> 16u) ^ endpoint.port;
    ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
    ulong_a = (ulong_a >> 16u) ^ ulong_a;

    return ulong_a;
}

HashNode::HashNode(ring_id_t id, std::size_t hash): id(id), hash(hash) {}

HashRing::HashRing(): n(4), size(0) {}

HashRing::HashRing(unsigned int servers_n)
{
    if (servers_n > MAX_SERVERS_N)
        servers_n = MAX_SERVERS_N;

    this->n = servers_n;
    this->size = 0;
}

unsigned int HashRing::_find_smallest_bigger(std::size_t hash)
{
    int l, m, r;
    std::vector<HashNode> &nodes = this->nodes;

    l = 0;
    r = nodes.size() - 1;
    m = l + ((r - l) / 2);

    while (l < r && nodes[m].hash != hash) {
        if (nodes[m].hash < hash)
            l = m + 1;
        else
            r = m;

        m = l + ((r - l) / 2);
    }

    return m;
}

void HashRing::add(Endpoint server_endpoint)
{
    ring_id_t id;
    auto &endpoints = this->endpoints;
    auto &nodes = this->nodes;

    if (this->size >= this->n)
        return;

    id = endpoint_to_ring_id(server_endpoint);

    if (endpoints.find(id) != endpoints.end())
        return;

    for (unsigned int i = 0; i < REPLICAS_N; ++i) {
        std::size_t hash = ring_label_hash(ring_label_from_ring_id(id, i));

        if (nodes.empty() || hash >= (*(--nodes.end())).hash)
            nodes.push_back(HashNode(id, hash));
        else
            nodes.insert(nodes.begin() + this->_find_smallest_bigger(hash), HashNode(id, hash));
    }

    endpoints[id] = server_endpoint;

    ++this->size;
}

void HashRing::remove(Endpoint server_endpoint)
{
    ring_id_t id;
    auto endpoints = this->endpoints;

    id = endpoint_to_ring_id(server_endpoint);

    if (endpoints.find(id) == endpoints.end())
        return;

    for (auto iter = nodes.begin(); iter != nodes.end(); ++iter)
        if ((*iter).id == id) {
            nodes.erase(iter);
            --iter;
        }

    endpoints.erase(id);

    --this->size;
}

Endpoint HashRing::distribute(Endpoint client_endpoint)
{
    unsigned int index;
    ring_id_t id;

    id = endpoint_to_ring_id(client_endpoint);

    index = this->_find_smallest_bigger(ring_label_hash(ring_label_from_ring_id(id, 0)));

    return this->endpoints[this->nodes[index].id];
}
