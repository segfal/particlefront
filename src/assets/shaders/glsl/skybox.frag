#version 450

layout(location = 0) in vec3 vDir;
layout(binding = 1) uniform samplerCube skyboxTex;
layout(location = 0) out vec4 FragColor;

const float kExposure = 0.2;
const float kContrast = 5.0;
const float kGamma = 2.2;
const vec3 kLuminanceWeights = vec3(0.2126, 0.7152, 0.0722);

vec3 toneMapReinhard(vec3 hdrColor) {
    float luminance = dot(hdrColor, kLuminanceWeights);
    if (luminance <= 1e-6) {
        return hdrColor;
    }
    float mappedLum = luminance / (1.0 + luminance);
    return hdrColor * (mappedLum / luminance);
}

void main() {
    vec3 dir = normalize(vDir);
    vec3 hdr = max(texture(skyboxTex, dir).rgb * kExposure, vec3(0.0));
    vec3 mapped = toneMapReinhard(hdr);
    mapped = clamp(mapped, 0.0, 1.0);
    mapped = pow(mapped, vec3(max(kContrast, 1e-3)));
    mapped = pow(mapped, vec3(1.0 / kGamma));
    FragColor = vec4(mapped, 1.0);
}


