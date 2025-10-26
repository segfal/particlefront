#include "ModelManager.h"
#include "Model.h"
#include <filesystem>
#include "../utils.h"

ModelManager::ModelManager() {
    loadModels("src/assets/models/");
}

ModelManager::~ModelManager() {
    shutdown();
}

void ModelManager::shutdown() {
    for (auto& pair : models) {
        delete pair.second;
    }
    models.clear();
}

void ModelManager::loadModels(std::string path, std::string prevName) {
    namespace fs = std::filesystem;
    fs::path searchPath = resolvePath(path);
    std::error_code ec;
    if ((!fs::exists(searchPath, ec) || !fs::is_directory(searchPath, ec)) && prevName.empty()) {
        searchPath = resolvePath("src/assets/models/");
    }
    if (!fs::exists(searchPath, ec) || !fs::is_directory(searchPath, ec)) {
        return;
    }
    for (const auto& entry : fs::directory_iterator(searchPath)) {
        std::string name = prevName + entry.path().stem().string();
        if (entry.path().extension() == ".gltf" || entry.path().extension() == ".glb") {
            models[name] = new Model(name);
            models[name]->loadFromFile(entry.path().string());
        } else if (entry.is_directory()) {
            loadModels(entry.path().string(), name + "_");
        }
    }
}

Model* ModelManager::getModel(const std::string& name) {
    if (models.find(name) != models.end()) {
        return models[name];
    }
    return nullptr;
}