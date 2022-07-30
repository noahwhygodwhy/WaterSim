


#include "Shader.hpp"
using namespace glm;
using namespace std;

unsigned int makeShader(string filename, uint type) {

    long unsigned int length;
    ifstream stream(filename, ios::in | ios::ate | ios::binary);
    length = long unsigned int(stream.tellg());
    stream.seekg(0, ios::beg);
    char* shaderSource = new char[length + 1];
    shaderSource[length] = '\0';

    stream.read(shaderSource, length);

    unsigned int shader;
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    int  success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("ERROR::SHADER::%s::COMPILATION_FAILED\n%s\n", filename.c_str(), infoLog);
        exit(-1);
    }
    else
    {
        printf("SHADER::%s::COMPILATION_SUCCEEDED\n", filename.c_str());
    }
    return shader;
}

//an object that has a vertex shader and frag shader for easier handling
Shader::Shader(const char* vertexPath, const char* fragmentPath) {


    vertShader = makeShader(vertexPath, GL_VERTEX_SHADER);
    fragShader = makeShader(fragmentPath, GL_FRAGMENT_SHADER);
    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

}
Shader::Shader() {
}

Shader::~Shader() {

}

void Shader::use() const {
    glUseProgram(program);
}


void Shader::setBool(const string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(program, name.c_str()), (int)value);
}
void Shader::setInt(const string& name, int value) const
{
    glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}
void Shader::setFloat(const string& name, float value) const
{
    glUniform1f(glGetUniformLocation(program, name.c_str()), value);
}
void Shader::setMatFour(const string& name, mat4 value) const
{
    glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, false, value_ptr(value));
}
void Shader::setVecThree(const string& name, vec3 value) const
{
    glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, value_ptr(value));
}
