
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
int intersectTri(float orig[3], float dir[3],
	float vert0[3], float vert1[3], float vert2[3],
	float *t, float *u, float *v)
{
	float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	float det, inv_det;

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

float intersectSphere(float4 rayOrigin, float4 rayDir, float sphereRadius, float4 sphereOrigin)
{
	//Light Dir
	float4 L = sphereOrigin - rayOrigin;
	float tca = dot(L, rayDir);

	if (tca < 0)
	{
		return 0.0f;
	}

	float distanceSquared = dot(L, L) - tca * tca;
	float radiusSquared = sphereRadius * sphereRadius;

	if (distanceSquared > radiusSquared)
	{
		return 0.0f;
	}

	float thc = sqrt((radiusSquared) - distanceSquared);

	//Calculate sphere entry and exit distance
	float t0 = tca - thc;
	float t1 = tca + thc;

	//return distance(t0, t1);
	return t0;

	//distance = normaliseFloat(distance, s.radius + s.radius, 0.0f) * 255.0f;
}

__kernel void rayTracer(__global int* output,
	int numSpheres, __global float4* sphereOrigins, __global float* sphereRadius, __global float4* sphereColours,
	int numCubes, __global float4* cubeVertices, __global float4* cubeColours,
	__global float4* rayOrigins, float4 rayDir)
{
	float4 result = (float4)(0.0f,0.0f,0.0f,255.0f);

	struct Ray ray;
	ray.origin = rayOrigins[get_global_id(0)];
 	ray.direction = rayDir;

	const unsigned int numOfTrianglesPerCube = 12;


	float rayOriginConverted[3] = { ray.origin.x, ray.origin.y, ray.origin.z };
	float rayDirConverted[3] = { ray.direction.x, ray.direction.y, ray.direction.z };

	float tri0[3];
	float tri1[3];
	float tri2[3];

	float t = 0;
	float u = 0;
	float v = 0;

	float4 closestColour = (float4)(0.0f, 0.0f, 0.0f, 255.0f);
	float closest = 300000.0f; //Set to high number so it will always be beaten

	//Process Cubes
	for (unsigned int cubeIndex = 0; cubeIndex < numCubes; cubeIndex++)
	{
		//Due to all cubes triangles being in the same array,
		//I offset the triangle indexes by the current cube index
		unsigned int triOffset = cubeIndex * numOfTrianglesPerCube * 3;

		for (unsigned int triIndex = 0; triIndex < numOfTrianglesPerCube * 3; triIndex += 3)
		{
			tri0[0] = cubeVertices[triOffset + triIndex].x;
			tri0[1] = cubeVertices[triOffset + triIndex].y;
			tri0[2] = cubeVertices[triOffset + triIndex].z;

			tri1[0] = cubeVertices[triOffset + triIndex + 1].x;
			tri1[1] = cubeVertices[triOffset + triIndex + 1].y;
			tri1[2] = cubeVertices[triOffset + triIndex + 1].z;

			tri2[0] = cubeVertices[triOffset + triIndex + 2].x;
			tri2[1] = cubeVertices[triOffset + triIndex + 2].y;
			tri2[2] = cubeVertices[triOffset + triIndex + 2].z;

			if (intersectTri(rayOriginConverted, rayDirConverted, tri0, tri1, tri2, &t, &u, &v) == 1)
			{
				if ((float)t < closest)
				{
					closest = (float)t;
					closestColour = cubeColours[cubeIndex];
				}
			}
		}
	}

	//Process Spheres
	for (unsigned int sphereIndex = 0; sphereIndex < numSpheres; sphereIndex++)
	{
		float distance = intersectSphere(ray.origin, ray.direction, sphereRadius[sphereIndex], sphereOrigins[sphereIndex]);

		if (distance == 0.0f)
			continue;

		if (distance < closest)
		{
			closest = distance;
			closestColour = sphereColours[sphereIndex];
		}
	}

	//Check any object is closer than the default setting, if not return black colour as no intersects occurred
	if (closest == 300000.0f)
	{
		result = closestColour;
	}
	else
	{
		float colourScalar = 255.0f - (normaliseFloat(closest, 180.0f, 0.0f) * 255.0f);
		result = colourScalar * closestColour;
		result.w = 255.0f;
	}

	output[(get_global_id(0) * 4)	 ] = result.x;
	output[(get_global_id(0) * 4) + 1] = result.y;
	output[(get_global_id(0) * 4) + 2] = result.z;
	output[(get_global_id(0) * 4) + 3] = result.w;
}
