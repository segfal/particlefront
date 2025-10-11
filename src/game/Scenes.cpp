#include <vector>
#include <functional>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include "../engine/UIManager.h"
#include "../engine/TextObject.h"
#include "../engine/UIObject.h"
#include "../engine/EntityManager.h"
#include "../engine/Entity.h"
#include "../engine/ModelManager.h"
#include "../engine/Camera.h"
#include "Scenes.h"

#define PI 3.14159265358979323846

void Scene0() {
    UIManager* uiMgr = UIManager::getInstance();
    TextObject* titleText = new TextObject("ParticleFront", "Lato", {0.0f, 0.0f}, {50.0f, 500.0f}, {1, 1}, "titleText", {1.0f, 1.0f, 1.0f});
    UIObject* container = new UIObject({0.0f, 0.0f}, {0.5f, 0.5f}, {1, 1}, "mainContainer", "window");
    uiMgr->addUIObject(titleText);
    EntityManager* entityManager = EntityManager::getInstance();
    Entity* exampleEntity = new Entity("cubeEntity", "pbr", {0.0f, 0.0f, 0.0f}, {0.0f, PI * 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f});
    exampleEntity->setModel(ModelManager::getInstance()->getModel("cube"));

    const glm::vec3 targetPosition = exampleEntity->getPosition();
    const glm::vec3 cameraPosition = {5.0f, 5.0f, 5.0f};
    const glm::vec3 toTarget = glm::normalize(targetPosition - cameraPosition);
    const float yaw = glm::degrees(std::atan2(toTarget.x, -toTarget.z));
    const float clampedY = glm::clamp(toTarget.y, -1.0f, 1.0f);
    const float pitch = glm::degrees(std::asin(clampedY));
    Camera* camera = new Camera(cameraPosition, {pitch, yaw, 0.0f}, 60.0f);
    entityManager->addEntity("camera", camera);
    entityManager->addEntity("cubeEntity", exampleEntity);
}

std::map<int, std::function<void()>> Scenes::sceneList = {
    {0, Scene0}
};