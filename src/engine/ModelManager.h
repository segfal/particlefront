#pragma once
#include <map>
#include "Model.h"

class ModelManager {
public:
    ModelManager();
    ~ModelManager();

    void loadModels(std::string path, std::string prevName = "");
    Model* getModel(const std::string& name) {
        if (models.find(name) != models.end()) {
            return models[name];
        }
        return nullptr;
    }

    static ModelManager* getInstance() {
        static ModelManager instance;
        return &instance;
    }

    void shutdown() {
        for (auto& pair : models) {
            delete pair.second;
        }
        models.clear();
    }

    Model* getModel(const std::string& name);

private:
    std::map<std::string, Model*> models;
};