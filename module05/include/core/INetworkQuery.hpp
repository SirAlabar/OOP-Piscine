#ifndef INETWORKQUERY_HPP
#define INETWORKQUERY_HPP

#include <vector>

class Node;
class Rail;

// Narrow read-only interface that EventFactory needs from the network.
// Graph implements this; EventFactory depends only on this interface,
class INetworkQuery
{
public:
    virtual ~INetworkQuery() = default;

    virtual std::vector<Node*> getNodes() const = 0;
    virtual std::vector<Rail*> getRails() const = 0;
};

#endif