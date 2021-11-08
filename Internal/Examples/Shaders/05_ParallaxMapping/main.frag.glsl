#version 450

layout(location = 0) in vec2 iTexCoord;
layout(location = 1) in vec3 iFragmentPosition;
layout(location = 2) in vec3 iNormal;

layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 1) uniform texture2D uAlbedoMap;
layout(set = 0, binding = 2) uniform texture2D uNormalMap;
layout(set = 0, binding = 3) uniform texture2D uHeightMap;
layout(set = 0, binding = 4) uniform sampler uSampler; 

layout (push_constant) uniform constants
{
	vec3    lightPosition;
	vec3    viewPosition;
    float   heightScale;
} PushConstants;

mat3 CalculateTBN();

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float height = texture(sampler2D(uHeightMap, uSampler), iTexCoord).r;     
    return texCoords - viewDir.xy * (height * PushConstants.heightScale);
}

void main()
{
    mat3 TBN = CalculateTBN();
   
    vec3 tangentView = normalize(TBN * PushConstants.viewPosition - TBN * iFragmentPosition);
    vec3 tangentLightPosition = TBN * PushConstants.lightPosition;
    vec3 tangentFragmentPosition = TBN * iFragmentPosition;

    vec2 uv = iTexCoord;
    uv = ParallaxMapping(iTexCoord, tangentView);

    if (uv.x > 1.0 || uv.y > 1.0 || uv.x < 0.0 || uv.y < 0.0)
        discard;

    // obtain normal from normal map
    vec3 normal = texture(sampler2D(uNormalMap, uSampler), iTexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);   
   
    vec3 color = texture(sampler2D(uAlbedoMap, uSampler), uv).rgb;
    vec3 ambient = 0.1 * color;

    vec3 lightDir = normalize(tangentLightPosition - tangentFragmentPosition);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + tangentView);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;

    FragColor = vec4(diffuse + specular + ambient, 1.0);
}

mat3 CalculateTBN()
{
    vec3 Q1     = dFdx(iFragmentPosition);
    vec3 Q2     = dFdy(iFragmentPosition);
    vec2 st1    = dFdx(iTexCoord);
    vec2 st2    = dFdy(iTexCoord);

    vec3 N      = normalize(iNormal);
    vec3 T      = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B      = -normalize(cross(N, T));
    mat3 TBN    = mat3(T, B, N);

    return TBN;
}