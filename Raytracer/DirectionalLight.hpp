#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include <glm/glm.hpp>
#include "Ray.hpp"
#include "Light.hpp"
using namespace std;
using namespace glm;



constexpr double bigNumberButNotInfinity = 10000000000000.0;

class DirectionalLight : public Light
{
public:
	DirectionalLight(dvec3 direction, dvec3 color);
	~DirectionalLight();
	double getDistance(const Ray& ray) const;
	Ray getRay(const dvec3& rayOrigin) const;
	double getAttenuation(double distance, double constant, double linear, double quadratic)const;
private:
	dvec3 direction;
	dvec3 reverseDirection;
};

#endif;