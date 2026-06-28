
#include "D3D11Renderer.h"

#include <vector>

TD3D11DevicePtr           gpDevice;
TD3D11DeviceContextPtr    gpDeviceCtx;
TDXGISwapChain1Ptr        gpSwapChain;
TD3D11RenderTargetViewPtr gpTargetColor;
TD3D11DepthTargetViewPtr  gpTargetDepth;
uint32_t                  gWindowWidth, gWindowHeight;

// ======================================================================================
// FXC
// ======================================================================================

namespace NFXC
{
   typedef HRESULT (*TD3DCompile)(LPCVOID                  pSrcData,
                                  SIZE_T                   SrcDataSize,
                                  LPCSTR                   pSourceName,
                                  const D3D_SHADER_MACRO * pDefines,
                                  ID3DInclude *            pInclude,
                                  LPCSTR                   pEntrypoint,
                                  LPCSTR                   pTarget,
                                  UINT                     Flags1,
                                  UINT                     Flags2,
                                  ID3DBlob **              ppCode,
                                  ID3DBlob **              ppErrorMsgs);

   typedef HRESULT (*TD3DReflect)(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void ** ppReflector);

   HMODULE     spModule     = nullptr;
   TD3DCompile spD3DCompile = nullptr;
   TD3DReflect spD3DReflect = nullptr;

   bool initialize()
   {
      if ( spModule == nullptr )
      {
         static char const * skCompiler = "D3dcompiler_47.dll";

         HMODULE const pModule = LoadLibraryA(skCompiler);
         if ( pModule == nullptr )
         {
            printf("Failed to load compiler dll: %s", skCompiler);
            return false;
         }

         TD3DCompile const pD3DCompile = reinterpret_cast<TD3DCompile>(GetProcAddress(pModule, "D3DCompile"));
         if ( pD3DCompile == nullptr )
         {
            printf("Failed to find D3DCompile function pointer.");
            return false;
         }

         TD3DReflect const pD3DReflect = reinterpret_cast<TD3DReflect>(GetProcAddress(pModule, "D3DReflect"));
         if ( pD3DReflect == nullptr )
         {
            printf("Failed to find D3DReflect function pointer.");
            return false;
         }

         spModule     = pModule;
         spD3DCompile = pD3DCompile;
         spD3DReflect = pD3DReflect;
      }

      return true;
   }

   // -----------------------------------------------------------------------------------

   bool compile(std::string const & srcData, std::string const & srcPath, char const * pEntrypoint, char const * pTarget, TMicrosoftComPtr<ID3DBlob> & pShader)
   {
      if ( !initialize() )
      {
         return false;
      }

      TMicrosoftComPtr<ID3DBlob> pErrorMsgs;
      if ( FAILED(spD3DCompile(srcData.c_str(),
                               srcData.size(),
                               srcPath.c_str(),
                               nullptr /*pDefines*/,
                               nullptr /*pIncludes*/,
                               pEntrypoint,
                               pTarget,
                               0 /*Flags1*/,
                               0 /*Flags2*/,
                               pShader.GetAddressOf(),
                               pErrorMsgs.GetAddressOf())) )
      {
         char const * const pError = reinterpret_cast<char const *>(pErrorMsgs->GetBufferPointer());
         std::string        errorMsg(pError, pError + pErrorMsgs->GetBufferSize() - 1);
         printf("%s\n", errorMsg.c_str());
         return false;
      }

      return true;
   }

   // -----------------------------------------------------------------------------------

#if 0
    bool reflect(TMicrosoftComPtr<ID3DBlob> &pShader,
                    TMicrosoftComPtr<ID3D12ShaderReflection> &pReflector)
    {
        if (!initialize())
        {
            return false;
        }

        if (FAILED(spD3DReflect(pShader->GetBufferPointer(),
                                pShader->GetBufferSize(),
                                IID_PPV_ARGS(pReflector.GetAddressOf()))))
        {
            return false;
        }

        return true;
    }
#endif
} // namespace NFXC

// =================================================================
// NRenderer
// =================================================================

namespace NRenderer
{
   // --------------------------------------------------------------

   bool initialize(HWND hWnd)
   {
      TDXGIFactory1Ptr pDXGIFactory;
      if ( FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory))) )
      {
         return false;
      }

      std::vector<IDXGIAdapter1 *> availableAdapters;
      {
         UINT            adapterIdx = 0;
         IDXGIAdapter1 * pAdapter   = nullptr;

         while ( pDXGIFactory->EnumAdapters1(adapterIdx, &pAdapter) != DXGI_ERROR_NOT_FOUND )
         {
            if ( pAdapter != nullptr )
            {
               availableAdapters.push_back(pAdapter);
            }

            adapterIdx++;
         }
      }

      TD3D11DevicePtr        pDevice;
      TD3D11DeviceContextPtr pDeviceCtx;
      IDXGIAdapter1 *        pBestAdapter = nullptr;

      constexpr D3D_FEATURE_LEVEL const kRequestedFeatureLevel = D3D_FEATURE_LEVEL_11_1;

      UINT const createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG;

      for ( IDXGIAdapter1 * pAdapter : availableAdapters )
      {
         D3D_FEATURE_LEVEL featureLevel;
         HRESULT           hr = ::D3D11CreateDevice(pAdapter,
                                                    D3D_DRIVER_TYPE_UNKNOWN,
                                                    NULL, /*software*/
                                                    createDeviceFlags,
                                                    &kRequestedFeatureLevel,
                                                    1 /*featureLevelArrayCount*/,
                                                    D3D11_SDK_VERSION,
                                                    &pDevice,
                                                    &featureLevel,
                                                    &pDeviceCtx);

         if ( FAILED(hr) )
         {
            continue;
         }

         if ( featureLevel != kRequestedFeatureLevel )
         {
            continue;
         }

         if ( !pDevice || !pDeviceCtx )
         {
            continue;
         }

         gpDevice     = pDevice;
         gpDeviceCtx  = pDeviceCtx;
         pBestAdapter = pAdapter;
         break;
      }

      if ( !gpDevice || !gpDeviceCtx )
      {
         return false;
      }

      TDXGIFactory4Ptr pDXGIFactory4;
      if ( FAILED(pBestAdapter->GetParent(IID_PPV_ARGS(&pDXGIFactory4))) )
      {
         return false;
      }

      DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
      swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      swapChainDesc.BufferCount           = 2;
      swapChainDesc.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;
      swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      swapChainDesc.SampleDesc.Count      = 1;

      TDXGISwapChain1Ptr pSwapChain;
      if ( FAILED(pDXGIFactory4->CreateSwapChainForHwnd(gpDevice.Get(),
                                                        hWnd,
                                                        &swapChainDesc,
                                                        nullptr /*fullscreen desc*/,
                                                        nullptr /*pRestrictToOutput*/,
                                                        &pSwapChain)) )
      {
         return false;
      }

      gpSwapChain = pSwapChain;

      // setup render target
      {
         TD3D11Texture2DPtr pSwapColor;
         if ( FAILED(gpSwapChain->GetBuffer(0 /*buffer*/, IID_PPV_ARGS(&pSwapColor))) )
         {
            return false;
         }

         D3D11_RENDER_TARGET_VIEW_DESC rtView = {};
         rtView.Format                        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
         rtView.ViewDimension                 = D3D11_RTV_DIMENSION_TEXTURE2D;
         rtView.Texture2D.MipSlice            = 0;

         if ( FAILED(gpDevice->CreateRenderTargetView(pSwapColor.Get(), &rtView /*pDesc*/, &gpTargetColor)) )
         {
            return false;
         }

         D3D11_TEXTURE2D_DESC colorDesc = {};
         pSwapColor->GetDesc(&colorDesc);

         D3D11_TEXTURE2D_DESC depthDesc = {};
         depthDesc.Width                = colorDesc.Width;
         depthDesc.Height               = colorDesc.Height;
         depthDesc.ArraySize            = 1;
         depthDesc.Format               = DXGI_FORMAT_D32_FLOAT;
         depthDesc.SampleDesc.Count     = 1;
         depthDesc.Usage                = D3D11_USAGE_DEFAULT;
         depthDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL;
         depthDesc.CPUAccessFlags       = 0;
         depthDesc.MiscFlags            = 0;

         TD3D11Texture2DPtr pSwapDepth;
         if ( FAILED(gpDevice->CreateTexture2D(&depthDesc, nullptr /*initialData*/, &pSwapDepth)) )
         {
            return false;
         }

         if ( FAILED(gpDevice->CreateDepthStencilView(pSwapDepth.Get(), nullptr /*pDesc*/, &gpTargetDepth)) )
         {
            return false;
         }

         gWindowWidth  = colorDesc.Width;
         gWindowHeight = colorDesc.Height;
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
}; // namespace NRenderer