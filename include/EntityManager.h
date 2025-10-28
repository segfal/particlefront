#pragma once
#include <map>
#include <string>
#include <Entity.h>

class EntityManager {
public:
    EntityManager() = default;
    ~EntityManager() {
        shutdown();
    }

    std::map<std::string, Entity*>& getAllEntities() { return entities; }

    void addEntity(const std::string& name, Entity* entity) {
        entities[name] = entity;
    }

    Entity* getEntity(const std::string& name) {
        if (entities.find(name) != entities.end()) {
            return entities[name];
        }
        return nullptr;
    }

    void removeEntity(const std::string& name) {
        if (entities.find(name) == entities.end()) return;
        delete entities[name];
        entities.erase(name);
    }

    void updateAll(float deltaTime) {
        for (auto& pair : entities) {
            pair.second->update(deltaTime);
        }
    }

    void shutdown() {
        for (auto& [name, entity] : entities) {
            delete entity;
        }
        entities.clear();
    }

    static EntityManager* getInstance() {
        static EntityManager instance;
        return &instance;
    }

private:
    std::map<std::string, Entity*> entities;
};