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
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2 P = viewDir.xy / viewDir.z * PushConstants.heightScale; 
    vec2 deltaTexCoords = P / numLayers;
  
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(sampler2D(uHeightMap, uSampler), currentTexCoords).r;
      
    while (currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(sampler2D(uHeightMap, uSampler), currentTexCoords).r;  
        currentLayerDepth += layerDepth;  
    }
    
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(sampler2D(uHeightMap, uSampler), prevTexCoords).r - currentLayerDepth + layerDepth;
 
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main()
{
    mat3 invTBN = transpose(CalculateTBN());

    vec3 V = invTBN * normalize(PushConstants.viewPosition - iFragmentPosition);
    vec2 texCoords = ParallaxMapping(iTexCoord, V);

    vec3 normal = texture(sampler2D(uNormalMap, uSampler), texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 albedo = texture(sampler2D(uAlbedoMap, uSampler), texCoords).rgb;
    vec3 ambient = 0.1 * albedo;
    // diffuse
    vec3 lightDir = invTBN * normalize(PushConstants.lightPosition - iFragmentPosition);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * albedo;
    // specular    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + V);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(albedo + diffuse + specular, 1.0);
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