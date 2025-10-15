struct InputEvent {
    enum class Type {
        KeyPress,
        KeyRelease,
        MouseMove,
        MouseButtonPress,
        MouseButtonRelease,
        Scroll
    } type;

    union {
        struct { int key; int scancode; int mods; } keyEvent;
        struct { double x; double y; } mouseMoveEvent;
        struct { int button; int mods; } mouseButtonEvent;
        struct { double xOffset; double yOffset; } scrollEvent;
    };
};