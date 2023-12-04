#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 worldPos;

layout(binding = 0) uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
}ubo;

void main(){
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

	worldPos = vec3(ubo.model * vec4(inPosition, 1.0));
	normal = normalize(transpose(inverse(mat3(ubo.model))) * inNormal);
	fragTexCoord = inTexCoord;
}