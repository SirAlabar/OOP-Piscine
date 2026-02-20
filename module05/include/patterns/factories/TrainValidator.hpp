#ifndef TRAINVALIDATOR_HPP
#define TRAINVALIDATOR_HPP

#include <string>

struct TrainConfig;
class Graph;

// Result of a train configuration validation.
struct ValidationResult
{
    bool        valid = false;
    std::string error;
};

// Single authoritative validator for TrainConfig against a network graph.
// Used by TrainFactory (creation gate) and Application (error reporting).
class TrainValidator
{
public:
    static ValidationResult validate(const TrainConfig& config, const Graph* network);
};

#endif