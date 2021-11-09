#version 460

layout (local_size_x = 16, local_size_y = 16) in;

layout (set = 0, binding = 0, rgba32f) uniform image2D uOutputImage;

int maxIterations = 100;

float Map(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

vec2 SquareImaginary(vec2 number)
{
	return vec2(
        pow(number.x, 2) - pow(number.y, 2),
		2 * number.x * number.y
	);
}

vec3 IterateJulia(vec2 coord)
{
	vec2 z = coord;
	for(int i = 0; i < maxIterations; i++)
	{
		z = SquareImaginary(z);
		z.x = z.x - 0.4;
		z.y = z.y - 0.6;

		if (length(z) > 2)
		{
			float color = 1.0 - float(i) / maxIterations;
			return vec3(color);
		}
	}

	return vec3(0.0);
}

void main() 
{
	vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);
	ivec2 size = imageSize(uOutputImage);
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);

	vec2 drawCoord = vec2(coord.x, coord.y);
	drawCoord /= size;
	drawCoord -= vec2(0.5);
	drawCoord = vec2(Map(drawCoord.x, -0.5, 0.5, -1.5, 1.5), Map(drawCoord.y, -0.5, 0.5, -1.5, 1.5));

	vec3 c = IterateJulia(drawCoord);
	pixel = vec4(c, 1.0);

	imageStore(uOutputImage, coord, pixel);
}