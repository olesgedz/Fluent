#version 450

layout(location = 0) in vec2 iTexCoord;
layout(location = 1) in vec3 iFragmentPosition;
layout(location = 2) in vec3 iNormal;

layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 1) uniform sampler uSampler;
layout(set = 0, binding = 2) uniform texture2D uTextures[100];

void main()
{
    vec2 coord = iTexCoord;
    coord.y = 1.0 - coord.y;
    FragColor = texture(sampler2D(uTextures[0], uSampler), coord);
}