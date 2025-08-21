#include "Model.h"
#include "Mesh.h"
#include <tuple> 
#include <iomanip>

// Model::Model(const std::string& path, std::shared_ptr<Shader> defaultShader, bool gamma)
//         : m_defaultShader(defaultShader), gammaCorrection(gamma) {
//         loadModel(path);
//     }

// Model::Model(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Shader> shader) {
//     auto defaultMaterial = std::make_shared<Material>(shader);
//     m_meshes.emplace_back(std::move(vertices), std::move(indices), defaultMaterial);
// }


Model::Model(const std::string& path, bool gamma)
    : gammaCorrection(gamma) // REMOVE m_defaultShader from the initializer list
{
    loadModel(path);
}

// --- The primitive constructor is ALREADY CORRECT! No changes needed. ---
Model::Model(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    auto defaultMaterial = std::make_shared<Material>();
    m_meshes.emplace_back(vertices, indices, defaultMaterial);
}

// void Model::Draw(Shader& shader, int& textureUnit) {
//         for (Mesh& mesh : m_meshes) {
//             mesh.Draw(shader,textureUnit);
//         }
//     }

void Model::DrawWithMaterial(Shader& shader, int& textureUnit) {
    for (Mesh& mesh : m_meshes) {
        mesh.DrawWithMaterial(shader, textureUnit);
    }
}
void Model::DrawGeometryOnly() {
    for (Mesh& mesh : m_meshes) {
        mesh.DrawGeometryOnly();
    }
}
// void Model::addMesh(const Mesh&& mesh) {
//     m_meshes.emplace_back(std::move(mesh));
// }

void Model::loadModel(const std::string& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }
        m_directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }


// void Model::processNode(aiNode* node, const aiScene* scene) {
//         for (unsigned int i = 0; i < node->mNumMeshes; i++) {
//             aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
//             m_meshes.emplace_back(processMesh(mesh, scene));
//         }
//         for (unsigned int i = 0; i < node->mNumChildren; i++) {
//             processNode(node->mChildren[i], scene);
//         }
//     }

// void Model::processNode(aiNode* node, const aiScene* scene) {
//         for (unsigned int i = 0; i < node->mNumMeshes; i++) {
//         aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

//         // Call the modified processMesh
//         auto [vertices, indices, material] = processMesh(mesh, scene); // C++17 structured binding

//         // Now, use emplace_back with the returned data
//         m_meshes.emplace_back(std::move(vertices), std::move(indices), material);
//     }
//         for (unsigned int i = 0; i < node->mNumChildren; i++) {
//             processNode(node->mChildren[i], scene);
//         }
//     }

void Model::processNode(aiNode* node, const aiScene* scene) {
    // Process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];

        // Call our function that returns the tuple of data
        auto [vertices, indices, material] = processMesh(assimpMesh, scene);
        m_meshes.emplace_back(std::move(vertices), std::move(indices), material);
    }

    // Then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

// Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
        

//         std::vector<Vertex> vertices;
//         std::vector<unsigned int> indices;

       
//         // Walk through each of the mesh's vertices
//     for (unsigned int i = 0; i < mesh->mNumVertices; i++)
//         {
//             Vertex vertex; // Create a temporary vertex to fill

//             // --- 1. Process vertex positions, normals, and texture coordinates ---
//             glm::vec3 vector; 
//             // Positions
//             vector.x = mesh->mVertices[i].x;
//             vector.y = mesh->mVertices[i].y;
//             vector.z = mesh->mVertices[i].z;
//             vertex.Position = vector;

//             // Normals
//             if (mesh->HasNormals())
//             {
//                 vector.x = mesh->mNormals[i].x;
//                 vector.y = mesh->mNormals[i].y;
//                 vector.z = mesh->mNormals[i].z;
//                 vertex.Normal = vector;
//             }

//             // Texture Coordinates
//             if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
//             {
//                 glm::vec2 vec;
//                 vec.x = mesh->mTextureCoords[0][i].x; 
//                 vec.y = mesh->mTextureCoords[0][i].y;
//                 vertex.TexCoords = vec;
//             }
//             else
//                 vertex.TexCoords = glm::vec2(0.0f, 0.0f);


//             // --- 2. THIS IS WHERE YOU ADD YOUR CODE ---
//             //    (It needs to be inside this loop)
//             // Tangent and Bitangent
//             if (mesh->HasTangentsAndBitangents())
//             {
//                 // Tangent
//                 vector.x = mesh->mTangents[i].x;
//                 vector.y = mesh->mTangents[i].y;
//                 vector.z = mesh->mTangents[i].z;
//                 vertex.Tangent = vector;

//                 // Bitangent
//                 vector.x = mesh->mBitangents[i].x;
//                 vector.y = mesh->mBitangents[i].y;
//                 vector.z = mesh->mBitangents[i].z;
//                 vertex.Bitangent = vector;
//             }
//             // ---------------------------------------------

//             // You would also load bone data here if you have it
//             // ...

//             vertices.emplace_back(vertex); // Add the completed vertex to the list
//         }

//         for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
//             aiFace face = mesh->mFaces[i];
//             for (unsigned int j = 0; j < face.mNumIndices; j++)
//                 indices.emplace_back(face.mIndices[j]);
//         }


//         aiMaterial* assimpMaterial = scene->mMaterials[mesh->mMaterialIndex];
//         std::shared_ptr<Material> meshMaterial = loadMaterial(assimpMaterial);

//         return Mesh(vertices, indices, meshMaterial);
//     }

std::tuple<std::vector<Vertex>, std::vector<unsigned int>, std::shared_ptr<Material>>
Model::processMesh(aiMesh* mesh, const aiScene* scene){
        

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    // std::shared_ptr<Material> material;
       
        // Walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex; // Create a temporary vertex to fill

            // --- 1. Process vertex positions, normals, and texture coordinates ---
            glm::vec3 vector; 
            // Positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // Normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }

            // Texture Coordinates
            if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);


            // --- 2. THIS IS WHERE YOU ADD YOUR CODE ---
            //    (It needs to be inside this loop)
            // Tangent and Bitangent
            if (mesh->HasTangentsAndBitangents())
            {
                // Tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;

                // Bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            // ---------------------------------------------

            // You would also load bone data here if you have it
            // ...

            vertices.emplace_back(vertex); // Add the completed vertex to the list
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.emplace_back(face.mIndices[j]);
        }

        // --- TEMPORARY DEBUGGING CODE ---
    // Code for printing mesh data DEBUG
        // std::cout << "--- Blender Cube Vertex Data ---" << std::endl;
        // std::cout << std::fixed << std::setprecision(3); // Nicer formatting for floats
        
        // std::cout << "vertices = {" << std::endl;
        // for (const auto& v : vertices) {
        //     std::cout << "    { {" 
        //               << v.Position.x << "f, " << v.Position.y << "f, " << v.Position.z << "f}, {"
        //               << v.Normal.x << "f, " << v.Normal.y << "f, " << v.Normal.z << "f}, {"
        //               << v.TexCoords.x << "f, " << v.TexCoords.y << "f}, {"
        //               << v.Tangent.x << "f, " << v.Tangent.y << "f, " << v.Tangent.z << "f}, {"
        //               << v.Bitangent.x << "f, " << v.Bitangent.y << "f, " << v.Bitangent.z << "f} }," << std::endl;
        // }
        // std::cout << "};" << std::endl << std::endl;

        // std::cout << "--- Blender Cube Index Data ---" << std::endl;
        // std::cout << "indices = {" << std::endl << "    ";
        // for (size_t i = 0; i < indices.size(); ++i) {
        //     std::cout << indices[i] << ", ";
        //     if ((i + 1) % 6 == 0) { // Newline every two triangles
        //         std::cout << std::endl << "    ";
        //     }
        // }
        // std::cout << "\n};" << std::endl;

        // exit(0);
  

        aiMaterial* assimpMaterial = scene->mMaterials[mesh->mMaterialIndex];
        std::shared_ptr<Material> meshMaterial = loadMaterial(assimpMaterial);
        std::shared_ptr<Material> material;

        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
            // You likely have a 'loadMaterialTextures' function or similar here.
            // It should create and return a std::shared_ptr<Material>.
            material = loadMaterial(aiMat); // Assuming you have a function like this
        } else {
            // If no material, create a default one
            // material = std::make_shared<Material>(m_defaultShader);
            material = std::make_shared<Material>();
        }

        // return Mesh(vertices, indices, meshMaterial);
        return std::make_tuple(std::move(vertices), std::move(indices), material);
    }
    
std::shared_ptr<Material> Model::loadMaterial(aiMaterial* mat) {
        // Create a new Material with our default shader
        // auto material = std::make_shared<Material>(m_defaultShader);
        auto material = std::make_shared<Material>();

        // We use a helper lambda to load textures and add them to our material.
        auto addTextureToMaterial = [&](aiTextureType type, Texture::TextureType texType) {
            for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
                auto texture = loadMaterialTexture(mat, type, i, texType);
                if (texture) {
                    material->addTexture(texture);
                }
            }
        };

        // Load all texture types
        addTextureToMaterial(aiTextureType_DIFFUSE, Texture::TextureType::Diffuse);
        addTextureToMaterial(aiTextureType_SPECULAR, Texture::TextureType::Specular);
        addTextureToMaterial(aiTextureType_HEIGHT, Texture::TextureType::Normal); // Assimp calls normal maps "height maps" sometimes
        addTextureToMaterial(aiTextureType_NORMALS, Texture::TextureType::Normal);
        // addTextureToMaterial(aiTextureType_BASE_COLOR, Texture::TextureType::BaseColor);
        

        aiColor3D basecolor(0.f, 0.f, 0.f);
        mat->Get(AI_MATKEY_COLOR_DIFFUSE, basecolor);
        material->BaseColor = glm::vec3(basecolor.r, basecolor.b, basecolor.g);

        return material;
    }

std::shared_ptr<Texture> Model::loadMaterialTexture(aiMaterial* mat, aiTextureType type, int texIndex, Texture::TextureType texType) {
        aiString str;
        mat->GetTexture(type, texIndex, &str);
        std::string path = str.C_Str();

        // 1. Check if the texture is already in our cache
        if (m_texturesLoaded.count(path)) {
            return m_texturesLoaded[path];
        }

        // 2. If not, load it
        std::string fullPath = m_directory + '/' + path;
        auto texture = std::make_shared<Texture>(fullPath, texType);

        // 3. Add it to the cache and return it
        m_texturesLoaded[path] = texture;
        return texture;
    }