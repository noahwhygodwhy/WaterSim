#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include <glm/glm.hpp>
#include "Light.hpp"
#include "CoordinateHelpers.hpp"

using namespace std;
using namespace glm;

class PointLight : public Light
{
public:
	PointLight(const dvec3& position, const dvec3& color);
	~PointLight();
	double getDistance(const Ray& ray) const;
	Ray getRay(const dvec3& rayOrigin) const;
	double getAttenuation(double distance, double constant, double linear, double quadratic)const;
private:
	dvec3 position; 

};

#endif;