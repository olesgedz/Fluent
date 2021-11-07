#version 450

layout(location = 0) in vec2 iTexCoord;
layout(location = 1) in vec3 iWorldPosition;
layout(location = 2) in vec3 iNormal;

layout(location = 0) out vec4 outColor;

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
vec2 ParallaxMapping(vec2 texCoords, vec3 V);

void main()
{
    mat3 TBN = CalculateTBN();

    vec3 tangentViewPosition    = TBN * PushConstants.viewPosition;
    vec3 tangentLightPosition   = TBN * PushConstants.lightPosition;
    vec3 tangentFragPosition    = TBN * iWorldPosition; 

    vec3 viewDirection = normalize(tangentViewPosition - tangentFragPosition);
    vec2 uv = ParallaxMapping(iTexCoord, viewDirection);
    if (uv.x > 1.0 || uv.y > 1.0 || uv.x < 0.0 || uv.y < 0.0)
        discard;

    vec3 normal = normalize(texture(sampler2D(uNormalMap, uSampler), iTexCoord).rgb * 2.0 - 1.0);
    vec3 albedo = texture(sampler2D(uAlbedoMap, uSampler), uv).rgb;
    vec3 ambient = 0.1 * albedo;
    vec3 lightDirection = normalize(tangentLightPosition - tangentFragPosition);
    vec3 diffuse = max(dot(lightDirection, normal), 0.0) * albedo;

    // specular    
    vec3 R = reflect(-lightDirection, normal);
    vec3 H = normalize(lightDirection + viewDirection);  
    float spec = pow(max(dot(normal, H), 0.0), 32.0);
    vec3 specular = vec3(0.2) * spec;

    vec3 color = ambient + diffuse + specular;

    outColor = vec4(color, 1.0);
}

mat3 CalculateTBN()
{
    vec3 Q1     = dFdx(iWorldPosition);
    vec3 Q2     = dFdy(iWorldPosition);
    vec2 st1    = dFdx(iTexCoord);
    vec2 st2    = dFdy(iTexCoord);

    vec3 N      = normalize(iNormal);
    vec3 T      = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B      = -normalize(cross(N, T));
    mat3 TBN    = mat3(T, B, N);

    return TBN;
}

vec2 ParallaxMapping(vec2 texCoords, vec3 V)
{
    const float minLayers = 8;
    const float maxLayers = 32;

    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), V)));  

    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;

    vec2 P = V.xy / V.z * PushConstants.heightScale; 
    vec2 deltaTexCoords = P / numLayers;
  
    vec2  currentTexCoords     = iTexCoord;
    float currentDepthMapValue = texture(sampler2D(uHeightMap, uSampler), currentTexCoords).r;
      
    while (currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(sampler2D(uHeightMap, uSampler), currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(sampler2D(uHeightMap, uSampler), prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}