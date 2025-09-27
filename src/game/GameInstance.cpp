#include "../engine/UIManager.cpp"
#include "../engine/UIObject.cpp"
#include "../engine/TextObject.cpp"
#include "../engine/FontManager.cpp"
#include "../engine/Renderer.cpp"

class GameInstance {
private:
    UIManager* uiManager;
    FontManager* fontManager;
    Renderer* renderer;
public:
    GameInstance() {
        renderer = Renderer::getInstance();
    }
    ~GameInstance() {
        delete uiManager;
        delete fontManager;
    }

    void setupUI() {
        uiManager = new UIManager();
        fontManager = new FontManager();
        fontManager->loadFont("../assets/fonts/Lato.ttf", "Lato", 48);
        TextObject* titleText = new TextObject("ParticleFront", "Lato", {0.5f, 0.1f}, {5.0f, 1.0f}, {0, 0}, "titleText");
        UIObject* container = new UIObject({0.1f, 0.1f}, {0.8f, 0.8f}, {0, 0}, "mainContainer", "");
        container->addChild(titleText);
        uiManager->addUIObject(container);
        renderer->overwriteUIManager(uiManager);
    }
};