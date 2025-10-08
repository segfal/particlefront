#pragma once
#include <map>
#include <functional>
#include <iostream>

class Scenes {
public:
    static std::map<int, std::function<void()>> sceneList;
};