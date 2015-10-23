#include <vector>

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

class Ray
{
public:
	Ray(vec3 o, vec3 d) : origin(o), dir(glm::normalize(d)) {}
	~Ray() {}

	vec3 getPoint(const float dist) const
	{
		return origin + dist * dir;
	}

	vec3 origin, dir;
};

class Object
{
public:

	Object(vec3 position = vec3(0), vec3 color = vec3(1), float opacity = 1, float reflectivity = 0)
		: position(position), surfaceColor(color), reflectivity(reflectivity) {}
	~Object() {}

	virtual bool intersect(const Ray &ray, float &dist) const = 0;

	virtual vec3 getNormal(const vec3 &incident) const = 0;

	vec3 position;
	float opacity, reflectivity;
	vec3 surfaceColor;
};

class Sphere : public Object
{
public:

	Sphere(vec3 position = vec3(0), float radius = 1, vec3 color = vec3(1), float opacity = 1, float reflectivity = 0)
	: Object(position, color, opacity, reflectivity), radius(radius), radiusSquared(radius * radius) {}
	~Sphere() {}

	bool intersect(const Ray &ray, float &dist) const override
	{
#if 0
		vec3 l = position - ray.origin;
		float v = glm::dot(l, ray.dir);
		float disc = radiusSquared - glm::dot(l, l) + (v * v);

		if (disc < 0)
			return false;
		else
		{
			// set p to the point of intersection
			dist = v - glm::sqrt(disc);
			return true;
		}
#endif
		vec3 l = position - ray.origin;
		float disc = glm::dot(l, ray.dir);
		if (disc < 0) return false;
		float dSquared = glm::dot(l, l) - disc * disc;
		if (dSquared > radiusSquared) return false;
		float thc = sqrt(radiusSquared - dSquared);
		dist = disc - thc;

		return true;
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

	if (objHit->reflectivity > 0 && depth < MAX_DEPTH)
	{
		float facingratio = glm::dot(-ray.dir, normal);
		// change the mix value to tweak the effect
		float fresneleffect = glm::mix(pow(1 - facingratio, 3), 1.0f, 0.1f);
		vec3 reflectDir(glm::reflect(ray.dir, normal));
		Ray r1(intersectionPoint + normal, reflectDir);

		pointColor += fresneleffect * objHit->reflectivity * objHit->surfaceColor * traceRay(objects, r1, depth + 1);
	}

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

void render(const std::vector<Object*> &objects)
{
	unsigned width = 1280, height = 720;
	vec3 *image = new vec3[width * height], *pixel = image;
	float invWidth = 1 / float(width), invHeight = 1 / float(height);
	float fov = 65, aspectratio = width / float(height);
	float angle = tan(glm::pi<float>() * 0.5 * fov / 180.0f);

	// Trace rays
	for (unsigned y = 0; y < height; ++y) {
		for (unsigned x = 0; x < width; ++x, ++pixel) {
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
			float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
			vec3 raydir(xx, yy, -1);
			Ray ray(vec3(0), raydir);
			*pixel = traceRay(objects, ray, 0);
		}
	}
	
	// save result to a bitmap image
	writeBitmap("./image.bmp", (char *)image, width, height);
	delete [] image;
}

int main(int argc, char **argv)
{
	std::vector<Object*> objects;


	//spheres.push_back(Sphere(vec3(5, 5, -15), 3, vec3(0.9, 0.9, 0.2)));
	
	objects.push_back(new Sphere(vec3(0.0, 0, -20), 4, vec3(1.00, 0.32, 0.36), 1, 0.7));
	objects.push_back(new Sphere(vec3(5.0, -1, -15), 2, vec3(0.90, 0.76, 0.46), 1, 1));
	objects.push_back(new Sphere(vec3(5.0, 0, -25), 3, vec3(0.35, 0.97, 0.37), 1, 0.5));
	objects.push_back(new Sphere(vec3(-5.5, 0, -15), 3, vec3(0.9, 0.9, 0.9), 1, 1));
	//objects.push_back(new Sphere(vec3(0.0, -10008, -20), 10000, vec3(0.20, 0.20, 0.20), 0, 0.0));
	
	objects.push_back(new Sphere(vec3(0, 5, -15), 1.0, vec3(0.2, 0.32, 0.9), 1, 1));

	objects.push_back(new Disk(vec3(0, -5, -30), 40.0, vec3(0, -1, 0), vec3(0.2, 0.2, 0.2), 1, 0.5));

	render(objects);

	for (auto obj : objects)
	{
		delete obj;
	}

	return EXIT_SUCCESS;
}
