#version 450

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec3 normalVec;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in mat3 TBN;

layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D metallicMap;
layout(binding = 3) uniform sampler2D roughnessMap;
layout(binding = 4) uniform sampler2D normalMap;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outMaterial;

vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, texCoord).xyz * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}

void main() {
    vec4 baseColor = texture(albedoMap, texCoord);
    vec3 albedo = pow(baseColor.rgb, vec3(2.2));
    float alpha = baseColor.a;
    
    float mask = texture(metallicMap, texCoord).a;
    if (mask < 0.2) discard;
    
    float metallic = texture(metallicMap, texCoord).r;
    float roughness = max(texture(roughnessMap, texCoord).r, 0.05);
    
    vec3 normal = getNormalFromMap();
    
    outAlbedo = vec4(albedo, alpha);
    outNormal = vec4(normal * 0.5 + 0.5, 1.0);
    outMaterial = vec4(metallic, roughness, 0.0, 1.0);
}