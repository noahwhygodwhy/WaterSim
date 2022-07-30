#include "DirectionalLight.hpp"
#include "CoordinateHelpers.hpp"

DirectionalLight::DirectionalLight(dvec3 direction, dvec3 color) : Light(color)
{
	this->direction = glm::normalize(direction);
	this->reverseDirection = -this->direction;
}

DirectionalLight::~DirectionalLight()
{
}

double DirectionalLight::getDistance(const Ray& ray) const {
	return bigNumberButNotInfinity;
}

Ray DirectionalLight::getRay(const dvec3& rayOrigin) const {


	//dvec3 alteredRayOrigin = transformPos(rayOrigin, mat4(1.0), view);

	dvec3 alteredReverseDirection = dvec4(this->reverseDirection, 1.0f);
	
	dvec3 transposition = transformPos(this->reverseDirection * bigNumberButNotInfinity, mat4(1));

	return Ray(rayOrigin, glm::normalize(transposition - rayOrigin));
}
double DirectionalLight::getAttenuation(double distance, double constant, double linear, double quadratic)const {
	return 1.0;
}
