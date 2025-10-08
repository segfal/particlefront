#include <vector>
#include <functional>
#include <iostream>
#include "../engine/UIManager.h"
#include "../engine/TextObject.h"
#include "../engine/UIObject.h"
#include "Scenes.h"

void Scene0() {
    UIManager* uiMgr = UIManager::getInstance();
    TextObject* titleText = new TextObject("ParticleFront", "Lato", {0.0f, 0.0f}, {50.0f, 500.0f}, {1, 1}, "titleText", {1.0f, 1.0f, 1.0f});
    UIObject* container = new UIObject({0.0f, 0.0f}, {0.5f, 0.5f}, {1, 1}, "mainContainer", "window");
    container->addChild(titleText);
    uiMgr->addUIObject(container);
}

std::map<int, std::function<void()>> Scenes::sceneList = {
    {0, Scene0}
};