#pragma once

#include <string>
#include <map>

class UIObject;

class UIManager {
private:
    std::map<std::string, UIObject*> uiObjects;

public:
    UIManager() = default;
    ~UIManager();

    void loadTextures();
    void addUIObject(UIObject* obj);
    void removeUIObject(UIObject* obj);
    UIObject* getUIObject(const std::string& name);
    std::map<std::string, UIObject*>& getUIObjects();
    void clear();

    static UIManager* getInstance();
    static unsigned int getUIObjectCount();
};
