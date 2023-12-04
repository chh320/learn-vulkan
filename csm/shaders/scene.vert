#version 450
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

layout(location = 0) out vec3 worldFragPos;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 uv;
layout(location = 3) out vec3 viewPos;

layout(push_constant) uniform PushConsts{
    layout(offset = 0) vec3 offset;
    layout(offset = 12) int modelID;
} consts;


layout(binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} ubo;

void main(){
    mat4 model = ubo.model;
    
    if(consts.modelID == 1){
        for(int i = 0; i < 3; i++){
            model[i][i] *= 300;
        }
    }

    vec3 pos = aPos + consts.offset;
    worldFragPos = vec3(model * vec4(pos, 1.0));
    viewPos = vec3(ubo.view * model * vec4(pos, 1.0));
    uv = aUV;
    normal = normalize(aNormal);
    
    gl_Position = ubo.proj * ubo.view * model * vec4(pos, 1.0);
}