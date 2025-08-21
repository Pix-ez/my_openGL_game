// AssetProcessor.cpp
#include "AssetProcessor.h"
#include "Mesh.h" // We need the Vertex struct definition
#include "Material.h" // For material properties

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <fstream>
#include <vector>

// Define a simple binary format for our mesh files
namespace MeshFile {
    struct Header {
        char magic[4] = {'M', 'E', 'S', 'H'};
        uint32_t version = 1;
        uint32_t meshCount = 0;
    };

    struct MeshChunk {
        uint32_t vertexCount;
        uint32_t indexCount;
            
        // Material Data
        aiVector3D baseColor; // Using Assimp's aiVector3D for base color
        float shininess;

        uint32_t diffusePathLength;
        uint32_t specularPathLength;
        uint32_t normalPathLength;
        // Paths will be written immediately after this struct
    };
}


bool AssetProcessor::ProcessModel(const std::filesystem::path& sourceModelPath, 
                                  const std::filesystem::path& projectAssetsPath, 
                                  const std::filesystem::path& destinationMeshPath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(sourceModelPath.string(), 
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    std::ofstream outfile(destinationMeshPath, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Could not open file for writing: " << destinationMeshPath << std::endl;
        return false;
    }

    // --- Write File Header ---
    MeshFile::Header fileHeader;
    fileHeader.meshCount = scene->mNumMeshes;
    outfile.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));

    // --- Write Each Mesh as a Chunk ---
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // 1. Extract Vertex and Index data (like in your old Model class)
        std::vector<Vertex> vertices;
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            Vertex vertex;
            vertex.Position = {mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z};
            vertex.Normal = {mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z};
            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords = {mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y};
            }
            if (mesh->mTangents) {
                vertex.Tangent = {mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z};
            }
            if (mesh->mBitangents) {
                vertex.Bitangent = {mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z};
            }
            vertices.push_back(vertex);
        }

        std::vector<unsigned int> indices;
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            for (unsigned int j = 0; j < mesh->mFaces[f].mNumIndices; ++j) {
                indices.push_back(mesh->mFaces[f].mIndices[j]);
            }
        }
        
        // 2. Extract Material data and process textures
        MeshFile::MeshChunk chunkHeader;
        chunkHeader.vertexCount = vertices.size();
        chunkHeader.indexCount = indices.size();
        
        aiColor3D color(0.f, 0.f, 0.f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        chunkHeader.baseColor = {color.r, color.g, color.b};
        material->Get(AI_MATKEY_SHININESS, chunkHeader.shininess);

        aiString diffusePathStr, specularPathStr, normalPathStr;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &diffusePathStr);
        material->GetTexture(aiTextureType_SPECULAR, 0, &specularPathStr);
        material->GetTexture(aiTextureType_HEIGHT, 0, &normalPathStr); // Assimp often stores normal maps here

        std::string diffuseRelPath = ProcessTexture(diffusePathStr.C_Str(), sourceModelPath.parent_path(), projectAssetsPath);
        std::string specularRelPath = ProcessTexture(specularPathStr.C_Str(), sourceModelPath.parent_path(), projectAssetsPath);
        std::string normalRelPath = ProcessTexture(normalPathStr.C_Str(), sourceModelPath.parent_path(), projectAssetsPath);

        chunkHeader.diffusePathLength = diffuseRelPath.length();
        chunkHeader.specularPathLength = specularRelPath.length();
        chunkHeader.normalPathLength = normalRelPath.length();

        // 3. Write chunk to file
        outfile.write(reinterpret_cast<const char*>(&chunkHeader), sizeof(chunkHeader));
        outfile.write(diffuseRelPath.c_str(), diffuseRelPath.length());
        outfile.write(specularRelPath.c_str(), specularRelPath.length());
        outfile.write(normalRelPath.c_str(), normalRelPath.length());
        outfile.write(reinterpret_cast<const char*>(vertices.data()), vertices.size() * sizeof(Vertex));
        outfile.write(reinterpret_cast<const char*>(indices.data()), indices.size() * sizeof(unsigned int));
    }

    std::cout << "Successfully processed model " << sourceModelPath << " to " << destinationMeshPath << std::endl;
    return true;
}

std::string AssetProcessor::ProcessTexture(const std::filesystem::path& sourceTexturePath,
                                           const std::filesystem::path& sourceModelDirectory,
                                           const std::filesystem::path& projectAssetsPath) {
    if (sourceTexturePath.empty()) {
        return "";
    }

    // Assimp paths can be relative to the model, so construct the full source path
    auto fullSourcePath = sourceModelDirectory / sourceTexturePath;

    // The destination path will be inside the project's assets/textures folder
    auto destinationPath = projectAssetsPath / "textures" / sourceTexturePath.filename();

    // The relative path to store in the material file
    std::string relativePath = "textures/" + sourceTexturePath.filename().string();

    // If the texture isn't already in our project, copy it
    if (!std::filesystem::exists(destinationPath)) {
        try {
            std::filesystem::create_directories(destinationPath.parent_path());
            std::filesystem::copy_file(fullSourcePath, destinationPath);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Warning: Could not copy texture " << fullSourcePath << " to " << destinationPath << " - " << e.what() << std::endl;
            return "";
        }
    }
    return relativePath;
}