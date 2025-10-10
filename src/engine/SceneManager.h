#pragma once
#include <functional>
#include <map>
#include <string>

class SceneManager {
public:
    SceneManager();
    ~SceneManager();
    void addScene(int id, std::function<void()> func);
    void switchScene(int id);
    static SceneManager* getInstance();
    void shutdown();
private:
    int currentScene = 0;
    std::map<int, std::function<void()>> scenes;
};