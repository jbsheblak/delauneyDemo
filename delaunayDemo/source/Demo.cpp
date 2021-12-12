
#include "Demo.h"

#include "D3D11Renderer.h"
#include <vector>

// =================================================================
// NDemo
// =================================================================

namespace NDemo
{
    void simulate()
    {
    }

    void render()
    {
        float clearColor [] = {0.0f, 0.0f, 0.0f, 1.0f};
        gpDeviceCtx->ClearRenderTargetView(gpTargetColor.Get(), clearColor);
        gpDeviceCtx->ClearDepthStencilView(gpTargetDepth.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        gpDeviceCtx->OMSetRenderTargets(1, gpTargetColor.GetAddressOf(), gpTargetDepth.Get());

        gpSwapChain->Present(1, 0);
    }
}