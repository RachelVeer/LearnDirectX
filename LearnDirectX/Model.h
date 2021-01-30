#pragma once

#include "Mesh.h"
#include <wrl.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <iostream>

class Model
{
public:
    Model(ID3D11Device1* device, ID3D11DeviceContext* immediateContext, ID3DBlob* vs, ID3D11InputLayout* vLayout,
        ID3D11Buffer* vb, ID3D11Buffer* ib, std::string const &path)
    {
        this->m_d3dDevice = device;
        this->m_ImmediateContext = immediateContext;
        this->m_ImmediateContext = immediateContext;
        this->m_VertexShader = vs;
        this->m_VertexLayout = vLayout;
        this->m_VertexBuffer = vb;
        this->m_IndexBuffer = ib;
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
    // model data
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(std::string path)
    {
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

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

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // process vertex positions, normals and texture coordinates
            DirectX::XMFLOAT4 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vector.w = 1.0f;
            vertex.Position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vector.w = 1.0f;
            vertex.Normal = vector;

            vertices.push_back(vertex);
        }
        // process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return Mesh(m_d3dDevice.Get(), m_ImmediateContext.Get(), m_VertexShader.Get(), m_VertexLayout.Get(), 
            m_VertexBuffer.Get(), m_IndexBuffer.Get(), vertices, indices);
    }
};