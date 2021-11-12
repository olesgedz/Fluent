#version 460

layout (local_size_x = 16, local_size_y = 16) in;

layout (set = 0, binding = 0, rgba32f) uniform image2D uOutputImage;

const float MAX_DISTANCE = 1000.0;

float SignedDistanceToCircle(vec2 point, vec2 center, float radius)
{
	return length(center - point) - radius;
}

float SignedDistanceToBox(vec2 point, vec2 center, vec2 size)
{
	vec2 offset = abs(point - center) - size;

	float unsignedDst = length(max(offset, vec2(0.0)));
	float dstInsideBox = max(min(offset.x, 0), min(offset.y, 0));

	return unsignedDst + dstInsideBox;
}

float TorusDistance(vec3 eye, vec3 center, float r1, float r2)
{   
    vec2 q = vec2(length((eye - center).xz) - r1, eye.y - center.y);
    return length(q) - r2;
}

float SignedDistanceToScene(vec3 point)
{
	float dstToScene = MAX_DISTANCE;
	dstToScene = TorusDistance(point, vec3(0.0, 0.0, -1.0), 0.5, 0.5);
	return dstToScene;
}

void main() 
{
	vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);
	ivec2 size = imageSize(uOutputImage);
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);

	vec2 drawCoord = vec2(coord) / vec2(size) - 0.5;

	vec3 color = vec3(0.0);
	if (SignedDistanceToScene(vec3(drawCoord, 0.0)) <= 0.05)
		color = vec3(1.0);

	imageStore(uOutputImage, coord, vec4(color, 1.0));
}