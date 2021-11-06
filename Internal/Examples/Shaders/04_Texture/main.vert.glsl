#version 450

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec2 iTexCoord;

layout (location = 0) out vec2 TexCoord;

layout (set = 0, binding = 0) uniform uCameraBuffer
{
	mat4 projection;
	mat4 view;
	mat4 model;
} ubo;

void main() 
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(iPosition, 1.0);
	TexCoord = iTexCoord;
}