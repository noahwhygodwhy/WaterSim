#ifndef MODEL_H
#define MODEL_H
#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//#include "Mesh.hpp"
//#include "Shader.hpp"
#include "Triangle.hpp"
#include "Material.hpp"

using namespace std;
using namespace glm;

//class Model;





class Model
{

    static map<string, Model*> loadedModels;
    //unsigned int boundingVBO;

public:

    Model(string modelname, dvec3 pos = dvec3(0.0), dvec3 rotation = dvec3(0.0));
    Model();
    ~Model();
    mat4 transform;
    vector<Triangle*> children;
    vector<Material*> modelMaterials;
private:

    mat4 originalTransform;
    string directory;
    pair<vec3, vec3> boundingBox;
    void loadModel(string modelName);
    vector<Triangle*> processNode(aiNode* node, const aiScene* scene, const mat4& parentTx, float scaleFactor);
    vector<Triangle*> processMesh(aiMesh* mesh, const aiScene* scene, const mat4& nodeTx);
    //vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

};

#endif
