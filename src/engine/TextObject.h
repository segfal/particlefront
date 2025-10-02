#pragma once

#include <string>
#include <glm/glm.hpp>
#include "UIObject.h"

class TextObject : public UIObject {
public:
    TextObject(
        std::string text,
        std::string font,
        glm::vec2 position,
        glm::vec2 size,
        glm::ivec2 corner,
        std::string name
    );

    std::string text;
    std::string font;
    glm::vec3 color = glm::vec3(1.0f);
};
