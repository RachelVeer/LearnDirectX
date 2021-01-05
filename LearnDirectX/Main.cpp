#include <Windows.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <wrl.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// Globals.
static bool g_Running = true;
Microsoft::WRL::ComPtr<IDXGIFactory2> g_IDXGIFactory;
Microsoft::WRL::ComPtr<ID3D11Device> g_d3dDevice;
Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_ImmediateContext;
Microsoft::WRL::ComPtr<IDXGISwapChain1> g_SwapChain;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_RenderTargetView;

// Forward declaration (for window registration)
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    LPCTSTR windowClass = L"Learn DirectX Class";
    HWND hWnd = nullptr;

    // Register window
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;       // Can use getmodulehandle to decouple main param dependency 
    wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = windowClass;
    wc.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

    RegisterClassEx(&wc);

    // Create window.

    hWnd = CreateWindowEx(
        0,
        windowClass,
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
    ShowWindow(hWnd, SW_SHOW);

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
        hWnd,
        &sd,
        nullptr,
        nullptr,
        &g_SwapChain);

    // TODO: Create RTV
    // Lead: https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-initialize
    // Create render target view (RTV).
    ID3D11Texture2D* pBackBuffer;
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

    // Message loop.
    MSG msg = {0};
    const float color[] = { 1.0f, 0.3f, 0.4f, 1.0f };
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
            g_ImmediateContext->ClearRenderTargetView(g_RenderTargetView.Get(), color);
            g_SwapChain->Present(1, 0);
        }
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}