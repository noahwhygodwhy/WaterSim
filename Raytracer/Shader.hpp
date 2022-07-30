#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "glad/glad.h"

using namespace std;
using namespace glm;

class Shader
{
public:
	unsigned int vertShader; ///<The pointer to the vertex shader.
	unsigned int fragShader; ///<The pointer to the fragment shader.

	unsigned int program;

	Shader();
	Shader(const char* vertexPath, const char* fragmentPath);
	~Shader();

	void use() const;

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setMatFour(const string& name, mat4 value) const;
	void setVecThree(const string& name, vec3 value) const;

private:

};

#endif
