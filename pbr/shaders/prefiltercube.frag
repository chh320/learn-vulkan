#version 450

layout (location = 0) in vec3 inPos;
layout (location = 0) out vec4 outColor;
layout (binding = 0) uniform sampler2D[] texSampler;

layout (push_constant) uniform PushConsts {
	layout (offset = 64) float roughness;
	layout (offset = 68) uint numSamples;
} consts;

#define PI 3.1415926535897932384626433832795

// https://learnopengl-cn.github.io/07%20PBR/03%20IBL/02%20Specular%20IBL/
float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}  

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

// from vector to uv
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

    vec3 R = N;
    vec3 V = R;

    float totalWeight = 0.0;
    vec3 prefilterColor = vec3(0.0);

    for(uint i = 0; i < consts.numSamples; i++){
        vec2 Xi = Hammersley(i, consts.numSamples);
        vec3 H = ImportanceSampleGGX(Xi, N, consts.roughness);
        vec3 L = normalize(2.0 * dot(H, V) * H - V);
        
        float NdotL = max(dot(N, L), 0.0);

        if(NdotL > 0.0)
        {
            vec2 uv = SampleSphericalMap(L);
            uv.y = 1.0 - uv.y;
            prefilterColor += texture(texSampler[0], uv).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilterColor = prefilterColor / totalWeight;
    

	outColor = vec4(prefilterColor, 1.0);

}