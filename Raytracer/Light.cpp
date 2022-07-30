#include "Light.hpp"



Light::Light(const dvec3& color, bool multisample)
{
	this->multisample = multisample;
	this->color = color;
}

Light::~Light()
{
}