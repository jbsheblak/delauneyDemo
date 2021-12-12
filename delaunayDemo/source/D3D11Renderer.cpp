
#include "D3D11Renderer.h"

#include <vector>

namespace
{
    struct SData
    {
        TD3D11DevicePtr mpDevice;
        TD3D11DeviceContextPtr mpDeviceCtx;
    };

    SData sData;
}

// =================================================================
// NRenderer
// =================================================================

namespace NRenderer
{
    // --------------------------------------------------------------

    bool initialize()
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

            sData.mpDevice = pDevice;
            sData.mpDeviceCtx = pDeviceCtx;
            break;
        }

        return (sData.mpDevice && sData.mpDeviceCtx);
    }

    // --------------------------------------------------------------

    void shutdown()
    {
        sData = SData();
    }

    // --------------------------------------------------------------
};