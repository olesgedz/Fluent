#version 450

layout(location = 0) in vec3 iNormal;
layout(location = 1) in vec2 iTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform texture2D uTexture;
layout(set = 0, binding = 2) uniform sampler uSampler;

void main()
{
    outColor = texture(sampler2D(uTexture, uSampler), iTexCoord);
}