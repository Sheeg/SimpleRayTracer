#include <vector>
#include <string>
#include <iostream>
#include <chrono>

#include "object.h"
#include "bitmap.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>

using glm::vec3;

static const vec3 bgColor(0.1, 0.17, 0.3), ambientColor(0.2, 0.2, 0.2);

static std::vector<vec3> lights;

static const int MAX_DEPTH = 6;

vec3 traceRay(const std::vector<Object*> &objects, const Ray &ray, const int depth)
{
	vec3 pointColor = vec3(0), intersectionPoint;
	float minDist = std::numeric_limits<float>::infinity();
	Object const *objHit = nullptr;

	float bias = 0.01f;

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

	float facingratio = glm::dot(ray.dir, -normal);
	float fresneleffect = glm::mix(glm::pow(1.0f - facingratio, 3.0f), 1.0f, 0.1f);

	vec3 reflection(0);
	vec3 refraction(0);

	
#if 1
	if (objHit->reflectivity > 0 && depth < MAX_DEPTH)
	{
		vec3 reflectDir(glm::reflect(ray.dir, normal));
		//vec3 reflectDir = 2 * (glm::dot(-ray.dir, normal)) * normal + ray.dir;
		Ray reflectRay(intersectionPoint + normal * bias, reflectDir);

		reflection = traceRay(objects, reflectRay, depth + 1);
		//pointColor += fresneleffect * objHit->reflectivity * objHit->surfaceColor * reflection;
		//pointColor += fresneleffect * reflection + refraction * (1 - fresneleffect) * (1 - objHit->opacity) * objHit->surfaceColor;
		pointColor += fresneleffect * reflection;
	}
#endif

	for (auto &light : lights)
	{

		vec3 lightDir = glm::normalize(light - intersectionPoint);
		bool shadow = false;
		Ray shadowRay(intersectionPoint + normal * bias, lightDir);
		float lightDist;
		for (auto obj : objects)
		{
			if (obj->intersect(shadowRay, lightDist))
			{
				shadow = true;
				break;
			}
		}
		if (!shadow)
		{
			float cutoff = 0.0001;
			float r = 20;
			vec3 L = light - intersectionPoint;
			float distance = glm::length(L);
			float d = glm::max(distance - r, 0.0f);
			L /= distance;

			// calculate attenuation
			float denom = d / r + 1;
			float attenuation = 1 / (denom*denom);

			// attenuation should equal 0 when we are beyond max range
			attenuation = (attenuation - cutoff) / (1 - cutoff);
			attenuation = glm::max(attenuation, 0.0f);

			float diff = glm::max(0.0f, glm::dot(L, normal));
			//pointColor += objHit->surfaceColor * diff * attenuation;

			vec3 reflectDir = glm::reflect(-L, normal);
			float specular = glm::pow(glm::max(glm::dot(normal, reflectDir), 0.0f), 80.0f);

			pointColor += (objHit->surfaceColor * diff + specular) * attenuation;
		}
	}

	return pointColor;
}

void render(const std::vector<Object*> &objects, const char *filename)
{
	unsigned width = 1280, height = 720;
	vec3 *image = new vec3[width * height], *pixel = image;
	const vec3 cameraPosition = vec3(0.0, 40.0, 80.0);
	const vec3 cameraDirection = glm::normalize(vec3(0.0, -0.5, -1.0));
	const vec3 cameraUp = glm::normalize(vec3(0.0, 1.0, 0.0));

	/*const vec3 cameraPosition = vec3(0.0, 80.0, .0);
	const vec3 cameraDirection = glm::normalize(vec3(0.0, -1.0, 0.0));
	const vec3 cameraUp = glm::normalize(vec3(0.0, 0.0, 1.0));*/

	float fov = 50.0;
	float fovx = glm::pi<float>() * fov / 360.0;
	float fovy = fovx * float(height) / float(width);

	float ulen = glm::tan(fovx);
	float vlen = glm::tan(fovy);

	for (unsigned y = height; y > 0; --y)
	{
		for (unsigned x = width; x > 0; --x, ++pixel)
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
	
	////objects.push_back(new Sphere(vec3(0.0, 0, -20), 4, vec3(1.00, 0.32, 0.36), 1.0, 1.0));
	//objects.push_back(new Sphere(vec3(0.0, 0, -20), 4, vec3(0, 0, 0), 1.0, 0.8));
	//objects.push_back(new Sphere(vec3(5.0, -1, -15), 2, vec3(0.90, 0.76, 0.46), 1.0, 1));
	//objects.push_back(new Sphere(vec3(5.0, 0, -25), 3, vec3(0.35, 0.97, 0.37), 1.0, 1));
	//objects.push_back(new Sphere(vec3(-6, 0, -5), 3, vec3(0.9, 0.9, 0.9), 1.0, 1));	
	//objects.push_back(new Sphere(vec3(0, 5, -15), 1.0, vec3(0.2, 0.32, 0.9), 1.0, 1));

	objects.push_back(new Sphere(vec3(-6, 4, 0), 4, vec3(0.7, 0.2, 0.2), 1.0, 0.8));

	objects.push_back(new Sphere(vec3(6, 4, 0), 4, vec3(0.2, 0.7, 0.2), 1.0, 0.8));

	objects.push_back(new Sphere(vec3(0, 4, 6), 4, vec3(0.2, 0.2, 0.7), 1.0, 0.8));

	objects.push_back(new Disk(vec3(0, 0, 0), 40.0, glm::normalize(vec3(0, 1, 0)), vec3(0.2, 0.2, 0.25), 0.2, 1));

	lights.push_back(vec3(10, 10, 10));
	lights.push_back(vec3(-10, 10, 10));

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
