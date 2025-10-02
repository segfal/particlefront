#pragma once

#include <string>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "Image.h"
#include "UIManager.h"

class Renderer;

struct Character {
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
    Image texture;
    std::vector<VkDescriptorSet> descriptorSets;
};

struct Font {
    std::string fontName;
    std::map<char, Character> characters;
};

class FontManager : public UIManager {
public:
    FontManager();
    ~FontManager();

    void loadFont(const std::string& fontPath, const std::string& fontName, int fontSize);
    Font* getFont(const std::string& fontName);

    static FontManager* getInstance();

private:
    std::map<std::string, Font> fonts;
    Renderer* renderer;
};
