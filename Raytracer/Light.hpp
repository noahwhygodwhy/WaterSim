#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>


#include "Ray.hpp"

using namespace std;
using namespace glm;


class Light
{
public:
	Light(const dvec3& color, bool multisample = false);
	~Light();
	//virtual vec3 rayHit(const vec3& rayOrigin, const mat4& view)const = 0;
	virtual double getDistance(const Ray& ray) const = 0;
	virtual Ray getRay(const dvec3& rayOrigin) const = 0;
	virtual double getAttenuation(double distance, double constant, double linear, double quadratic)const = 0;
	dvec3 color;
	bool multisample;
private:

};


#endif