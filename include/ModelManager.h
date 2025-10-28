#pragma once
#include <map>
#include <Model.h>

class ModelManager {
public:
    ModelManager();
    ~ModelManager();

    void loadModels(std::string path, std::string prevName = "");

    static ModelManager* getInstance() {
        static ModelManager instance;
        return &instance;
    }

    void shutdown();

    Model* getModel(const std::string& name);

private:
    std::map<std::string, Model*> models;
};