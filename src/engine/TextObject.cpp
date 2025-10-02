#include "TextObject.h"

TextObject::TextObject(
    std::string text,
    std::string font,
    glm::vec2 position,
    glm::vec2 size,
    glm::ivec2 corner,
    std::string name
) : text(std::move(text)), font(std::move(font)), UIObject(position, size, corner, name, "") {}
