#pragma once
#include <wrl.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <vector>

struct Vertex
{
    DirectX::XMFLOAT4 Position;
    DirectX::XMFLOAT4 Normal;
};

class Mesh
{
public:
    // d3d data
    Microsoft::WRL::ComPtr<ID3D11Device1> m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_ImmediateContext;
    Microsoft::WRL::ComPtr<ID3DBlob> m_VertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_VertexLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer, m_VertexBuffer;
    // Mesh data.
    std::vector<Vertex> m_Vertices;
    std::vector<unsigned int> m_Indices;

    Mesh(ID3D11Device1* device, ID3D11DeviceContext* immediateContext, ID3DBlob* vs, ID3D11InputLayout* vLayout,
        ID3D11Buffer* vb, ID3D11Buffer* ib,
        std::vector<Vertex> vertices, std::vector<unsigned int> indices)
    {
        this->m_Vertices = vertices;
        this->m_Indices = indices;
        this->m_d3dDevice = device;
        this->m_ImmediateContext = immediateContext;
        this->m_VertexShader = vs;
        this->m_VertexLayout = vLayout;
        this->m_VertexBuffer = vb;
        this->m_IndexBuffer = ib;

        setupMesh();
    }
    void Draw() // Seems draw taking in shader is for constant buffer stuff essentially. We'll see.
    {
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        m_ImmediateContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
        m_ImmediateContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
        m_ImmediateContext->DrawIndexed(m_Indices.size(), 0, 0);
    }
private:
    // set d3d members here?
    void setupMesh()
    {
        // Describe vertex layout
        {
            // Define Input (vertex) Layout
            D3D11_INPUT_ELEMENT_DESC layout[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            m_d3dDevice->CreateInputLayout(layout, (UINT)std::size(layout), m_VertexShader->GetBufferPointer(), m_VertexShader->GetBufferSize(), &m_VertexLayout);
        }

        // Create vertex buffer.
        {
            // Fill in buffer description.
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.ByteWidth = sizeof(Vertex) * (UINT)m_Vertices.size();
            bufferDesc.StructureByteStride = sizeof(Vertex);
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0;
            bufferDesc.MiscFlags = 0;

            // Fill in subresource data. 
            D3D11_SUBRESOURCE_DATA InitData = {};
            InitData.pSysMem = m_Vertices.data();
            InitData.SysMemPitch = 0;
            InitData.SysMemSlicePitch = 0;

            // Create the vertex buffer.
            m_d3dDevice->CreateBuffer(&bufferDesc, &InitData, &m_VertexBuffer);

            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            m_ImmediateContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
        }

        // Create index buffer.
        {
            // Fill in buffer description.
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.ByteWidth = sizeof(unsigned int) * (UINT)m_Indices.size();
            bufferDesc.StructureByteStride = sizeof(unsigned int);
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0;
            bufferDesc.MiscFlags = 0;

            // Define the resource data.
            D3D11_SUBRESOURCE_DATA InitData = {};
            InitData.pSysMem = m_Indices.data();
            InitData.SysMemPitch = 0;
            InitData.SysMemSlicePitch = 0;

            // Create the buffer with the device.
            m_d3dDevice->CreateBuffer(&bufferDesc, &InitData, &m_IndexBuffer);

            // Set the buffer.
            m_ImmediateContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
        }

        // Set the input layout.
        m_ImmediateContext->IASetInputLayout(m_VertexLayout.Get());

        // Set topology.
        m_ImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
};
