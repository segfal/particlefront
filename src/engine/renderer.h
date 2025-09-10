#ifndef PARTICLEFRONT_ENGINE_RENDERER_H
#define PARTICLEFRONT_ENGINE_RENDERER_H

#include <cstdint>
#include <memory>
#include <glm/glm.hpp>

namespace engine {

class Mesh;
class Shader;
class Texture;
class Camera;

class Renderer {
public:
    struct InitInfo {
        int width = 1280;
        int height = 720;
        bool vsync = true;
        bool debugContext = false;
    };
    static Renderer& Get();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    Renderer();
    ~Renderer();

    void run();
};
}

#endif