#include <vector>
#include <string>
#include <iostream>
#include <chrono>

#include "bitmap.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>

using glm::vec3;

static const vec3 bgColor(0.1, 0.17, 0.3), ambientColor(0.2, 0.2, 0.2);

static const vec3 lightPos(20, 20, -5);

static const int MAX_DEPTH = 6;

// ray with an origin and direciton
class Ray
{
public:
	Ray(vec3 o, vec3 d) : origin(o), dir(glm::normalize(d)) {}
	~Ray() {}

	// get point at distance along ray
	vec3 getPoint(const float dist) const
	{
		return origin + dist * dir;
	}

	vec3 origin, dir;
};

// abstract base class for a shape with a center and some material properties
class Object
{
public:

	Object(vec3 position = vec3(0), vec3 color = vec3(1), float opacity = 1, float reflectivity = 0)
		: position(position), surfaceColor(color), opacity(opacity), reflectivity(reflectivity) {}
	virtual ~Object() {}

	// check if ray intersects with object and set dist to point of intersection on ray
	virtual bool intersect(const Ray &ray, float &dist) const = 0;

	// return normal at the point of incident
	virtual vec3 getNormal(const vec3 &incident) const = 0;

	vec3 position;
	float opacity, reflectivity;
	vec3 surfaceColor;
};

// sphere specied by a radius
class Sphere : public Object
{
public:

	Sphere(vec3 position = vec3(0), float radius = 1, vec3 color = vec3(1), float opacity = 1, float reflectivity = 0)
	: Object(position, color, opacity, reflectivity), radius(radius), radiusSquared(radius * radius) {}
	~Sphere() {}

	bool intersect(const Ray &ray, float &dist) const override
	{
#if 1
		vec3 l = position - ray.origin;
		float disc = glm::dot(l, ray.dir);
		if (disc < 0)
			return false;
		float dSquared = glm::dot(l, l) - disc * disc;
		if (dSquared > radiusSquared) 
			return false;
		float thc = sqrt(radiusSquared - dSquared);
		dist = disc - thc;

		return true;
#endif
#if 0
		vec3 rc = ray.origin - position;
		float c = glm::dot(rc, rc) - radiusSquared;
		float b = dot(ray.dir, rc);
		float d = b * b - c;
		float t = -b - glm::sqrt(glm::abs(d));
		if (d < 0 || t < 0)
			return false;
		else
		{
			dist = t;
			return true;
		}
#endif
	}

	vec3 getNormal(const vec3 &incident) const override
	{
		return glm::normalize(incident - position);
	}

	float radius, radiusSquared;
};

// flat disk specified by radius and plane normal
class Disk : public Object
{
public:

	Disk(vec3 position = vec3(0), float radius = 1, vec3 normal = vec3(0, 1, 0), vec3 color = vec3(1), float opacity = 1, float reflectivity = 1)
		: Object(position, color, opacity, reflectivity), radius(radius), radiusSquared(radius * radius), normal(glm::normalize(normal)) {}
	~Disk() {}

	bool intersect(const Ray &ray, float &dist) const override
	{
		float denom = glm::dot(normal, ray.dir);
		if (denom > 1e-6) {
			vec3 p0l0 = position - ray.origin;
			dist = glm::dot(p0l0, normal) / denom;
			if (dist >= 0)
			{
				vec3 p = ray.getPoint(dist);
				vec3 v = p - position;
				float d2 = dot(v, v);
				return d2 <= radiusSquared;
			}
		}

		return false;
	}

	vec3 getNormal(const vec3 &incident) const override
	{
		return -normal;
	}

	float radius, radiusSquared;
	vec3 normal;
};

vec3 traceRay(const std::vector<Object*> &objects, const Ray &ray, const int depth)
{
	vec3 pointColor = vec3(0), intersectionPoint;
	float minDist = INFINITY;
	Object const *objHit = nullptr;

	for (auto obj : objects)
	{
		float dist;
		if (obj->intersect(ray, dist))
		{
			// we hit something!
			if (dist < minDist)
			{
				objHit = obj;
				minDist = dist;
			}
		}
	}

	if (!objHit)
	{
		// if we didn't hit anything, return the background color
		return bgColor;
	}

	intersectionPoint = ray.getPoint(minDist);

	vec3 normal = objHit->getNormal(intersectionPoint);

	float facingratio = glm::dot(-ray.dir, normal);
	float fresneleffect = glm::mix(glm::pow(1.0f - facingratio, 3.0f), 1.0f, 0.1f);

	vec3 reflection(0);
	vec3 refraction(0);

	if (objHit->reflectivity > 0 && depth < MAX_DEPTH)
	{
		vec3 reflectDir(glm::reflect(ray.dir, normal));
		Ray r1(intersectionPoint + normal, reflectDir);

		//pointColor += fresneleffect * objHit->reflectivity * objHit->surfaceColor * traceRay(objects, r1, depth + 1);
		reflection = traceRay(objects, r1, depth + 1);
	}

	pointColor += fresneleffect * reflection + refraction * (1 - fresneleffect) * (1 - objHit->opacity) * objHit->surfaceColor;

	pointColor += ambientColor * objHit->surfaceColor;

	vec3 lightDir = glm::normalize(lightPos - intersectionPoint);

	Ray shadowRay(intersectionPoint, lightDir);
	float lightDist;
	for (auto obj : objects)
	{
		if (obj->intersect(shadowRay, lightDist))
		{
			// if we hit something on the way to the light, then we are in shadow
			//std::cout << "Light dist: " << lightDist << "\n";
			if (lightDist > 0) return pointColor;
		}
	}

	float diff = glm::max(0.0f, glm::dot(lightDir, normal));
	pointColor += objHit->surfaceColor * diff;

	vec3 reflectDir = glm::reflect(-lightDir, normal);
	pointColor += objHit->surfaceColor * glm::pow(glm::max(glm::dot(normal, reflectDir), 0.0f), 50.0f);


	return pointColor;
}

void render(const std::vector<Object*> &objects, const char *filename)
{
	unsigned width = 1280, height = 720;
	vec3 *image = new vec3[width * height], *pixel = image;
	const vec3 cameraPosition = vec3(0.0, 0.0, 20.0);
	const vec3 cameraDirection = glm::normalize(vec3(0.0, 0.0, -1.0));
	const vec3 cameraUp = glm::normalize(vec3(0.0, 1.0, 0.0));

	float fov = 50.0;
	float fovx = glm::pi<float>() * fov / 360.0;
	float fovy = fovx * float(height) / float(width);

	float ulen = tan(fovx);
	float vlen = tan(fovy);

	for (unsigned y = height; y > 0; --y)
	{
		for (unsigned x = 0; x < width; ++x, ++pixel)
		{
			float u = (2.0 * ((x + 0.5) / width) - 1.0);
			float v = (2.0 * ((y + 0.5) / height) - 1.0);

			vec3 nright = glm::normalize(glm::cross(cameraUp, cameraDirection));
			vec3 currPixel = cameraPosition + cameraDirection + nright * u * ulen + cameraUp * v *vlen;

			vec3 rayDirection = glm::normalize(currPixel - cameraPosition);
			Ray ray(cameraPosition, rayDirection);
			*pixel = traceRay(objects, ray, 0);
		}
	}
	
	// save result to a bitmap image
	writeBitmap(filename, (char *)image, width, height);
	delete [] image;
}

int main(int argc, char **argv)
{
	std::string filename;
	if (argc == 2)
		filename = argv[1];
	else
		filename = "image.bmp";

	std::vector<Object*> objects;

	//spheres.push_back(Sphere(vec3(5, 5, -15), 3, vec3(0.9, 0.9, 0.2)));
	
	objects.push_back(new Sphere(vec3(0.0, 0, -20), 4, vec3(1.00, 0.32, 0.36), 0.2, 0.7));
	objects.push_back(new Sphere(vec3(5.0, -1, -15), 2, vec3(0.90, 0.76, 0.46), 1.0, 1));
	objects.push_back(new Sphere(vec3(5.0, 0, -25), 3, vec3(0.35, 0.97, 0.37), 1.0, 0.5));
	objects.push_back(new Sphere(vec3(-5.5, 0, -10), 3, vec3(0.9, 0.9, 0.9), 1.0, 1));
	//objects.push_back(new Sphere(vec3(0.0, -10008, -20), 10000, vec3(0.20, 0.20, 0.20), 0, 0.0));
	
	objects.push_back(new Sphere(vec3(0, 5, -15), 1.0, vec3(0.2, 0.32, 0.9), 1.0, 1));

	objects.push_back(new Disk(vec3(0, -5, -30), 40.0, vec3(0, -1, 0), vec3(0.2, 0.2, 0.2), 1.0, 0.0));

	//objects.push_back(new Sphere(vec3(0.0, 0, 0), 0.01, vec3(1.00, 0.32, 0.36), 0.2, 0.7));

	typedef std::chrono::high_resolution_clock clock;
	typedef std::chrono::milliseconds ms;
	typedef std::chrono::duration<double> duration;

	auto start = clock::now();

	render(objects, filename.c_str());

	auto finish = clock::now();
	duration timeTaken = finish - start;
	std::cout << "Render complete, took " << std::chrono::duration_cast<ms>(timeTaken).count() << "ms\n";

	for (auto obj : objects)
	{
		delete obj;
	}

	return EXIT_SUCCESS;
}
