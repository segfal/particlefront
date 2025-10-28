#pragma once
#include <UIObject.h>
#include <TextObject.h>
#include <functional>

class ButtonObject : public UIObject {
public:
    ButtonObject(
        glm::vec2 position,
        glm::vec2 size,
        glm::ivec2 corner,
        std::string name,
        std::string texture,
        std::string text,
        std::function<void()> onClick
    ) : UIObject(position, size, corner, name, texture),
        text(std::move(text)),
        onClick(std::move(onClick)) {
            this->addChild(new TextObject(this->text, "Lato", {0.0f, 0.0f}, {size.x - 10.0f, size.y - 10.0f}, {1, 1}, name + "_text", {1.0f, 1.0f, 1.0f}));
        }
    std::string text;
    std::function<void()> onClick;
};