#include <vector>
#include <math.h>
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "lodepng.h"

#include "Ray.h"
#include "Triangle.h"

//Macros
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


void encodePNG(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height);
int intersectTri(double orig[3], double dir[3], double vert0[3], double vert1[3], double vert2[3], double *t, double *u, double *v);
int collide(Ray& ray);
int main(int argc, char * argv[]);

//Global Triangle
Triangle tri;

int main(int argc, char * argv[])
{
	//Create Tri
	tri.a = glm::vec3(150, 0, 0);
	tri.b = glm::vec3(0, 300, 0);
	tri.c = glm::vec3(300, 300, 0);

	std::vector<unsigned char> pixels;
	//Dimensions * 4 Bytes (RGBA)
	pixels.reserve((300 * 300) * 4);

	glm::mat4 proj = glm::ortho(0.0f, 300.0f, 300.0f, 0.0f, 0.0f, 1000.0f);


	for (unsigned int y = 0; y < 300; y++)
	{
		for (unsigned int x = 0; x < 300; x++)
		{
			Ray ray;
			ray.origin = glm::vec4(x, y, 0, 1);
			ray.direction = proj * glm::vec4(0, 0, 1, 1);

			int result = collide(ray);

			//if Result equal 1 set red channel to max
			pixels.push_back(result == 1 ? 255 : 0);
			pixels.push_back(0);
			pixels.push_back(0);
			pixels.push_back(255);

			//Also print to console
			//std::cout << result;
		}
		//New line for each row
		//std::cout << std::endl;
	}

	encodePNG("ray.png", pixels, 300, 300);

	return 0;
}

void encodePNG(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height)
{
	//Encode the image
	unsigned error = lodepng::encode(filename, image, width, height);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

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

//Quick function to convert params to be suitable for the intersect code
int collide(Ray& ray)
{
	double rayOrigin[3]{ ray.origin.x, ray.origin.y, ray.origin.z };
	double rayDir[3]{ ray.direction.x, ray.direction.y, ray.direction.z };

	double tri0[3]{ tri.a.x, tri.a.y, tri.a.z };
	double tri1[3]{ tri.b.x, tri.b.y, tri.b.z };
	double tri2[3]{ tri.c.x, tri.c.y, tri.c.z };

	double t = 0;
	double u = 0;
	double v = 0;

	return intersectTri(rayOrigin, rayDir, tri0, tri1, tri2, &t, &u, &v);
}