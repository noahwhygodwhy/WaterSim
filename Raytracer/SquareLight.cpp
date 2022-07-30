#include "SquareLight.hpp"

using namespace std;
using namespace glm;

extern bool prd;

SquareLight::SquareLight(dvec3 position, dvec3 normal, double width, double height, dvec3 color) : Light(color, true)
{

	dvec3 worldUp = dvec3(0.0, 1.0, 0.0);

	this->normal = normalize(normal);
	printf("normal: %s\n", glm::to_string(this->normal).c_str());

	this->right = normalize(glm::cross(worldUp, this->normal));
	if (isinf(this->right.x) || isnan(this->right.x)) {//incase of vertical normal
		this->right = dvec3(1.0, 0.0, 0.0);
	}

	printf("right: %s\n", glm::to_string(this->right).c_str());

	this->up = normalize(glm::cross(this->normal, this->right));

	printf("up: %s\n", glm::to_string(this->up).c_str());

	this->position = position;
	printf("position: %s\n", glm::to_string(this->position).c_str());
	this->width = width;
	printf("width: %f\n", (this->width));
	this->height = height;
	printf("height: %f\n", (this->height));

	this->bottomLeft = this->position - ((this->right * 0.5 * width) + (this->up * 0.5 * height));

	printf("botom left: %s\n", glm::to_string(bottomLeft).c_str());

	this->topRight = bottomLeft + (this->right * width) + (this->up * height);
	printf("topRight: %s\n", glm::to_string(topRight).c_str());

}

SquareLight::~SquareLight()
{

}
//returns pseudorandom between 0.0 and 1.0
double randDub() {

	return static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
}

Ray SquareLight::getRay(const dvec3& rayOrigin) const {
	double randX = randDub();
	double randY = randDub();



	dvec3 randomPosition = this->bottomLeft + (this->right * randX*width) + (this->up * randY*height);


	return Ray(rayOrigin, glm::normalize(randomPosition - rayOrigin));

}

double SquareLight::getAttenuation(double distance, double constant, double linear, double quadratic)const {
	return (constant + (linear * distance) + (quadratic * (distance * distance)));
}

double SquareLight::getDistance(const Ray& ray) const {
	dvec3 hitPoint = this->getIntersection(ray);
	return glm::distance(ray.origin, hitPoint);
}

//this is a lot simpler because it should only receive rays that
//we know for a fact have already intersected the square
//we just have to find that point again
dvec3 SquareLight::getIntersection(const Ray& ray)const {
	double denom = glm::dot(this->normal, ray.direction);
	if (abs(denom) < glm::epsilon<double>()) {//should never happen
		printf("SQUARE LIGHT POSITION/INTERSECTION NOT RIGHT");
		exit(-1);
	}

	dvec3 d = this->position - ray.origin;
	double t = glm::dot(d, this->normal) / denom;
	if (t < glm::epsilon<double>()) {//should never happen
		printf("SQUARE LIGHT POSITION/INTERSECTION NOT RIGHT");
		exit(-1);
	}
	return ray.origin + (ray.direction * t);
		
	
}
