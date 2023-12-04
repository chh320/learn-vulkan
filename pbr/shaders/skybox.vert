#version 450

layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 localPos;

layout(binding = 0) uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
}ubo;

void main(){
	localPos = aPos;
	mat4 view = mat4(mat3(ubo.view));
	vec4 clips= ubo.proj * view * vec4(aPos, 1.0);
	gl_Position  = clips.xyww;
}