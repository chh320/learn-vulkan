#version 450

layout (location = 0) in vec3 inPos;
layout (location = 0) out vec4 outColor;
layout (binding = 0) uniform sampler2D[] texSampler;

layout (push_constant) uniform PushConsts {
	layout (offset = 64) float deltaPhi;
	layout (offset = 68) float deltaTheta;
} consts;

#define PI 3.1415926535897932384626433832795

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec3 N = normalize(inPos);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float TWO_PI = PI * 2.0;
    float HALF_PI = PI * 0.5;
    
    vec3 color = vec3(0.0);
    uint sampleCount = 0u;
    for(float phi = 0.0; phi < TWO_PI; phi+= consts.deltaPhi)
    {
        for(float theta = 0.0; theta < HALF_PI; theta += consts.deltaTheta)
        {
            vec3 TBN = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleDir = TBN.x * right + TBN.y * up + TBN.z * N;

            vec2 uv = SampleSphericalMap(normalize(sampleDir));
            uv.y = 1.0 - uv.y;

            color += texture(texSampler[0], uv).rgb * cos(theta) * sin(theta);
            sampleCount++;
        }
    }
    
    outColor = vec4(PI * color / float(sampleCount), 1.0);
}