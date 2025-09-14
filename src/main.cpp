#include "./engine/Renderer.h"

int main() {
    engine::Renderer* renderer = new engine::Renderer();
    renderer->run();
    delete renderer;
    return 0;
}