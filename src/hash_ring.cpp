#include <iostream>

#include <vector>
#include <unordered_map>

#include <hash_ring.hpp>

#define MAX_SERVERS_N 1024
#define REPLICA_N 2


std::size_t id_hash(unsigned int id)
{
    std::size_t ulong_a = id;

	ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
	ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
	ulong_a = (ulong_a >> 16u) ^ ulong_a;

	return ulong_a;
}

HashRing::HashRing(unsigned int servers_n)
{
    if (servers_n > MAX_SERVERS_N)
        servers_n = MAX_SERVERS_N;

    this->n = servers_n;
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
    unsigned int id;
    auto &ids = this->ids;
    auto &endpoints = this->endpoints;
    auto &nodes = this->nodes;

    if (this->size >= this->n || ids.find(server_endpoint) != ids.end())
        return;

    for (unsigned int i = 0; i < this->n; ++i)
        if (endpoints.find(i) == endpoints.end()) {
            id = i;

            break;
        }

    for (unsigned int i = 0; i < REPLICA_N; ++i) {
        std::size_t hash = id_hash(id + i * this->n);

        if (nodes.empty() || hash >= (*(--nodes.end())).hash)
            nodes.push_back(HashNode(id, hash));
        else
            nodes.insert(nodes.begin() + this->_find_smallest_bigger(hash), HashNode(id, hash));
    }

    ids[server_endpoint] = id;
    endpoints[id] = server_endpoint;
}

void HashRing::remove(Endpoint server_endpoint)
{
    unsigned int id;

    if (ids.find(server_endpoint) == ids.end())
        return;

    id = ids[server_endpoint];

    ids.erase(server_endpoint);
    endpoints.erase(id);

    for (auto iter = this->nodes.begin(); iter != this->nodes.end(); ++iter)
        if ((*iter).id == id) {
            this->nodes.erase(iter);
            --iter;
        }
}

Endpoint HashRing::distribute(Endpoint client_endpoint)
{

}

void HashRing::print_ring()
{
    for (auto node : nodes)
        std::cout << node.id << ' ' << node.hash << '\n';
}
