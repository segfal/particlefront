#version 450
layout (location = 0) in vec2 fragTexCoord;
layout(binding = 0) uniform sampler2D fontTexture;
layout(push_constant) uniform PushConstants {
    vec3 textColor;
    uint isUI;
    mat4 model;
    uint[3] padding;
} pc;
layout (location = 0) out vec4 outColor;

void main(){
    if(pc.isUI==1) {
        outColor = texture(fontTexture, fragTexCoord);
    } else {
        vec2 glyphUV = vec2(fragTexCoord.x, 1.0 - fragTexCoord.y);
        float alpha = texture(fontTexture, glyphUV).r;
        outColor = vec4(pc.textColor, alpha);
        if(alpha < 0.01) discard;
    }
}