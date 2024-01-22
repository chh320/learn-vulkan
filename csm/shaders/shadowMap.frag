#version 450
layout(location = 0) in vec2 uv;

layout(binding = 2) uniform sampler2D[] tex;

layout(push_constant) uniform PushConsts{
    layout(offset = 12) int modelID;
} consts;

void main(){
    if(consts.modelID == 4){
        float alpha = texture(tex[1], uv).r;
        if(alpha < 0.5) discard;
    }
}