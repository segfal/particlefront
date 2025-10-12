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
#include "../engine/Camera.h"
#include "Scenes.h"

#define PI 3.14159265358979323846

void MainMenu() {
    UIManager* uiMgr = UIManager::getInstance();
    TextObject* titleText = new TextObject("ParticleFront", "Lato", {0.0f, 0.0f}, {50.0f, 500.0f}, {1, 1}, "titleText", {1.0f, 1.0f, 1.0f});
    UIObject* container = new UIObject({0.0f, 0.0f}, {0.5f, 0.5f}, {1, 1}, "mainContainer", "window");
    container->addChild(titleText);
    ButtonObject* startButton = new ButtonObject({0.0f, -60.0f}, {200.0f, 50.0f}, {1, 1}, "startButton", "window", "Start Game", StartGame);
    container->addChild(startButton);
    uiMgr->addUIObject(container);
}

void Scene1() {
    EntityManager* entityMgr = EntityManager::getInstance();
    Entity* exampleEntity = new Entity("exampleCube", "pbr", {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f});
    exampleEntity->setModel(ModelManager::getInstance()->getModel("cube"));
    entityMgr->addEntity("exampleCube", exampleEntity);

    const glm::vec3 targetPosition = exampleEntity->getPosition();
    const glm::vec3 cameraPosition = {5.0f, 5.0f, 5.0f};
    const glm::vec3 toTarget = glm::normalize(targetPosition - cameraPosition);
    const float yaw = glm::degrees(std::atan2(toTarget.x, -toTarget.z));
    const float clampedY = glm::clamp(toTarget.y, -1.0f, 1.0f);
    const float pitch = glm::degrees(std::asin(clampedY));

    Entity* cameraEntity = new Camera(cameraPosition, {pitch, yaw, 0.0f}, 45.0f);
    entityMgr->addEntity("camera", cameraEntity);
}

std::map<int, std::function<void()>> Scenes::sceneList = {
    {0, MainMenu},
    {1, Scene1},
};