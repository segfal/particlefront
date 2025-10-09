#pragma once
#include <vector>
#include <glm/glm.hpp>

class Model;

class Entity {
public:
    Entity() = default;
    virtual ~Entity() = default;
    virtual void update(float deltaTime) = 0;
private:
    int id;
    std::vector<Entity*> children;
    Entity* parent = nullptr;
    Model* model = nullptr;
};