#pragma once
#include <string>
#include <vector>

class Model {
public:
    Model(std::string name)
        : name(std::move(name)) {};
    ~Model() = default;
    void loadFromFile(const std::string& path);
private:
    std::string name;
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
};