#pragma once

#include "Mesh.h"
#include "DDSTextureLoader11.h"
#include <wrl.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <iostream>

class Model
{
public:
    Model(ID3D11Device1* device, ID3D11DeviceContext* immediateContext, ID3DBlob* vs, ID3D11InputLayout* vLayout,
        ID3D11Buffer* vb, ID3D11Buffer* ib, ID3D11Resource* resource, ID3D11ShaderResourceView* srv,
        ID3D11SamplerState* sampler,
        std::string const &path)
    {
        this->m_d3dDevice = device;
        this->m_ImmediateContext = immediateContext;
        this->m_VertexShader = vs;
        this->m_VertexLayout = vLayout;
        this->m_VertexBuffer = vb;
        this->m_IndexBuffer = ib;
        this->m_Resource = resource;
        this->m_ShaderResourceView = srv;
        this->m_SamplerState = sampler;
        loadModel(path);
    }
    void Draw()
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw();
    }
private:
    //d3d objects
    Microsoft::WRL::ComPtr<ID3D11Device1> m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_ImmediateContext;
    Microsoft::WRL::ComPtr<ID3DBlob> m_VertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_VertexLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer, m_VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Resource> m_Resource;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShaderResourceView;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerState;
    // model data
    std::vector<Mesh> meshes;
    std::vector<Texture> textures_loaded;
    std::string directory;

    void loadModel(std::string path)
    {
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
    }
    void processNode(aiNode* node, const aiScene* scene)
    {
        // process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }
    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // process vertex positions, normals and texture coordinates
            DirectX::XMFLOAT3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;

            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                DirectX::XMFLOAT2 vector;
                vector.x = (float)mesh->mTextureCoords[0][i].x;
                vector.y = (float)mesh->mTextureCoords[0][i].y;
                
                vertex.TexCoords = vector;
            }
            else
            {
                vertex.TexCoords = DirectX::XMFLOAT2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }
        // process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
                aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<Texture> specularMaps = loadMaterialTextures(material,
                aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        return Mesh(m_d3dDevice.Get(), m_ImmediateContext.Get(), m_VertexShader.Get(), m_VertexLayout.Get(), 
            m_VertexBuffer.Get(), m_IndexBuffer.Get(), vertices, indices, textures);
    }

    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
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
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); // add to loaded textures
            }
        }
        return textures;
    }
    unsigned int TextureFromFile(const char* path, const std::string& directory)
    {
        unsigned int textureID = 1;
        // Lightweight DDS file loader from the DirectX Tool Kit.
        // Source: https://github.com/Microsoft/DirectXTK/wiki/DDSTextureLoader
        // Directory seems relative to project path. 
        DirectX::CreateDDSTextureFromFile(m_d3dDevice.Get(), L"../resources/objects/backpack/diffuse.dds", &m_Resource, &m_ShaderResourceView);

        // Create sample state.
        D3D11_SAMPLER_DESC samplerDesc = {};
        SecureZeroMemory(&samplerDesc, sizeof(samplerDesc));
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        m_d3dDevice->CreateSamplerState(&samplerDesc, &m_SamplerState);

        // Set shader resource for first texture.
        m_ImmediateContext->PSSetShaderResources(0, 1, m_ShaderResourceView.GetAddressOf());
        // Bind sampler state.
        m_ImmediateContext->PSSetSamplers(0, 1, m_SamplerState.GetAddressOf());

        return textureID;
    }
};