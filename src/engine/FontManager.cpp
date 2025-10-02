#include "FontManager.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include <freetype/include/ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <vector>

FontManager::FontManager() {
    renderer = Renderer::getInstance();
}
FontManager::~FontManager() = default;

void FontManager::loadFont(const std::string& fontPath, const std::string& fontName, int fontSize) {
    FT_Library ft;
    if(FT_Init_FreeType(&ft)) {
        throw std::runtime_error("could not init FreeType library");
    }
    FT_Face face;
    if(FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        throw std::runtime_error("failed to load font: " + fontPath);
    }
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    FT_ULong codepoints[128];
    for(FT_ULong c = 0; c < 128; c++) codepoints[c] = c;
    for(FT_ULong c : codepoints) {
        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr<<"failed to load glyph: "<<(char)c<<std::endl;
            continue;
        }
        Character character = {
            .size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            .bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            .advance = face->glyph->advance.x,
        };
        if(face->glyph->bitmap.width == 0 || face->glyph->bitmap.rows == 0 || face->glyph->bitmap.buffer == nullptr) {
            unsigned char dummyPixel = 0;
            renderer->createTextureImage(1, 1, &dummyPixel, character.texture.image, character.texture.imageMemory, VK_FORMAT_R8_UNORM);
        } else renderer->createTextureImage(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer, character.texture.image, character.texture.imageMemory, VK_FORMAT_R8_UNORM);
        renderer->createTextureImageView(VK_FORMAT_R8_UNORM, character.texture.image, character.texture.imageView);
        Shader* uiShader = renderer->getShaderManager()->getShader("ui");
        Image* imgPtr = &character.texture;
        std::vector<Image*> textures = {imgPtr};
        std::vector<VkBuffer> uniformBuffers;
        character.descriptorSets = renderer->createDescriptorSets(
            uiShader->descriptorPool,
            uiShader->descriptorSetLayout,
            uiShader->vertexBitBindings,
            uiShader->fragmentBitBindings,
            textures,
            uniformBuffers
        );
        fonts[fontName].characters.insert(std::pair<char, Character>(c, character));
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}
Font* FontManager::getFont(const std::string& fontName) {
    auto it = fonts.find(fontName);
    if(it != fonts.end()) {
        return &it->second;
    }
    return nullptr;
}

FontManager* FontManager::getInstance() {
    static FontManager instance;
    return &instance;
}