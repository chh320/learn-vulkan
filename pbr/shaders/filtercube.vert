#version 450

layout (location = 0) in vec3 aPos;

layout (push_constant) uniform PushConsts{
	layout (offset = 0) mat4 mvp;
} pushConsts;

layout (location = 0) out vec3 outUV;

out gl_PerVertex{
	vec4 gl_Position;
};

void main()
{
	outUV = aPos;
	gl_Position = pushConsts.mvp * vec4(aPos, 1.0);
}