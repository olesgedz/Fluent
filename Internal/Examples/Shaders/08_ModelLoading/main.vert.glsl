#version 450

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec2 iTexCoord;
layout (location = 3) in vec3 iTangent;
layout (location = 4) in vec3 iBitangent;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) out vec3 FragmentPosition;
layout (location = 2) out vec3 Normal;

layout (set = 0, binding = 0) uniform uCameraBuffer
{
	mat4 projection;
	mat4 view;
} ubo;

layout (push_constant) uniform constants
{
	mat4 model;
} PushConstants;

void main() 
{
	FragmentPosition = vec3(PushConstants.model * vec4(iPosition, 1.0));
    Normal = iNormal;
	TexCoord = iTexCoord;
    gl_Position = ubo.projection * ubo.view * PushConstants.model * vec4(iPosition, 1.0);
}