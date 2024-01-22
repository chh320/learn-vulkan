#version 450
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

layout(location = 0) out vec2 uv;

#define SHADOW_MAP_CASCADE_COUNT 4

layout(push_constant) uniform PushConsts {
	vec3 position;
    int modelID;
	int cascadeIndex;
} pushConsts;

layout(binding = 0) uniform lightUniformBuffer{
	mat4[SHADOW_MAP_CASCADE_COUNT] lightProjView;
}lightUBO;

layout(binding = 1) uniform UniformBufferObject{
	mat4 model;
}ubo;

void main(){
    mat4 model = ubo.model;
    if(pushConsts.modelID == 1){
        for(int i = 0; i < 3; i++){
            model[i][i] *= 300;
        }
    }
    if(pushConsts.modelID == 2){
        for(int i = 0; i < 3; i++){
            model[i][i] *= 2;
        }
    }
	uv = aUV;
    vec3 pos = aPos + pushConsts.position;
	gl_Position = lightUBO.lightProjView[pushConsts.cascadeIndex] * model * vec4(pos, 1.0);
}