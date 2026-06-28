
#pragma once

#include <D3Dcompiler.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_5.h>
#include <string>
#include <wrl.h>

//#include "dxc/dxcapi.h"

template <typename T>
using TMicrosoftComPtr = Microsoft::WRL::ComPtr<T>;

// Common
typedef TMicrosoftComPtr<IUnknown> TUnknownPtr;
//typedef TMicrosoftComPtr<IDxcBlob> TD3DBlobPtr;

// DXGI
typedef TMicrosoftComPtr<IDXGIAdapter>         TDXGIAdapterPtr;
typedef TMicrosoftComPtr<IDXGIAdapter1>        TDXGIAdapter1Ptr;
typedef TMicrosoftComPtr<IDXGIAdapter2>        TDXGIAdapter2Ptr;
typedef TMicrosoftComPtr<IDXGIAdapter3>        TDXGIAdapter3Ptr;
typedef TMicrosoftComPtr<IDXGIDevice>          TDXGIDevicePtr;
typedef TMicrosoftComPtr<IDXGIDevice1>         TDXGIDevice1Ptr;
typedef TMicrosoftComPtr<IDXGIDeviceSubObject> TDXGIDeviceSubObjectPtr;
typedef TMicrosoftComPtr<IDXGIFactory>         TDXGIFactoryPtr;
typedef TMicrosoftComPtr<IDXGIFactory1>        TDXGIFactory1Ptr;
typedef TMicrosoftComPtr<IDXGIKeyedMutex>      TDXGIKeyedMutexPtr;
typedef TMicrosoftComPtr<IDXGIObject>          TDXGIObjectPtr;
typedef TMicrosoftComPtr<IDXGIOutput>          TDXGIOutputPtr;
typedef TMicrosoftComPtr<IDXGIResource>        TDXGIResourcePtr;
typedef TMicrosoftComPtr<IDXGISurface1>        TDXGISurface1Ptr;
typedef TMicrosoftComPtr<IDXGIFactory4>        TDXGIFactory4Ptr;
typedef TMicrosoftComPtr<IDXGISwapChain>       TDXGISwapChainPtr;
typedef TMicrosoftComPtr<IDXGISwapChain1>      TDXGISwapChain1Ptr;
typedef TMicrosoftComPtr<IDXGISwapChain3>      TDXGISwapChain3Ptr;

// D3D11
typedef TMicrosoftComPtr<ID3D11Device>           TD3D11DevicePtr;
typedef TMicrosoftComPtr<ID3D11DeviceContext>    TD3D11DeviceContextPtr;
typedef TMicrosoftComPtr<ID3D11Texture2D>        TD3D11Texture2DPtr;
typedef TMicrosoftComPtr<ID3D11RenderTargetView> TD3D11RenderTargetViewPtr;
typedef TMicrosoftComPtr<ID3D11DepthStencilView> TD3D11DepthTargetViewPtr;
typedef TMicrosoftComPtr<ID3D11Buffer>           TD3D11BufferPtr;
typedef TMicrosoftComPtr<ID3D11VertexShader>     TD3D11VertexShaderPtr;
typedef TMicrosoftComPtr<ID3D11PixelShader>      TD3D11PixelShaderPtr;
typedef TMicrosoftComPtr<ID3D11InputLayout>      TD3D11InputLayoutPtr;

namespace NFXC
{
   bool compile(std::string const & srcData, std::string const & srcPath, char const * pEntrypoint, char const * pTarget, TMicrosoftComPtr<ID3DBlob> & pShader);
}

// =================================================================
// NRenderer
// =================================================================

extern TD3D11DevicePtr           gpDevice;
extern TD3D11DeviceContextPtr    gpDeviceCtx;
extern TDXGISwapChain1Ptr        gpSwapChain;
extern TD3D11RenderTargetViewPtr gpTargetColor;
extern TD3D11DepthTargetViewPtr  gpTargetDepth;
extern uint32_t                  gWindowWidth, gWindowHeight;

namespace NRenderer
{
   bool initialize(HWND hWnd);
   void shutdown();
}; // namespace NRenderer