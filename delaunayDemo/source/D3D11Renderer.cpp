
#include "D3D11Renderer.h"

#include <vector>

TD3D11DevicePtr gpDevice;
TD3D11DeviceContextPtr gpDeviceCtx;
TDXGISwapChain1Ptr gpSwapChain;
TD3D11RenderTargetViewPtr gpTargetColor;
TD3D11DepthTargetViewPtr gpTargetDepth;

// =================================================================
// NRenderer
// =================================================================

namespace NRenderer
{
    // --------------------------------------------------------------

    bool initialize(HWND hWnd)
    {
        TDXGIFactory1Ptr pDXGIFactory;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory))))
        {
            return false;
        }

        std::vector<IDXGIAdapter1*> availableAdapters;
        {
            UINT adapterIdx = 0;
            IDXGIAdapter1 * pAdapter = nullptr;
        
            while (pDXGIFactory->EnumAdapters1(adapterIdx, &pAdapter) != DXGI_ERROR_NOT_FOUND)
            {
                if (pAdapter != nullptr)
                {
                    availableAdapters.push_back(pAdapter);
                }

                adapterIdx++;
            }
        }

        TD3D11DevicePtr pDevice;
        TD3D11DeviceContextPtr pDeviceCtx;
        IDXGIAdapter1 *pBestAdapter = nullptr;

        constexpr D3D_DRIVER_TYPE const driverType = D3D_DRIVER_TYPE_UNKNOWN;
        constexpr D3D_FEATURE_LEVEL const kRequestedFeatureLevel = D3D_FEATURE_LEVEL_11_1;

        UINT const createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED |
                                       D3D11_CREATE_DEVICE_DEBUG;

        for (IDXGIAdapter1* pAdapter : availableAdapters)
        {
            D3D_FEATURE_LEVEL featureLevel;
            HRESULT hr = ::D3D11CreateDevice(pAdapter,
                                             D3D_DRIVER_TYPE_UNKNOWN,
                                             NULL, /*software*/
                                             createDeviceFlags,
                                             &kRequestedFeatureLevel,
                                             1 /*featureLevelArrayCount*/,
                                             D3D11_SDK_VERSION,
                                             &pDevice,
                                             &featureLevel,
                                             &pDeviceCtx);

            if (FAILED(hr))
            {
                continue;
            }

            if (featureLevel != kRequestedFeatureLevel)
            {
                continue;
            }

            if (!pDevice || !pDeviceCtx)
            {
                continue;
            }

            gpDevice = pDevice;
            gpDeviceCtx = pDeviceCtx;
            pBestAdapter = pAdapter;
            break;
        }

        if (!gpDevice || !gpDeviceCtx)
        {
            return false;
        }

        TDXGIFactory4Ptr pDXGIFactory4;
        if (FAILED(pBestAdapter->GetParent(IID_PPV_ARGS(&pDXGIFactory4))))
        {
            return false;
        }

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.SampleDesc.Count = 1;

        TDXGISwapChain1Ptr pSwapChain;
        if (FAILED(pDXGIFactory4->CreateSwapChainForHwnd(gpDevice.Get(),
                                                         hWnd,
                                                         &swapChainDesc,
                                                         nullptr /*fullscreen desc*/,
                                                         nullptr /*pRestrictToOutput*/,
                                                         &pSwapChain)))
        {
            return false;
        }

        gpSwapChain = pSwapChain;

        // setup render target
        {
            TD3D11Texture2DPtr pSwapColor;
            if (FAILED(gpSwapChain->GetBuffer(0 /*buffer*/, IID_PPV_ARGS(&pSwapColor))))
            {
                return false;
            }

            if (FAILED(gpDevice->CreateRenderTargetView(pSwapColor.Get(), nullptr /*pDesc*/, &gpTargetColor)))
            {
                return false;
            }

            D3D11_TEXTURE2D_DESC colorDesc = {};
            pSwapColor->GetDesc(&colorDesc);

            D3D11_TEXTURE2D_DESC depthDesc = {};
            depthDesc.Width = colorDesc.Width;
            depthDesc.Height = colorDesc.Height;
            depthDesc.ArraySize = 1;
            depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
            depthDesc.SampleDesc.Count = 1;
            depthDesc.Usage = D3D11_USAGE_DEFAULT;
            depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            depthDesc.CPUAccessFlags = 0;
            depthDesc.MiscFlags = 0;

            TD3D11Texture2DPtr pSwapDepth;
            if (FAILED(gpDevice->CreateTexture2D(&depthDesc, nullptr /*initialData*/, &pSwapDepth)))
            {
                return false;
            }

            if (FAILED(gpDevice->CreateDepthStencilView(pSwapDepth.Get(), nullptr /*pDesc*/, &gpTargetDepth)))
            {
                return false;
            }
        }
        
        return true;
    }

    // --------------------------------------------------------------

    void shutdown()
    {
        gpTargetColor.Reset();
        gpTargetDepth.Reset();
        gpSwapChain.Reset();
        gpDeviceCtx.Reset();
        gpDevice.Reset();
    }

    // --------------------------------------------------------------
};