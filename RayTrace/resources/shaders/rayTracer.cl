
//Macros
// Ref: http://cs.lth.se/tomas_akenine-moller
#define EPSILON 0.000001
#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2]; 

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

//Ref: http://cs.lth.se/tomas_akenine-moller
int intersectTri(double orig[3], double dir[3],
	double vert0[3], double vert1[3], double vert2[3],
	double *t, double *u, double *v)
{
	double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	double det, inv_det;

	/* find vectors for two edges sharing vert0 */
	SUB(edge1, vert1, vert0);
	SUB(edge2, vert2, vert0);

	/* begin calculating determinant - also used to calculate U parameter */
	CROSS(pvec, dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DOT(edge1, pvec);

	if (det > -EPSILON && det < EPSILON)
		return 0;
	inv_det = 1.0 / det;

	/* calculate distance from vert0 to ray origin */
	SUB(tvec, orig, vert0);

	/* calculate U parameter and test bounds */
	*u = DOT(tvec, pvec) * inv_det;
	if (*u < 0.0 || *u > 1.0)
		return 0;

	/* prepare to test V parameter */
	CROSS(qvec, tvec, edge1);

	/* calculate V parameter and test bounds */
	*v = DOT(dir, qvec) * inv_det;
	if (*v < 0.0 || *u + *v > 1.0)
		return 0;

	/* calculate t, ray intersects triangle */
	*t = DOT(edge2, qvec) * inv_det;

	return 1;
}

__kernel void rayTracer(__global int* output, __global float4* sphereOrigins, __global float* sphereRadius, __global float4* rayOrigins, float4 rayDir, __global float4* cubeVertices)
{
	int result;

	struct Ray ray;
	ray.origin = rayOrigins[get_global_id(0)];
 	ray.direction = rayDir;

	struct Sphere s;
	s.origin = sphereOrigins[0];
	s.radius = sphereRadius[0];


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
