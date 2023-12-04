#version 450

#define SHADOW_MAP_CASCADE_COUNT 4
#define ambient 0.3

layout(location = 0) in vec3 worldFragPos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 viewPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D[] tex;
layout(binding = 2) uniform sampler2DArray shadowMap;

layout(binding = 3) uniform shadowUBO{
    vec4 splitDepth;
    mat4 lightViewProj[SHADOW_MAP_CASCADE_COUNT];
    vec3 lightDir;
} ubo;

layout(push_constant) uniform PushConsts{
    layout(offset = 12) int modelID;
    layout(offset = 16) int colorCascades;
} consts;


const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);


float Shadow(vec4 shadowCoord, vec2 offset, int cascadedID){
    float shadow = 1.0;
    float bias = 0.0005;

    if(shadowCoord.z > -1.0 && shadowCoord.z < 1.0){
        float dist = texture(shadowMap, vec3(shadowCoord.xy + offset, cascadedID)).r;
        if(shadowCoord.w > 0 && dist < shadowCoord.z - bias){
            shadow = ambient;
        }
    }
    return shadow;
}

float PCF(vec4 shadowCoord, int cascadedID){
    vec2 texDim = textureSize(shadowMap, 0).xy;
    float scale = 0.75;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0f;
    int count = 0;
    int range = 1;
    for(int x = -range; x <= range; x++){
        for(int y = -range; y <= range; y++){
            shadowFactor += Shadow(shadowCoord, vec2(x * dx, y * dy), cascadedID);
            count++;
        }
    }
    return 1.0 * shadowFactor / count;
}

void main(){
    vec4 color;
    switch(consts.modelID){
        case 1:
            color = vec4(0.8, 0.7, 0.6, 1.0);    
            break;
        case 2:
            color = texture(tex[2], uv);
            break;
        case 3:
            float alpha = texture(tex[1], uv).r;
            if(alpha < 0.5) discard;
            color = texture(tex[0], uv);
            break;
    }

    int cascadedID = 0;
    for(int i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; i++){
        if(viewPos.z < ubo.splitDepth[i])
            cascadedID = i + 1;
    }

    vec4 shadowCoord = biasMat * ubo.lightViewProj[cascadedID] * vec4(worldFragPos, 1.0);
    float shadow = PCF(shadowCoord / shadowCoord.w, cascadedID);


    vec3 N = normalize(normal);
	vec3 L = normalize(-ubo.lightDir);
	vec3 H = normalize(L + viewPos);
	float diffuse = max(dot(N, L), ambient);
	vec3 lightColor = vec3(1.0);

    vec3 R = normalize(reflect(-L, N));
    vec3 h2 = normalize(R + viewPos);
    float spec = pow(max(dot(h2, N), 0.0), 64.0);
    vec3 specular = spec * lightColor;

	outColor.rgb = max(lightColor * ((diffuse + specular) * color.rgb), vec3(0.0));
	outColor.rgb *= shadow;
	outColor.a = color.a;

    if(consts.colorCascades == 1){
        switch(cascadedID) {
			case 0 : 
				outColor.rgb *= vec3(1.0f, 0.25f, 0.25f);
				break;
			case 1 : 
				outColor.rgb *= vec3(0.25f, 1.0f, 0.25f);
				break;
			case 2 : 
				outColor.rgb *= vec3(0.25f, 0.25f, 1.0f);
				break;
			case 3 : 
				outColor.rgb *= vec3(1.0f, 1.0f, 0.25f);
				break;
		}

    }

}