#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} ubo;

layout(location = 0) out vec3 FragPos;
layout(location = 1) out vec3 normalVec;
layout(location = 2) out vec2 texCoord;
layout(location = 3) out mat3 TBN;

void main() {
    vec4 worldPos = ubo.model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    
    vec3 T = normalize(mat3(ubo.model) * aTangent);
    vec3 N = normalize(mat3(ubo.model) * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    
    normalVec = N;
    texCoord = aTexCoord;
    
    gl_Position = ubo.proj * ubo.view * worldPos;
}
