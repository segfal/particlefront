#version 450
layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTexCoords;
layout (location = 0) out vec2 fragTexCoord;

layout(push_constant) uniform PushConstants {
    vec3 textColor;
    uint isUI;
    mat4 model;
    uint[3] padding;
} pc;

void main(){
    fragTexCoord = inTexCoords;
    gl_Position = pc.model * vec4(inPosition, 0.0, 1.0);
}