#pragma once
#include <UIObject.h>

class TextObject : public UIObject {
public:
    TextObject(
        std::string text,
        std::string font,
        glm::vec2 position,
        glm::vec2 size,
        glm::ivec2 corner,
        std::string name,
        glm::vec3 color
    ) : UIObject(position, size, corner, name, ""),
        text(std::move(text)),
        font(std::move(font)),
        color(color) {}
    std::string text;
    std::string font;
    glm::vec3 color;
};
