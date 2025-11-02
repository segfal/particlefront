#version 450

layout(location = 0) in vec2 texCoord;

layout(binding = 0) uniform sampler2D lightingTex;
layout(binding = 1) uniform sampler2D ssrTex;

layout(location = 0) out vec4 FragColor;

void main() {
    vec4 lighting = texture(lightingTex, texCoord);
    vec4 ssr = texture(ssrTex, texCoord);
    
    vec3 finalColor = lighting.rgb + ssr.rgb * ssr.a;
    
    FragColor = vec4(finalColor, lighting.a);
}
