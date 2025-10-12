#pragma once
#include <iostream>
#include "../../engine/SceneManager.h"

void StartGame() {
    // Logic to start the game
    std::cout<<"Start Game button clicked!"<<std::endl;
    SceneManager::getInstance()->switchScene(1);
}