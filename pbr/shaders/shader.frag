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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness){
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 lerp(vec3 a, vec3 b, float t){
	return a + (b - a) * t;
}

void main(){

	vec3 albedo = texture(texSampler[0], fragTexCoord).rgb;
	vec3 emission = texture(texSampler[1], fragTexCoord).rgb;
	float metalness = texture(texSampler[2], fragTexCoord).r;
	vec3 normal = texture(texSampler[3], fragTexCoord).rgb;
	float ao = texture(texSampler[4], fragTexCoord).r;
	float roughness = texture(texSampler[5], fragTexCoord).r;

	vec3 V = normalize(ubo.cameraPos - fragPos);
	vec3 N = normalize(normal);
	
	float NdotV = dot(N, V);

	vec3 dielectric = vec3(0.04);
	vec3 F0 = lerp(dielectric, albedo, metalness);
	vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);

	vec3 ks = F;
	vec3 kd = 1.0 - ks;
	// irradiance
	vec3 irradiance = vec3(0.9);
	vec3 diffuse = irradiance * albedo;

	// specular
	vec3 specular = vec3(0.1);

	vec3 ambient = (kd * diffuse + specular) * ao;
	vec3 color = emission + ambient;

	outColor = vec4(color, 1.0);

	//outColor = vec4(fragTexCoord, 0.0, 1.0);
}