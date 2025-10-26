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
#include "../engine/Collider.h"
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
    ModelManager* modelMgr = ModelManager::getInstance();
    EntityManager* entityMgr = EntityManager::getInstance();
    Entity* exampleEntity = new Entity("exampleCube", "pbr", {0.0f, 1.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {"materials_metal_albedo", "materials_metal_metallic", "materials_metal_roughness", "materials_metal_normal"});
    exampleEntity->setModel(modelMgr->getModel("cube"));
    OBBCollider* box1 = new OBBCollider({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, exampleEntity->getName(), {1.0f, 1.0f, 1.0f});
    exampleEntity->addChild(box1);
    entityMgr->addEntity("exampleCube", exampleEntity);
    Entity* exampleEntity2 = new Entity("exampleCube2", "pbr", {0.0f, 1.5f, -10.0f}, {0.0f, 45.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {"materials_metal_albedo", "materials_metal_metallic", "materials_metal_roughness", "materials_metal_normal"});
    exampleEntity2->setModel(modelMgr->getModel("cube"));
    OBBCollider* box2 = new OBBCollider({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, exampleEntity2->getName(), {1.0f, 1.0f, 1.0f});
    exampleEntity2->addChild(box2);
    entityMgr->addEntity("exampleCube2", exampleEntity2);

    Entity* floor = new Entity("floor", "pbr", {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {"materials_ground_albedo", "materials_ground_metallic", "materials_ground_roughness", "materials_ground_normal"});
    floor->setModel(ModelManager::getInstance()->getModel("ground"));
    ConvexCollider* floorBox = new ConvexCollider({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, floor->getName());
    floorBox->setVerticesInterleaved(modelMgr->getModel("ground-collider")->getVertices(), 8, 0, modelMgr->getModel("ground-collider")->getIndices(), {0.0f, 0.0f, 0.0f});
    floor->addChild(floorBox);
    entityMgr->addEntity("floor", floor);

    Entity* walls = new Entity("walls", "pbr", {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.2f, 1.2f, 1.2f}, {"materials_walls_albedo", "materials_walls_metallic", "materials_walls_roughness", "materials_walls_normal"});
    walls->setModel(ModelManager::getInstance()->getModel("walls"));
    entityMgr->addEntity("walls", walls);

    Player* player = new Player({0.0f, 10.0f, 0.0f}, {0.0f, 0.0f, 0.0f});

    entityMgr->addEntity("player", player);
}

std::map<int, std::function<void()>> Scenes::sceneList = {
    {0, MainMenu},
    {1, Scene1},
};