
struct Ray
{
	float4 origin;
	float4 direction;
};

struct Sphere
{
	float4 origin;
	int radius;
};

float normaliseFloat(float numberToNormalise, float max, float min)
{
	//normalise the number between zero and one
	float normalisedNumber = (numberToNormalise - min) / (max - min);

	//return the normalised number
	return normalisedNumber;
}

__kernel void rayTracer(__global int* output, __global float4* sphereOrigins, __global float* sphereRadius, __global float4* rayOrigins, float4 rayDir)
{
	//dst[get_global_id(0)] = buffer1[get_global_id(0)] + buffer2[get_global_id(0)];

	//float diff = max(dot(normal, lightDir), 0.0); //To calculate different light source

	//ray Ray struct
	//array of Sphere
	//projMat[4x4]

	//Output array of ints

	int result;

	struct Ray ray;
	ray.origin = rayOrigins[get_global_id(0)];
 	ray.direction = rayDir;

	struct Sphere s;
	s.origin = sphereOrigins[0];
	s.radius = sphereRadius[0];
	//s.origin = (float4)(50.0f, 50.0f, 0.0f, 1.0f);
	//s.radius = 50;


	//LightDir
	float4 L = s.origin - ray.origin;

	//Angle between the ray dir and light dir
	float tca = dot(L, ray.direction);

	if (tca < 0)
	{
		result = 0;
		return;
	}

	float radiusSquared = s.radius * s.radius;

	float distanceSquared = dot(L, L) - tca * tca;
	if (distanceSquared > radiusSquared)
	{
		result = 0;
		return;
	}

	float thc = sqrt(radiusSquared - distanceSquared);
	float t0 = tca - thc;
	float t1 = tca + thc;

	float distanceBetween = distance(t0, t1);



	distanceBetween = normaliseFloat(distanceBetween, s.radius + s.radius, 0.0f) * 255.0f;

	result = (int)(distanceBetween);

	if (result < 0)
		result = 0;

	output[(get_global_id(0) * 4)] = result;
	output[(get_global_id(0) * 4) + 1] = 0;
	output[(get_global_id(0) * 4) + 2] = 0;
	output[(get_global_id(0) * 4) + 3] = 255;
}
