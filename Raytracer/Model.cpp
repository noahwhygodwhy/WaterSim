
#include "Model.hpp"



using namespace std;
using namespace glm;


vector<Texture> textures_loaded;



unsigned char badData[3] = { 255, 0, 255 };
//Texture badTexture = { 1, 1, badData, "diffuse", "" };



Model::Model(string modelname, dvec3 pos, dvec3 rotation) {
    this->transform = mat4(1.0);
    this->transform = glm::rotate(this->transform, float(rotation.x), vec3(1.0, 0.0, 0.0));
    this->transform = glm::rotate(this->transform, float(rotation.y), vec3(0.0, 1.0, 0.0));
    this->transform = glm::rotate(this->transform, float(rotation.z), vec3(0.0, 0.0, 1.0));
    this->transform = glm::translate(this->transform, vec3(pos));


    //string directory = "./Models/" + modelname + "/" + modelname + ".fbx";
    this->directory = "./Models/" + modelname + "/";//
    this->loadModel(modelname);
}


Model::~Model() {

}
Model::Model() {}

vec3 tv3(const aiVector3D& a) {
    vec3 v;
    v.x = a.x;
    v.y = a.y;
    v.z = a.z;
    return v;
}
vec2 tv2(const aiVector3D& a) {
    vec2 v;
    v.x = a.x;
    v.y = a.y;
    return v;
}

mat4 tm4(aiMatrix4x4 a) {
    return mat4(a.a1, a.b1, a.c1, a.d1,
                a.a2, a.b2, a.c2, a.d2,
                a.a3, a.b3, a.c3, a.d3,
                a.a4, a.b4, a.c4, a.d4);
    //return transpose(mat4(a.a1, a.a2, a.a3, a.a4,
    //    a.b1, a.b2, a.b3, a.b4,
    //    a.c1, a.c2, a.c3, a.c4,
    //    a.d1, a.d2, a.d3, a.d4));
}

void Model::loadModel(string modelName) {
    string fullPath = this->directory + modelName + ".fbx";

    cout << "loading model " << fullPath.c_str() << endl;
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(fullPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_DropNormals);
    this->originalTransform = mat4(1.0f);
    //this->transform = mat4(1.0f);

    for (uint32_t matIndex = 0; matIndex < scene->mNumMaterials; matIndex++) {
        aiMaterial* mat = scene->mMaterials[matIndex];

        aiString* fileName = new aiString();


        printf("here1\n");
        aiReturn ret =  mat->GetTexture(aiTextureType_DIFFUSE, 0, fileName);
        printf("filename: %s\n", fileName->C_Str());
        if (fileName->length == 0) { 
            this->modelMaterials.push_back(new Material("Bug"));
            continue;
        }
        if (ret != AI_SUCCESS) {
            printf("Error getting the texture %s\n", fullPath.c_str());
            exit(-1);
        }

        Texture* diffuse = loadTexture(string(fileName->C_Str()), this->directory);

        //this->modelMaterials.push_back(Material("Glass", dvec3(0), diffuse->getVec3Sampler()));
        this->modelMaterials.push_back(new Material("PlainWhiteTees", dvec3(0), diffuse->getVec3Sampler()));

        //this->modelMaterials.push_back(Material("PlainWhiteTees"));

    }






    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
        printf("here1");
        return;
    }

    //directory = path.substr(0, path.find_last_of('/'));

    this->children = processNode(scene->mRootNode, scene, mat4(1.0f), 0.01f);

}

vector<Triangle*> Model::processNode(aiNode* node, const aiScene* scene, const mat4& parentTx, float scaleFactor) {
    vector<Triangle*> toReturn = vector<Triangle*>();

    mat4 tx = (tm4(node->mTransformation) * 0.01f);
    tx[3][3] = 1.0f;//fixes the scaling

    for (uint i = 0; i < node->mNumMeshes; i++) {

        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        vector<Triangle*> toAdd =processMesh(mesh, scene, tx);
        toReturn.insert(toReturn.end(), toAdd.begin(), toAdd.end());
    }

    for (uint i = 0; i < node->mNumChildren; i++) {

        vector<Triangle*> toAdd = processNode(node->mChildren[i], scene, tx, scaleFactor);
        toReturn.insert(toReturn.end(), toAdd.begin(), toAdd.end());
    }
    return toReturn;
}


vector<Triangle*> Model::processMesh(aiMesh* mesh, const aiScene* scene, const mat4& nodeTx) {

    vector<Triangle*> toReturn;
    vector<Vertex> vertices;
    vector<uint32_t> indices;

    //Texture diffuse;
    //Texture specular;



    for (uint i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        if (mesh->HasPositions()) {

            vertex.position = transformPos( tv3(mesh->mVertices[i]), this->transform*nodeTx);
            //printf("vertex position: %s\n", glm::to_string(vertex.position).c_str());
        }
        if (mesh->HasNormals()) {
            vertex.normal = glm::normalize(transformNormal(tv3(mesh->mNormals[i]), this->transform*nodeTx));
        }
        if (mesh->HasTangentsAndBitangents()) {
            vertex.tangent = tv3(mesh->mTangents[i]);
            vertex.bitangent = tv3(mesh->mBitangents[i]);
        }
        if (mesh->HasTextureCoords(0)) { //TODO: what is indexes of texture coords
            vertex.texCoords = tv2(mesh->mTextureCoords[0][i]);
        }
        else {
            vertex.texCoords = vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }
    //process indices
    for (uint i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (uint j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    Material* m;

    if (mesh->mMaterialIndex >= 0)
    {
        m = this->modelMaterials.at(mesh->mMaterialIndex);
    }
    else {
        m = new Material("PlainWhiteTees");
    }

    for (uint i = 0; i < indices.size(); i += 3) {
        toReturn.push_back(new Triangle(
            vertices.at(indices.at(i + 0)),
            vertices.at(indices.at(i + 1)),
            vertices.at(indices.at(i + 2)),
            m
        ));
    }

    return toReturn;
}


/*Texture textureFromFile(const char* path, const string& directory)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 3); //requires 3 components

    if (!data) {
        printf("error loading texture %s\n", filename.c_str());
    }

    Texture t;
    t.data = data;
    t.width = width;
    t.height = height;
    t.path = filename;
    return t;
}*/

/*vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    //printf("checking for textures of type %i\n", type);
    //printf("has %i of those\n", mat->GetTextureCount(type));
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        //printf("actually has %i textures of %i type\n", mat->GetTextureCount(type));
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture = textureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // add to loaded textures
        }
    }
    //printf("has %i textures\n", textures.size());
    textures.push_back(badTexture);
    return textures;
}*/