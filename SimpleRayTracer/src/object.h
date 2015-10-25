#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>

using glm::vec3;

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
		// intersection algorithm from tutorial
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
		// first intersection algorithm. Might be wrong?
		vec3 rc = ray.origin - position;
		float c = glm::dot(rc, rc) - radiusSquared;
		float b = glm::dot(ray.dir, rc);
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
#if 0
		// tutorial intersection code
		float denom = glm::dot(normal, ray.dir);
		if (abs(denom) > 1e-6) {
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
#endif
#if 1
	float denom = glm::dot(normal, ray.dir);
	if (abs(denom) > std::numeric_limits<float>::min())
	{
		dist = glm::dot(normal, position - ray.origin) / denom;
		if (dist >= 0)
		{
			vec3 p = ray.getPoint(dist);
			vec3 v = p - position;
			float d2 = dot(v, v);
			return d2 <= radiusSquared;
		}
	}

	return false;
#endif
	}

	vec3 getNormal(const vec3 &incident) const override
	{
		return normal;
	}

	float radius, radiusSquared;
	vec3 normal;

};
