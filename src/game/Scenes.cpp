#include <vector>
#include <functional>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include "../engine/UIManager.h"
#include "../engine/TextObject.h"
#include "../engine/ButtonObject.h"
#include "MainMenu/MainMenuFunctions.h"
#include "../engine/UIObject.h"
#include "../engine/EntityManager.h"
#include "../engine/Entity.h"
#include "../engine/ModelManager.h"
#include "Prefabs/Player.h"
#include "../engine/Camera.h"
#include "../engine/Renderer.h"
#include "Scenes.h"

#define PI 3.14159265358979323846

void MainMenu() {
    Renderer::getInstance()->setUIMode(true);
    UIManager* uiMgr = UIManager::getInstance();
    TextObject* titleText = new TextObject("ParticleFront", "Lato", {0.0f, 0.0f}, {50.0f, 500.0f}, {1, 1}, "titleText", {1.0f, 1.0f, 1.0f});
    UIObject* container = new UIObject({0.0f, 0.0f}, {0.5f, 0.5f}, {1, 1}, "mainContainer", "window");
    container->addChild(titleText);
    ButtonObject* startButton = new ButtonObject({0.0f, -60.0f}, {200.0f, 50.0f}, {1, 1}, "startButton", "window", "Start Game", StartGame);
    container->addChild(startButton);
    uiMgr->addUIObject(container);
}

void Scene1() {
    Renderer::getInstance()->setUIMode(false);
    EntityManager* entityMgr = EntityManager::getInstance();
    Entity* exampleEntity = new Entity("exampleCube", "pbr", {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f});
    exampleEntity->setModel(ModelManager::getInstance()->getModel("cube"));
    entityMgr->addEntity("exampleCube", exampleEntity);
    Entity* exampleEntity2 = new Entity("exampleCube2", "pbr", {0.0f, 0.0f, -10.0f}, {0.0f, 45.0f, 0.0f});
    exampleEntity2->setModel(ModelManager::getInstance()->getModel("cube"));
    entityMgr->addEntity("exampleCube2", exampleEntity2);


    Player* player = new Player({0.0f, 1.0f, -5.0f}, {0.0f, 0.0f, 0.0f});
    player->setModel(ModelManager::getInstance()->getModel("cube"));

    entityMgr->addEntity("player", player);
}

std::map<int, std::function<void()>> Scenes::sceneList = {
    {0, MainMenu},
    {1, Scene1},
};