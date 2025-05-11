#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <tiny_obj_loader.h>

#include <mesh.h>
#include "../Engine/texture.h"

#include "ShadingProgram.h"
#include <set>


unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma);

namespace terrain{

class Model
{
public:
	/*  Model Data */
	std::vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	std::vector<Mesh> _meshes;
	std::string directory;
	bool gammaCorrection;
	unsigned int drawMode;

	/*  Functions   */
	// constructor, expects a filepath to a 3D model.
	Model(std::string const &path, unsigned int drawMode = GL_TRIANGLES, bool gamma = false) : gammaCorrection(gamma)
	{
		this->drawMode = drawMode;
		loadModel(path);
	}

	// draws the model, and thus all its meshes
	void Draw(gl::ShadingProgram& shader)
	{
		for (unsigned int i = 0; i < _meshes.size(); i++)
			_meshes[i].Draw(shader, drawMode);
	}

	void Draw(gl::ShadingProgram& shader, unsigned int nIstance)
	{
		for (unsigned int i = 0; i < _meshes.size(); i++)
			_meshes[i].DrawIstances(shader, drawMode, nIstance);
	}

private:
	/*  Functions   */
	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(std::string const &path)
	{
		tinyobj::ObjReaderConfig reader_config;
		reader_config.mtl_search_path = std::filesystem::path(path).root_directory(); // Path to material files
		
		tinyobj::ObjReader reader;
		
		if (!reader.ParseFromFile(path, reader_config)) {
		  if (!reader.Error().empty()) {
			  std::cerr << "TinyObjReader: " << reader.Error();
		  }
		  exit(1);
		}
		
		if (!reader.Warning().empty()) {
		  std::cout << "TinyObjReader: " << reader.Warning();
		}
		
		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		[[maybe_unused]] auto& materials = reader.GetMaterials();
		
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			
			using vntIndex = std::tuple<int32_t, int32_t, int32_t>;
		  
			auto indicesToVertex = [&attrib](const vntIndex& vnt) -> Vertex {
				const auto& [vPos, nPos, tPos] = vnt;

				tinyobj::real_t vx = attrib.vertices[3*size_t(vPos)+0];
				tinyobj::real_t vy = attrib.vertices[3*size_t(vPos)+1];
				tinyobj::real_t vz = attrib.vertices[3*size_t(vPos)+2];
				
				glm::vec3 position{vx, vy, vz};

				glm::vec3 normal{0.0f, 0.0f, 0.0f};

				// Check if `normal_index` is zero or positive. negative = no normal data
				if (nPos >= 0) {
					tinyobj::real_t nx = attrib.normals[3*size_t(nPos)+0];
					tinyobj::real_t ny = attrib.normals[3*size_t(nPos)+1];
					tinyobj::real_t nz = attrib.normals[3*size_t(nPos)+2];

					normal = {nx, ny, nz};
				}
				
				glm::vec2 uv{0.0f, 0.0f};

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (tPos >= 0) {
					tinyobj::real_t tx = attrib.texcoords[2*size_t(tPos)+0];
					tinyobj::real_t ty = attrib.texcoords[2*size_t(tPos)+1];

					uv = {tx, ty};
				}

				return Vertex{position, normal, uv, {}, {}};
			};

			std::vector<vntIndex> uniqueIndicesSet;
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
				
				assert(fv == 3); // we handle triangles only
				
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					
					auto c = vntIndex{idx.vertex_index, idx.normal_index, idx.texcoord_index};
					
					uint32_t index = uniqueIndicesSet.size();
					
					// note: this approach could be slow for objects with a huge number of polygons 
					if(auto it = std::find(uniqueIndicesSet.begin(), uniqueIndicesSet.end(), c); it != uniqueIndicesSet.end())
					{
						index = static_cast<uint32_t>(std::distance(uniqueIndicesSet.begin(), it));
					}
					else
					{
						auto indices = vntIndex(idx.vertex_index, idx.normal_index, idx.texcoord_index);
						vertices.emplace_back(indicesToVertex(indices));
						uniqueIndicesSet.emplace_back(indices);
					}

					indices.emplace_back(index);
				}
				index_offset += fv;
				}

			std::cout << "Unique Indices For shape " << s << " : " << uniqueIndicesSet.size() << std::endl;
			std::cout << "Indices For shape " << s << " : " << indices.size() << std::endl;
			
			this->_meshes.emplace_back(Mesh(std::move(vertices), indices, {}));
		}
	}

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	// void processNode(aiNode *node, const aiScene *scene)
	// {
	// 	// process each mesh located at the current node
	// 	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	// 	{
	// 		// the node object only contains indices to index the actual objects in the scene. 
	// 		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
	// 		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
	// 		meshes.push_back(processMesh(mesh, scene));
	// 	}
	// 	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	// 	for (unsigned int i = 0; i < node->mNumChildren; i++)
	// 	{
	// 		processNode(node->mChildren[i], scene);
	// 	}

	// }

	// Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	// {
	// 	// data to fill
	// 	std::vector<Vertex> vertices;
	// 	std::vector<unsigned int> indices;
	// 	std::vector<Texture> textures;

	// 	// Walk through each of the mesh's vertices
	// 	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	// 	{
	// 		Vertex vertex;
	// 		glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
	// 						  // positions
	// 		vector.x = mesh->mVertices[i].x;
	// 		vector.y = mesh->mVertices[i].y;
	// 		vector.z = mesh->mVertices[i].z;
	// 		vertex.Position = vector;
	// 		// normals
	// 		vector.x = mesh->mNormals[i].x;
	// 		vector.y = mesh->mNormals[i].y;
	// 		vector.z = mesh->mNormals[i].z;
	// 		vertex.Normal = vector;
			
	// 		// texture coordinates
	// 		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
	// 		{
	// 			glm::vec2 vec;
	// 			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
	// 			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
	// 			vec.x = mesh->mTextureCoords[0][i].x;
	// 			vec.y = mesh->mTextureCoords[0][i].y;
	// 			vertex.TexCoords = vec;
	// 		}
	// 		else
	// 			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			
	// 		// tangent
	// 		vector.x = mesh->mTangents[i].x;
	// 		vector.y = mesh->mTangents[i].y;
	// 		vector.z = mesh->mTangents[i].z;
	// 		vertex.Tangent = vector;
	// 		// bitangent
	// 		vector.x = mesh->mBitangents[i].x;
	// 		vector.y = mesh->mBitangents[i].y;
	// 		vector.z = mesh->mBitangents[i].z;
	// 		vertex.Bitangent = vector;
	// 		vertices.push_back(vertex);
	// 	}
	// 	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	// 	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	// 	{
	// 		aiFace face = mesh->mFaces[i];
	// 		// retrieve all indices of the face and store them in the indices vector
	// 		for (unsigned int j = 0; j < face.mNumIndices; j++)
	// 			indices.push_back(face.mIndices[j]);
	// 	}
	// 	// process materials
	// 	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// 	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// 	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
	// 	// Same applies to other texture as the following list summarizes:
	// 	// diffuse: texture_diffuseN
	// 	// specular: texture_specularN
	// 	// normal: texture_normalN

	// 	// 1. diffuse maps
	// 	std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	// 	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	// 	// 2. specular maps
	// 	std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	// 	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	// 	// 3. normal maps
	// 	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	// 	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	// 	// 4. height maps
	// 	std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
	// 	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	// 	// return a mesh object created from the extracted mesh data
	// 	return Mesh(vertices, indices, textures);
	// }

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	// std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
	// {
	// 	std::vector<Texture> textures;
	// 	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	// 	{
	// 		aiString str;
	// 		mat->GetTexture(type, i, &str);
	// 		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
	// 		bool skip = false;
	// 		for (unsigned int j = 0; j < textures_loaded.size(); j++)
	// 		{
	// 			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
	// 			{
	// 				textures.push_back(textures_loaded[j]);
	// 				skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
	// 				break;
	// 			}
	// 		}
	// 		if (!skip)
	// 		{   // if texture hasn't been loaded already, load it
	// 			Texture texture;
	// 			texture.id = TextureFromFile(str.C_Str(), this->directory, false);
	// 			texture.type = typeName;
	// 			texture.path = str.C_Str();
	// 			textures.push_back(texture);
	// 			textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
	// 		}
	// 	}
	// 	return textures;
	// }
};

}
#endif