#include <InputManager.h>
#include <glfw/include/GLFW/glfw3.h>
#include <vector>
#include <functional>
#include <string>
#include <array>
#include <limits>
#include <utility>

void InputManager::processInput(GLFWwindow* window) {
    if (!window) return;
    std::vector<InputEvent> events;
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
        int state = glfwGetKey(window, key);
        if (state == GLFW_PRESS && keyStates[key] != GLFW_PRESS) {
            InputEvent event;
            event.type = InputEvent::Type::KeyPress;
            event.keyEvent = { key, 0, 0 };
            events.push_back(event);
        } else if (state == GLFW_RELEASE && keyStates[key] != GLFW_RELEASE) {
            InputEvent event;
            event.type = InputEvent::Type::KeyRelease;
            event.keyEvent = { key, 0, 0 };
            events.push_back(event);
        }
        keyStates[key] = state;
    }

    double xpos = std::numeric_limits<double>::quiet_NaN();
    double ypos = std::numeric_limits<double>::quiet_NaN();
    glfwGetCursorPos(window, &xpos, &ypos);
    if (!hasMousePos) {
        lastMousePos = {xpos, ypos};
        hasMousePos = true;
    }
    double deltaX = xpos - lastMousePos.x;
    double deltaY = ypos - lastMousePos.y;
    if (deltaX != 0.0 || deltaY != 0.0) {
        InputEvent event;
        event.type = InputEvent::Type::MouseMove;
        event.mouseMoveEvent = { deltaX, deltaY };
        events.push_back(event);
        lastMousePos.x = xpos;
        lastMousePos.y = ypos;
    }

    for (int button = GLFW_MOUSE_BUTTON_1; button <= GLFW_MOUSE_BUTTON_LAST; ++button) {
        int state = glfwGetMouseButton(window, button);
        if (state == GLFW_PRESS && mouseButtonStates[button] != GLFW_PRESS) {
            InputEvent event;
            event.type = InputEvent::Type::MouseButtonPress;
            event.mouseButtonEvent = { button, 0 };
            events.push_back(event);
        } else if (state == GLFW_RELEASE && mouseButtonStates[button] != GLFW_RELEASE) {
            InputEvent event;
            event.type = InputEvent::Type::MouseButtonRelease;
            event.mouseButtonEvent = { button, 0 };
            events.push_back(event);
        }
        mouseButtonStates[button] = state;
    }
    dispatch(events);
}

void InputManager::registerListener(std::function<void(const std::vector<InputEvent>&)> cb) {
    if (cb) {
        listeners.push_back(std::move(cb));
    }
}

void InputManager::resetMouseDelta() {
    hasMousePos = false;
}

void InputManager::dispatch(const std::vector<InputEvent>& events) {
for (const auto& listener : listeners) {
    if (listener) {
        listener(events);
        }
    }
}

InputManager::InputManager() {
    keyStates.fill(GLFW_RELEASE);
    mouseButtonStates.fill(GLFW_RELEASE);
    lastMousePos = {0.0, 0.0};
}