#pragma once
#include "UIObject.cpp"

class TextObject : public UIObject {
public:
    TextObject(
        std::string text,
        std::string font,
        glm::vec2 position,
        glm::vec2 size,
        glm::ivec2 corner,
        std::string name
    ) : text(text), font(font), UIObject(position, size, corner, name, "") {}

private:
    std::string text;
    std::string font;
};
