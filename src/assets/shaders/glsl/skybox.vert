#version 450

layout(location = 0) in vec3 aPos;

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
} ubo;

layout(location = 0) out vec3 vDir;

void main() {
	mat4 viewRotation = mat4(mat3(ubo.view));
	vec4 clipPos = ubo.proj * viewRotation * vec4(aPos, 1.0);
    gl_Position = vec4(clipPos.xy, clipPos.w, clipPos.w);
	vDir = aPos;
}
