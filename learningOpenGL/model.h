#pragma once
#ifndef MODEL_H
#define MODEL_H

#include "shader.h"
#include "mesh.h"
#include <vector>
#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);
class Model {
public:
	//ģ������
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;//�����м��ع����������ڸ�vector��
	bool gammaCorrection;


	Model(string const& path, bool gamma = false) : gammaCorrection(gamma) {
		loadModel(path);
	}
	void Draw(Shader shader) {
		for (unsigned int i = 0; i < meshes.size(); ++i) {
			meshes[i].Draw(shader);
		}
	};
private:


	void loadModel(string const &path) {
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs|aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene) {
		// ����ڵ����е���������еĻ���
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// �������������ӽڵ��ظ���һ����
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh *mesh, const aiScene* scene) {
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			// ������λ�á����ߺ���������
			glm::vec3 vector; // ע������Ϊ�˴���Assimp�����ݣ����Ƕ�����һ��vec3����ʱ������ʹ������һ����ʱ������ԭ����Assimp�������������ַ����ȶ����Լ���һ����������
			//���ǲ�����������ת����GLM�����������С�
			// positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			// normals
			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			// texture coordinates
			if (mesh->mTextureCoords[0]) // ��������Ƿ���İ�������������
			{
				glm::vec2 vec;
				//Assimp����һ��ģ����һ�������������8����ͬ���������꣬���ǲ����õ���ô�࣬����ֻ���ĵ�һ���������ꡣ
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
				// tangent
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;
				// bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
				vertices.push_back(vertex);
		}
		// ��������
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
			// �������
		//�ͽڵ�һ����һ������ֻ������һ��ָ����ʶ���������������Ҫ��ȡ���������Ĳ��ʣ����ǻ���Ҫ����������mMaterials���顣
		//�����������λ������mMaterialIndex�����У�����ͬ���������������һ�������Ƿ�����в���

			
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				// 1. diffuse maps
				vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
				textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
				// 2. specular maps
				vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
				textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
				// 3. normal maps
				std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
				textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
				// 4. height maps
				std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
				textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
				// 5. Reflection maps (Note that ASSIMP doesn't load reflection maps properly from wavefront objects, so we'll cheat a little by defining the reflection maps as ambient maps in the .obj file, which ASSIMP is able to load)
				vector<Texture> reflectionMaps = this->loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_reflection");
				textures.insert(textures.end(), reflectionMaps.begin(), reflectionMaps.end());

		return Mesh(vertices, indices, textures);


	}

	//���溯�������˸����������͵���������λ�ã���ȡ��������ļ�λ�ã������ز�������������
	//����Ϣ��������һ��Vertex�ṹ���С�
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			cout << str.C_Str()<<endl;
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
			{   // �������û�б����أ��������
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), directory,true);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture); // ��ӵ��Ѽ��ص�������
			}
		}
		return textures;
	}


};

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum internalFormat;
		GLenum dataFormat;
		if (nrComponents == 1)
		{
			internalFormat = dataFormat = GL_RED;
		}
		else if (nrComponents == 3)
		{
			internalFormat = gamma ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			internalFormat = gamma? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}


#endif