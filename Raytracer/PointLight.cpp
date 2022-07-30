#include "PointLight.hpp"

using namespace std;
using namespace glm;
PointLight::PointLight(const dvec3& position, const dvec3& color) : Light(color)
{
	this->position = position;

}

PointLight::~PointLight()
{
}

double PointLight::getDistance(const Ray& ray) const {
	dvec3 transposition = transformPos(this->position, mat4(1));
	return glm::distance(transposition, ray.origin);
}

Ray PointLight::getRay(const dvec3& rayOrigin) const {
	//dvec3 transposition = transformPos(this->position, mat4(1));
	return Ray(rayOrigin, glm::normalize(this->position - rayOrigin));
}

double PointLight::getAttenuation(double distance, double constant, double linear, double quadratic)const {
	return (constant + (linear * distance) + (quadratic * (distance * distance)));
}
