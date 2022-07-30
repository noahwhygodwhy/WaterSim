#ifndef SQUARE_LIGHT_H
#define SQUARE_LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include "Ray.hpp"
#include "Light.hpp"
#include <functional>
#include <random>
#include "Shape.hpp"

using namespace std;
using namespace glm;



//TODO: make a square light texturable with 

class SquareLight : public Light
{
public:
	SquareLight(dvec3 position, dvec3 normal, double width, double height, dvec3 color = dvec3(1.0));
	~SquareLight();
	dvec3 position;
	dvec3 normal;

	dvec3 bottomLeft;
	dvec3 topRight;
	dvec3 right;
	dvec3 up;

	double width;
	double height;
	Ray getRay(const dvec3& rayOrigin) const;
	double getAttenuation(double distance, double constant, double linear, double quadratic) const;
	double getDistance(const Ray& ray) const;
private:
	dvec3 getIntersection(const Ray& ray)const;

};


#endif