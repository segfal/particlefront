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
    
private:
    std::map<std::string, Model*> models;
};