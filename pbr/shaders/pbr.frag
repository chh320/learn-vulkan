#version 450
layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 worldPos;

layout(binding = 0) uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
}ubo;

layout(binding = 1) uniform sampler2D[] texSampler;
layout(binding = 2) uniform samplerCube irradianceMap;
layout(binding = 3) uniform samplerCube prefilteredMap;
layout(binding = 4) uniform sampler2D	brdfLutMap;

layout(binding = 5) uniform ShaderVauleUBO{
	int prefilteredCubeMipLevels;
	int debugViewInputs;
	int debugViewEquation;
}svubo;


// --------------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness){
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 getNormalFromMap()
{
	vec3 tangentNormal = texture(texSampler[3], fragTexCoord).rgb * 2.0 - 1.0;

	vec3 Q1  = dFdx(worldPos);
    vec3 Q2  = dFdy(worldPos);
    vec2 st1 = dFdx(fragTexCoord);
    vec2 st2 = dFdy(fragTexCoord);

    vec3 N   = normalize(normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float float_aces(float value)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	value = (value * (a * value + b)) / (value * (c * value + d) + e);
	return clamp(value, 0, 1);
}

vec3 tonemapping(vec3 color){
	for(int i = 0; i < 3; i++){
		color[i] = float_aces(color[i]);
	}
	return color;
}

// --------------------------------------------------------------------------------
void main()
{
	vec3 albedo = pow(texture(texSampler[0], fragTexCoord).rgb, vec3(2.2));
	vec3 emission = texture(texSampler[1], fragTexCoord).rgb;
	float metallic = texture(texSampler[2], fragTexCoord).r;
	float ao = texture(texSampler[4], fragTexCoord).r;
	float roughness = texture(texSampler[5], fragTexCoord).r;

	vec3 V = normalize(ubo.cameraPos - worldPos);
	vec3 N = getNormalFromMap();
	vec3 R = normalize(reflect(-V, N));

	float NdotV = max(dot(N, V), 0.0);

	vec3 dielectric = vec3(0.04);
	vec3 F0 = mix(dielectric, albedo, metallic);
	vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
	
	vec3 ks = F;
	vec3 kd = 1.0 - ks;
	kd *= 1.0 - metallic;

	// irradiance
	vec3 irradiance = textureLod(irradianceMap, N, 1.0).rgb;
	vec3 diffuse = irradiance * albedo;

	// specular
	vec3 prefilteredColor = textureLod(prefilteredMap, R, roughness * svubo.prefilteredCubeMipLevels).rgb;
	vec3 brdf = texture(brdfLutMap, vec2(NdotV, 1.0 - roughness)).rgb;
	vec3 specular = prefilteredColor * (F0 * brdf.x + brdf.y);

	vec3 ambient = (kd * diffuse + specular) * ao;
	vec3 color = emission + ambient;

	// HDR tonemapping
	color = tonemapping(color);

	// gamma correct
	color = pow(color, vec3(1.0 / 2.2));

	outColor = vec4(color, 1.0);

	// debug -----------------------------
	if(svubo.debugViewInputs > 0.0){
		int index = svubo.debugViewInputs;
		switch(index){
			case 1:
				outColor.rgb = pow(albedo, vec3(1.0 / 2.2));
				break;
			case 2:
				outColor.rgb = N;
				break;
			case 3:
				outColor.rgb = vec3(ao);
				break;
			case 4:
				outColor.rgb = emission;
				break;
			case 5:
				outColor.rgb = vec3(metallic);
				break;
			case 6:
				outColor.rgb = vec3(roughness);
				break;
		}
	}
	if(svubo.debugViewEquation > 0.0){
		int index = svubo.debugViewEquation;
		switch(index){
			case 1:
				outColor.rgb = diffuse;
				break;
			case 2:
				outColor.rgb = F;
				break;
			case 3:
				//outColor.rgb = vec3(G);
				//break;
			case 4: 
				//outColor.rgb = vec3(D);
				//break;
			case 5:
				outColor.rgb = specular;
				break;				
		}
	}

}