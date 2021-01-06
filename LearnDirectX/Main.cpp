#include <Windows.h>
#include <wrl.h>

#include <dxgi.h>
#include <dxgi1_2.h>

#include <d3d11.h>
#include <DirectXMath.h>

#include "Shader.h"

#include <array> // for std::size

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

// Globals.
HWND g_hWnd = nullptr;
LPCTSTR g_windowClass = L"Learn DirectX Class";
Microsoft::WRL::ComPtr<IDXGIFactory2> g_IDXGIFactory;
Microsoft::WRL::ComPtr<ID3D11Device> g_d3dDevice;
Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_ImmediateContext;
Microsoft::WRL::ComPtr<IDXGISwapChain1> g_SwapChain;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_RenderTargetView;
Microsoft::WRL::ComPtr<ID3D11Buffer> g_VertexBuffer;
Microsoft::WRL::ComPtr<ID3D11InputLayout> g_VertexLayout;
Microsoft::WRL::ComPtr<ID3D11VertexShader> g_VertexShader;
Microsoft::WRL::ComPtr<ID3D11PixelShader> g_PixelShader;

UINT verticesSize = 0;

// Forward declarations
void InitWindow(HINSTANCE hInstance);
void InitDirect3D();
void Render();
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

using namespace DirectX;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
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

    D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        &g_d3dDevice,
        &featureLevelsSupported,
        &g_ImmediateContext
    );

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

    // TODO: Create RTV
    // Lead: https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-initialize
    // Create render target view (RTV).
    ID3D11Texture2D* pBackBuffer = nullptr;
    // Get a pointer to the back buffer.
    g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&pBackBuffer));

    // Create a render target view.
    g_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_RenderTargetView);

    // Bind the view.
    g_ImmediateContext->OMSetRenderTargets(1, g_RenderTargetView.GetAddressOf(), nullptr);

    pBackBuffer->Release(); // Not using this anymore.

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
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    g_d3dDevice->CreateInputLayout(layout, (UINT)std::size(layout), vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), &g_VertexLayout);

    // Vertex stuff
    struct Vertex
    {
        XMFLOAT2 pos;
        XMFLOAT3 color;
    };

    Vertex vertices[] =
    {
        { XMFLOAT2( 0.0f,  0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT2( 0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT2(-0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    };

    verticesSize = (UINT)std::size(vertices);

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

    // Set the input layout.
    g_ImmediateContext->IASetInputLayout(g_VertexLayout.Get());

    // Set topology.
    g_ImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Render()
{
    g_ImmediateContext->OMSetRenderTargets(1, g_RenderTargetView.GetAddressOf(), nullptr);

    const float color[] = { 0.16f, 0.16f, 0.16f, 1.0f };
    g_ImmediateContext->ClearRenderTargetView(g_RenderTargetView.Get(), color);
    
    g_ImmediateContext->VSSetShader(g_VertexShader.Get(), nullptr, 0);
    g_ImmediateContext->PSSetShader(g_PixelShader.Get(), nullptr, 0);
    g_ImmediateContext->Draw(verticesSize, 0);

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