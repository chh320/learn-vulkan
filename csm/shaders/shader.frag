#version 450
layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;

layout(binding = 0) uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
}ubo;

layout(binding = 1) uniform sampler2D[] texSampler;

void main(){

	outColor = vec4(1.0);
}