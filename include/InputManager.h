#pragma once
#include <InputEvent.h>
#include <glfw/include/GLFW/glfw3.h>
#include <vector>
#include <functional>
#include <string>
#include <array>
#include <limits>
#include <utility>

class InputManager {
public:
    static InputManager* getInstance() {
        static InputManager instance;
        return &instance;
    }

    void processInput(GLFWwindow* window);

    void registerListener(std::function<void(const std::vector<InputEvent>&)> cb);

    void resetMouseDelta();

    void dispatch(const std::vector<InputEvent>& events);
private:
    InputManager();
    ~InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    std::vector<std::function<void(const std::vector<InputEvent>&)>> listeners;
    std::array<int, GLFW_KEY_LAST + 1> keyStates{};
    std::array<int, GLFW_MOUSE_BUTTON_LAST + 1> mouseButtonStates{};
    struct MousePos { double x; double y; } lastMousePos{};
    bool hasMousePos = false;
};