#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>

#include <dxgi.h>
#include <dxgi1_2.h>

#include <d3d11.h>
#include <DirectXMath.h>

#include "DDSTextureLoader11.h"
#include "Timer.h"
#include "Shader.h"
#include "Camera.h"

#include <array> // for std::size

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Winmm.lib")

using namespace DirectX;

// Structures
struct ConstantBuffer
{
    XMMATRIX model;
    XMMATRIX view;
    XMMATRIX projection;
    XMFLOAT4 lightPos;
    XMFLOAT4 viewPos;
    XMFLOAT4 objectColor;
    XMFLOAT4 lightColor;
};

XMFLOAT3 cubePositions[] = {
            XMFLOAT3( 0.0f,  0.0f,  0.0f),
            XMFLOAT3( 2.0f,  5.0f, 15.0f),
            XMFLOAT3(-1.5f, -2.2f, 2.5f),
            XMFLOAT3(-3.8f, -2.0f, 12.3f),
            XMFLOAT3( 2.4f, -0.4f, 3.5f),
            XMFLOAT3(-1.7f,  3.0f, 7.5f),
            XMFLOAT3( 1.3f, -2.0f, 2.5f),
            XMFLOAT3( 1.5f,  2.0f, 2.5f),
            XMFLOAT3( 1.5f,  0.2f, 1.5f),
            XMFLOAT3(-1.3f,  1.0f, 1.5f) 
        };

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
Microsoft::WRL::ComPtr<ID3D11InputLayout> g_VertexLayout2;
Microsoft::WRL::ComPtr<ID3D11VertexShader> g_VertexShader;
Microsoft::WRL::ComPtr<ID3D11VertexShader> g_VertexShader2;
Microsoft::WRL::ComPtr<ID3D11PixelShader> g_PixelShader;
Microsoft::WRL::ComPtr<ID3D11PixelShader> g_PixelShader2;
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

float g_width = 1600.0f;
float g_height = 900.0f;

UINT g_verticesSize = 0;
UINT g_indexCount = 0;

const float g_color[] = { 0.16f, 0.16f, 0.16f, 1.0f };
XMMATRIX g_transform;

Timer timer; // For simple maths

// Delta time.
Timer dt;
float deltaTime = 0.0f;

// Camera
Camera camera(XMFLOAT3(0.0f, 0.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f));
float lastX = g_width / 2.0f;
float lastY = g_height / 2.0f;
bool firstMouse = true;

// Key input logic.
bool moveForward;
bool moveBackward;
bool moveLeft;
bool moveRight;

// Lighting.
XMFLOAT4 lightPosition = XMFLOAT4(-1.2f, 1.0f, 2.0f, 1.0f);

// Forward declarations
void InitWindow(HINSTANCE hInstance);
void InitDirect3D();
void ProcessInput();
void Render(float angle);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    InitWindow(hInstance);
    InitDirect3D();

    // Message loop.
    MSG msg = {0};
    while (true)
    {
        deltaTime = dt.Mark();
        float angle = timer.Peek();
        ProcessInput();
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            // Check for quit because peekmessage does not signal this via return value.
		    if( msg.message == WM_QUIT )
		    {
			    // Return int (arg to PostQuitMessage is in wparam) signals quit.
			    return (int)msg.wParam;
		    }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // Render loop.
        {
            Render(1.0f);
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
        (int)g_width,
        (int)g_height,
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
    sd.Width = (UINT)g_width;
    sd.Height = (UINT)g_height;
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
        descDepth.Width = (UINT)g_width;
        descDepth.Height = (UINT)g_height;
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
    vp.Width = g_width;
    vp.Height = g_height;
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

    // First cube shader.
    {
        Shader CompileVertex(L"VertexShader.hlsl","VSmain", "vs_4_0", &vertexShader);
        Shader CompilePixel(L"PixelShader.hlsl","PSmain", "ps_4_0", &pixelShader);

        // Create Vertex Shader.
        g_d3dDevice->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), nullptr, &g_VertexShader);
        // Create Pixel Shader.
        g_d3dDevice->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), nullptr, &g_PixelShader);
        // Define Input (vertex) Layout
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        g_d3dDevice->CreateInputLayout(layout, (UINT)std::size(layout), vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), &g_VertexLayout);
    }
    // Light cube shader.
    {
        Shader CompileVertex(L"LightVertex.hlsl","VSmain", "vs_4_0", &vertexShader);
        Shader CompilePixel(L"LightPixel.hlsl","PSmain", "ps_4_0", &pixelShader);

        // Create Vertex Shader.
        g_d3dDevice->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), nullptr, &g_VertexShader2);
        // Create Pixel Shader.
        g_d3dDevice->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), nullptr, &g_PixelShader2);

         // Define Input (vertex) Layout
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        g_d3dDevice->CreateInputLayout(layout, (UINT)std::size(layout), vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), &g_VertexLayout2);
    }

    // Vertex stuff
    struct Vertex
    {
        XMFLOAT3 pos;
        XMFLOAT3 normal;
    };

    Vertex vertices[] = {
        // Position                       // Normal vectors (perpendicular)
        // Top
        { XMFLOAT3( -0.5f, 0.5f, -0.5f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(  0.5f, 0.5f, -0.5f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(  0.5f, 0.5f,  0.5f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3( -0.5f, 0.5f,  0.5f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },

        // Bottom
        { XMFLOAT3( -0.5f, -0.5f, -0.5f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(  0.5f, -0.5f, -0.5f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(  0.5f, -0.5f,  0.5f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3( -0.5f, -0.5f,  0.5f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },

        // Left
        { XMFLOAT3( -0.5f, -0.5f,  0.5f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3( -0.5f, -0.5f, -0.5f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3( -0.5f,  0.5f, -0.5f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3( -0.5f,  0.5f,  0.5f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

        // Right
        { XMFLOAT3( 0.5f, -0.5f,  0.5f ) , XMFLOAT3(1.0f, 0.0f, 0.0f)},
        { XMFLOAT3( 0.5f, -0.5f, -0.5f ) , XMFLOAT3(1.0f, 0.0f, 0.0f)},
        { XMFLOAT3( 0.5f,  0.5f, -0.5f ) , XMFLOAT3(1.0f, 0.0f, 0.0f)},
        { XMFLOAT3( 0.5f,  0.5f,  0.5f ) , XMFLOAT3(1.0f, 0.0f, 0.0f)},

        // Back
        { XMFLOAT3( -0.5f, -0.5f, -0.5f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(  0.5f, -0.5f, -0.5f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(  0.5f,  0.5f, -0.5f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3( -0.5f,  0.5f, -0.5f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },

        // Front
        { XMFLOAT3( -0.5f, -0.5f, 0.5f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(  0.5f, -0.5f, 0.5f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(  0.5f,  0.5f, 0.5f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3( -0.5f,  0.5f, 0.5f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },

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
            3,1,0,
            2,1,3,

            6,4,5,
            7,4,6,

            11,9,8,
            10,9,11,

            14,12,13,
            15,12,14,

            19,17,16,
            18,17,19,

            22,20,21,
            23,20,22

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
    g_ImmediateContext->IASetInputLayout(g_VertexLayout2.Get());

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


    // Constant buffer
    {
        // Fill in the buffer description.
        D3D11_BUFFER_DESC cbDesc = {};
        cbDesc.ByteWidth = sizeof(ConstantBuffer);
        cbDesc.Usage = D3D11_USAGE_DEFAULT;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = 0;
        cbDesc.MiscFlags = 0;
        cbDesc.StructureByteStride = 0;

        // Create the buffer.
        g_d3dDevice->CreateBuffer(&cbDesc, nullptr, &g_ConstantBuffer);

        // Initialize the constant.  
        // TODO: calculate this in a way where there are seperate contants...
        // i.e. some of these don't need to constantly change (see DX samples).

        ConstantBuffer cb = {};
        cb.model = XMMatrixIdentity();
        cb.view = XMMatrixIdentity();
        cb.projection = XMMatrixIdentity();
        g_ImmediateContext->UpdateSubresource(g_ConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);

    }
}

void ProcessInput()
{
    if (moveForward)
    { 
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if(moveBackward)
    {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if(moveLeft)
    {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if(moveRight)
    {
       camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

void Render(float angle)
{
    g_ImmediateContext->OMSetRenderTargets(1, g_RenderTargetView.GetAddressOf(), g_DSV.Get());
    g_ImmediateContext->ClearRenderTargetView(g_RenderTargetView.Get(), g_color);
    g_ImmediateContext->ClearDepthStencilView(g_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

    g_ImmediateContext->VSSetShader(g_VertexShader.Get(), nullptr, 0);
    g_ImmediateContext->PSSetShader(g_PixelShader.Get(), nullptr, 0);
    g_ImmediateContext->VSSetConstantBuffers(0, 1, g_ConstantBuffer.GetAddressOf());
    g_ImmediateContext->PSSetConstantBuffers(0, 1, g_ConstantBuffer.GetAddressOf());

    // First cube.
    {
        // Model matrix.
        XMMATRIX model = XMMatrixRotationX(XMConvertToRadians(angle)) *
                         XMMatrixRotationY(XMConvertToRadians(angle));
        // Camera/viewspace matrix.
        XMMATRIX view = camera.GetViewMatrix();
        // Projection matrix.
        XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(65.0f), g_width / g_height, 0.1f, 100.f);
        
        // Supply vertex shader constant data.
        ConstantBuffer cb = {};
        cb.model = XMMatrixTranspose(model);
        cb.view = XMMatrixTranspose(view);
        cb.projection = XMMatrixTranspose(projection);
        cb.lightPos = lightPosition;
        cb.viewPos = XMFLOAT4(camera.Position.x, camera.Position.y, camera.Position.z, 1.0f);
        cb.objectColor = XMFLOAT4(1.0f, 0.5f, 0.31f, 1.0f);
        cb.lightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); 
        g_ImmediateContext->UpdateSubresource(g_ConstantBuffer.Get(), 0, nullptr, &cb, 0, 0 );
        g_ImmediateContext->DrawIndexed(g_indexCount, 0, 0);
    }

    g_ImmediateContext->VSSetShader(g_VertexShader2.Get(), nullptr, 0);
    g_ImmediateContext->PSSetShader(g_PixelShader2.Get(), nullptr, 0);
    // Light cube.
    {
        // Model matrix.
        XMMATRIX model = XMMatrixScaling(0.2f, 0.2f, 0.2f) *
                        XMMatrixRotationX(XMConvertToRadians(angle)) *
                        XMMatrixRotationY(XMConvertToRadians(angle)) *
                        XMMatrixTranslation(lightPosition.x, lightPosition.y, lightPosition.z);
        // Camera/viewspace matrix.
        XMMATRIX view = camera.GetViewMatrix();
        // Projection matrix.
        XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(65.0f), g_width / g_height, 0.1f, 100.f);

        // Supply vertex shader constant data.
        ConstantBuffer cb = {};
        cb.model = XMMatrixTranspose(model);
        cb.view = XMMatrixTranspose(view);
        cb.projection = XMMatrixTranspose(projection);
        g_ImmediateContext->UpdateSubresource(g_ConstantBuffer.Get(), 0, nullptr, &cb, 0, 0 );
        g_ImmediateContext->DrawIndexed(g_indexCount, 0, 0);
    }

    g_SwapChain->Present(1, 0);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_KEYDOWN:
        {
            if(wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
                return 0;
            }
            if(wParam == 'W')
            {
               moveForward = true;
            }
            if(wParam == 'S')
            {
               moveBackward = true;
            }
            if(wParam == 'A')
            {
               moveLeft = true;
            }
            if(wParam == 'D')
            {
               moveRight = true;
            }
            if(wParam == VK_UP)
            {
                lightPosition.z -= 0.05f;
            }
            if(wParam == VK_DOWN)
            {
                lightPosition.z += 0.05f;
            }
            break;
        }
        case WM_KEYUP:
        {
            if(moveForward)
            {
                moveForward = false;
            }
            if(moveBackward)
            {
                moveBackward = false;
            }
            if(moveLeft)
            {
                moveLeft = false;
            }
            if(moveRight)
            {
                moveRight = false;
            }
            break;
        }
        case WM_LBUTTONDOWN:
        {
            SetCapture(hWnd);
            break;
        }
        case WM_LBUTTONUP:
        {
            ReleaseCapture();
            break;
        }
        case WM_MOUSEMOVE:
        {
            float xpos = (float)GET_X_LPARAM(lParam); 
            float ypos = (float)GET_Y_LPARAM(lParam);
            
            if (firstMouse)
            {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }
            
            float xoffset = xpos - lastX;
            float yoffset = ypos - lastY;
            
            lastX = xpos;
            lastY = ypos;

            camera.ProcessMouseMovement(xoffset, yoffset);
            return 0;
        }
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