#include <Windows.h>
#include <wrl.h>

#include <dxgi.h>
#include <dxgi1_2.h>

#include <d3d11.h>
#include <DirectXMath.h>

#include "Timer.h"
#include "Shader.h"
#include "DDSTextureLoader11.h"

#include <array> // for std::size

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Winmm.lib")

// Globals.
HWND g_hWnd = nullptr;
LPCTSTR g_windowClass = L"Learn DirectX Class";
Microsoft::WRL::ComPtr<IDXGIFactory2> g_IDXGIFactory;
Microsoft::WRL::ComPtr<ID3D11Device1> g_d3dDevice;
Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_ImmediateContext;
Microsoft::WRL::ComPtr<IDXGISwapChain1> g_SwapChain;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_RenderTargetView;
Microsoft::WRL::ComPtr<ID3D11Buffer> g_VertexBuffer;
Microsoft::WRL::ComPtr<ID3D11InputLayout> g_VertexLayout;
Microsoft::WRL::ComPtr<ID3D11VertexShader> g_VertexShader;
Microsoft::WRL::ComPtr<ID3D11PixelShader> g_PixelShader;
Microsoft::WRL::ComPtr<ID3D11Buffer> g_IndexBuffer;
Microsoft::WRL::ComPtr<ID3D11Resource> g_Resource;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_ShaderResourceView;
Microsoft::WRL::ComPtr<ID3D11SamplerState> g_SamplerState;
Microsoft::WRL::ComPtr<ID3D11BlendState1> g_BlendState;
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> g_DSState;
Microsoft::WRL::ComPtr<ID3D11Texture2D> g_DepthStencil;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> g_DSV;
Microsoft::WRL::ComPtr<ID3D11Buffer> g_ConstantBuffer;
Microsoft::WRL::ComPtr<ID3D11Buffer> g_ConstantBuffer2;

UINT g_verticesSize = 0;
UINT g_indexCount = 0;

const float g_color[] = { 0.16f, 0.16f, 0.16f, 1.0f };
float g_angle;

Timer timer; // For delta time

// Forward declarations
void InitWindow(HINSTANCE hInstance);
void InitDirect3D();
void Update(float angle);
void Render();
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

using namespace DirectX;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    InitWindow(hInstance);
    InitDirect3D();

    // Message loop.
    MSG msg = {0};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // Render loop.
            Update(timer.Peek());
            Render();
        }
    }

    return (int)msg.wParam;
}

void InitWindow(HINSTANCE hInstance)
{
    // Register window
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;       // Can use getmodulehandle to decouple main param dependency 
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = g_windowClass;
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    RegisterClassEx(&wc);

    // Create window.

    g_hWnd = CreateWindowEx(
        0,
        g_windowClass,
        L"Learn DirectX",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800,
        600,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    // Make the window visible (it isn't, by default). 
    ShowWindow(g_hWnd, SW_SHOW);
}

void InitDirect3D()
{
    // Direct 3D Initialization.
    // Describe swap chain.
    DXGI_SWAP_CHAIN_DESC1 sd = {};
    SecureZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.Width = 800;
    sd.Height = 600;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

    // Create device.
    const D3D_FEATURE_LEVEL featureLevels[] =
    { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
      D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevelsSupported;

    Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;

    D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        &d3dDevice,
        &featureLevelsSupported,
        &g_ImmediateContext
    );

    d3dDevice.As(&g_d3dDevice);

    // Create IDXGI factory. 
    CreateDXGIFactory1(__uuidof(IDXGIFactory1), &g_IDXGIFactory);

    // Create swap chain.
    g_IDXGIFactory->CreateSwapChainForHwnd(
        g_d3dDevice.Get(),
        g_hWnd,
        &sd,
        nullptr,
        nullptr,
        &g_SwapChain);

    // Create render target view (RTV).
    ID3D11Texture2D* pBackBuffer = nullptr;
    // Get a pointer to the back buffer.
    g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&pBackBuffer));

    // Create a render target view.
    g_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_RenderTargetView);

    pBackBuffer->Release(); // Not using this anymore.

    // Depth Buffer
    {
        D3D11_DEPTH_STENCIL_DESC depthDesc = { 0 };
        depthDesc.DepthEnable = TRUE;
        depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
        g_d3dDevice->CreateDepthStencilState(&depthDesc, &g_DSState);

        // Bind depth state.
        g_ImmediateContext->OMSetDepthStencilState(g_DSState.Get(), 1u);

        // Create depth stencil texture.
        D3D11_TEXTURE2D_DESC descDepth = {};
        descDepth.Width = 800u;
        descDepth.Height = 600u;
        descDepth.MipLevels = 1u;
        descDepth.ArraySize = 1u;
        descDepth.Format = DXGI_FORMAT_D32_FLOAT;
        descDepth.SampleDesc.Count = 1u;
        descDepth.SampleDesc.Quality = 0u;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        g_d3dDevice->CreateTexture2D(&descDepth, nullptr, &g_DepthStencil);

        // Create view of depth stencil texture.
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
        descDSV.Format = DXGI_FORMAT_D32_FLOAT;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0u;

        g_d3dDevice->CreateDepthStencilView(g_DepthStencil.Get(), &descDSV, &g_DSV);

        // Bind depth stencil view to output merger.
        g_ImmediateContext->OMSetRenderTargets(1u, g_RenderTargetView.GetAddressOf(), g_DSV.Get());
    }

    // Setup viewport.
    D3D11_VIEWPORT vp = {};
    vp.Width = 800;
    vp.Height = 600;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_ImmediateContext->RSSetViewports(1, &vp);

    // Basic initialization over, 
    // this sect is prep for drawing.

    // Shader setup. 
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    Shader CompileVertex(L"VertexShader.hlsl","VSmain", "vs_4_0", &vertexShader);
    Shader CompilePixel(L"PixelShader.hlsl","PSmain", "ps_4_0", &pixelShader);

    // Create Vertex Shader.
    g_d3dDevice->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), nullptr, &g_VertexShader);
    // Create Pixel Shader.
    g_d3dDevice->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), nullptr, &g_PixelShader);
    // Define Input (vertex) Layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    g_d3dDevice->CreateInputLayout(layout, (UINT)std::size(layout), vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), &g_VertexLayout);

    // Vertex stuff
    struct Vertex
    {
        XMFLOAT2 pos;
        XMFLOAT2 tex;
        XMFLOAT3 color;
    };

    Vertex vertices[] =
    {
        // Position               // Texture
        { XMFLOAT2( -0.5f,  0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT2(  0.5f,  0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT2(  0.5f, -0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT2( -0.5f, -0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
    };

    g_verticesSize = (UINT)std::size(vertices);

    // Create the Vertex buffer.
    {
        // Fill in buffer description.
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(Vertex) * (UINT)std::size(vertices);
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        // Fill in subresource data. 
        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = vertices;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;

        // Create the vertex buffer.
        g_d3dDevice->CreateBuffer(&bufferDesc, &InitData, &g_VertexBuffer);

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        g_ImmediateContext->IASetVertexBuffers(0, 1, g_VertexBuffer.GetAddressOf(), &stride, &offset);
    }

    // Create Index buffer.
    {
        // Create indices.
        unsigned int indices[] = 
        { 
            0, 1, 2,  // Top right triangle
            0, 2, 3   // Bottem left triangle (together form rectangle) 
        };

        g_indexCount = (UINT)std::size(indices);

        // Fill in buffer description.
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(unsigned int) * (UINT)std::size(indices);
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        // Define the resource data.
        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = indices;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;

        // Create the buffer with the device.
        g_d3dDevice->CreateBuffer(&bufferDesc, &InitData, &g_IndexBuffer);

        // Set the buffer.
        g_ImmediateContext->IASetIndexBuffer(g_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    }

    // Set the input layout.
    g_ImmediateContext->IASetInputLayout(g_VertexLayout.Get());

    // Set topology.
    g_ImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Blending
    {
        D3D11_BLEND_DESC1 blendState = {};
        SecureZeroMemory(&blendState, sizeof(blendState));
        blendState.RenderTarget[0].BlendEnable = true;
        blendState.RenderTarget[0].LogicOpEnable = false;
        blendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
        blendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        g_d3dDevice->CreateBlendState1(&blendState, &g_BlendState);

        float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        UINT sampleMask = 0xffffffff;

        g_ImmediateContext->OMSetBlendState(g_BlendState.Get(), blendFactor, sampleMask);
    }

    // Texture stuffs
    {
        // Lightweight DDS file loader from the DirectX Tool Kit.
        // Source: https://github.com/Microsoft/DirectXTK/wiki/DDSTextureLoader
        CreateDDSTextureFromFile(g_d3dDevice.Get(), L"Assets/Textures/container.dds", &g_Resource, &g_ShaderResourceView);

        // Create sample state.
        D3D11_SAMPLER_DESC samplerDesc = {};
        SecureZeroMemory(&samplerDesc, sizeof(samplerDesc));
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        g_d3dDevice->CreateSamplerState(&samplerDesc, &g_SamplerState);

        // Set shader resource for first texture.
        g_ImmediateContext->PSSetShaderResources(0, 1, g_ShaderResourceView.GetAddressOf());
        
        // Create second texture & set shader resource for it.  
        CreateDDSTextureFromFile(g_d3dDevice.Get(), L"Assets/Textures/awesomeface.dds", &g_Resource, &g_ShaderResourceView);
        g_ImmediateContext->PSSetShaderResources(1, 1, g_ShaderResourceView.GetAddressOf());
        
        // Bind sampler state. 
        g_ImmediateContext->PSSetSamplers(0, 1, g_SamplerState.GetAddressOf());
    }
}

void Update(float angle)
{
    // TODO: do this better...
    // i.e. not create the buffer every frame.
    // Constant buffer
    {
        struct ConstantBuffer
        {
            XMMATRIX transform;
        };

        // Supply vertex shader constant data.
        const ConstantBuffer cb =
        {
            XMMatrixTranspose(
                XMMatrixRotationX(XMConvertToRadians(55.0f)) *                                      // model
                XMMatrixTranslation(0.0f, 0.0f, 3.0f) *                                             // view 
                XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), 800.0f / 600.0f, 0.1f, 100.f)   // projection
            )
        };

        // Fill in the buffer description.
        D3D11_BUFFER_DESC cbDesc = {};
        cbDesc.ByteWidth = sizeof(cb);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.MiscFlags = 0;
        cbDesc.StructureByteStride = 0;

        // Fill in the subresource data.
        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = &cb;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;

        // Create the buffer.
        g_d3dDevice->CreateBuffer(&cbDesc, &InitData, &g_ConstantBuffer);
    }
}

void Render()
{
    g_ImmediateContext->OMSetRenderTargets(1, g_RenderTargetView.GetAddressOf(), g_DSV.Get());
    g_ImmediateContext->ClearRenderTargetView(g_RenderTargetView.Get(), g_color);
    g_ImmediateContext->ClearDepthStencilView(g_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

    g_ImmediateContext->VSSetShader(g_VertexShader.Get(), nullptr, 0);
    g_ImmediateContext->PSSetShader(g_PixelShader.Get(), nullptr, 0);
    g_ImmediateContext->VSSetConstantBuffers(0, 1, g_ConstantBuffer.GetAddressOf());
    g_ImmediateContext->DrawIndexed(g_indexCount, 0, 0);

    g_SwapChain->Present(1, 0);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}