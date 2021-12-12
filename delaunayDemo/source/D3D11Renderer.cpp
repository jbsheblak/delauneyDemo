
#include "D3D11Renderer.h"

#include <vector>

TD3D11DevicePtr gpDevice;
TD3D11DeviceContextPtr gpDeviceCtx;
TDXGISwapChain1Ptr gpSwapChain;
TD3D11RenderTargetViewPtr gpTargetColor;
TD3D11DepthTargetViewPtr gpTargetDepth;
uint32_t gWindowWidth, gWindowHeight;

// ======================================================================================
// FXC
// ======================================================================================

namespace NFXC
{
    typedef HRESULT(*TD3DCompile)(LPCVOID                pSrcData,
                                  SIZE_T                 SrcDataSize,
                                  LPCSTR                 pSourceName,
                                  const D3D_SHADER_MACRO *pDefines,
                                  ID3DInclude            *pInclude,
                                  LPCSTR                 pEntrypoint,
                                  LPCSTR                 pTarget,
                                  UINT                   Flags1,
                                  UINT                   Flags2,
                                  ID3DBlob               **ppCode,
                                  ID3DBlob               **ppErrorMsgs);

    typedef HRESULT(*TD3DReflect)(LPCVOID pSrcData,
                                    SIZE_T  SrcDataSize,
                                    REFIID  pInterface,
                                    void    **ppReflector);

    HMODULE spModule = nullptr;
    TD3DCompile spD3DCompile = nullptr;
    TD3DReflect spD3DReflect = nullptr;

#if 0
    void log_error_report(std::string const &errorString, std::string const & shaderSource, std::string const& shaderPath)
    {
        // parse the error buffer for line number and message information
        std::vector<SErrorLineAndMsg> errorInformation;
        {
            CStringUtils::TStringVector const errorStringLines = CStringUtils::Tokenize(errorString, "\n", CStringUtils::kAllowEmptyTokens_No);

            std::string const lineTagStartStr = "(";
            std::string const lineTagEndStr = "): error";
            std::string const lineNumEndStr = ",";
                
            errorInformation.reserve(errorStringLines.size());
            for (std::string const & errorStringLine : errorStringLines)
            {
                // example:
                // Simple.hlsl(99,23-65): error X3020: type mismatch
                    
                std::string::size_type const lineTagStart = errorStringLine.find(lineTagStartStr);
                if (lineTagStart != std::string::npos)
                {
                    std::string::size_type const lineTagEnd = errorStringLine.find(lineTagEndStr, lineTagStart);
                    if (lineTagEnd != std::string::npos)
                    {
                        std::string::size_type const lineNumEnd = errorStringLine.find(lineNumEndStr, lineTagStart);
                        if ((lineNumEnd != std::string::npos) && (lineNumEnd <= lineTagEnd))
                        {
                            std::string::size_type const lineNumStartIdx = lineTagStart + lineTagStartStr.size();
                            std::string::size_type const lineContentStartIdx = lineTagEnd + lineTagEndStr.size();
                                
                            SErrorLineAndMsg errorLineAndMsg;
                            errorLineAndMsg.mLine = CStringUtils::ParseUint32(errorStringLine.substr(lineNumStartIdx, (lineNumEnd-lineNumStartIdx)));
                            errorLineAndMsg.mMessage = errorStringLine.substr(lineContentStartIdx, errorStringLine.size()-lineContentStartIdx);
                            errorInformation.push_back(std::move(errorLineAndMsg));
                        }
                    }
                }
            }
        }

        std::string const brokenPath = "D:/BrokenCompileShader.hlsl";

        std::string brokenSource;
        {
            CStringUtils::TStringVector shaderSourceLines = CStringUtils::Tokenize(shaderSource, "\n", CStringUtils::kAllowEmptyTokens_Yes);

            // iterate backwards so we can insert information without disturbing line numbers
            for (int i = int(errorInformation.size()-1); i >= 0; --i)
            {   
                SErrorLineAndMsg const & errorInfo = errorInformation[i];
                if (errorInfo.mLine <= shaderSourceLines.size())
                {
                    shaderSourceLines.insert(shaderSourceLines.begin() + errorInfo.mLine - 1, std::string("// !!! ") + errorInfo.mMessage);
                }
            }
                    
            brokenSource += "/*\n";
            brokenSource += shaderPath;
            brokenSource += "\n";
            brokenSource += errorString;
            brokenSource += "*/\n";

            for (std::string const & shaderSourceLine : shaderSourceLines)
            {
                brokenSource += shaderSourceLine;
                brokenSource += "\n";
            }
        }

        sBrokenSourceLogger(brokenSource, errorString);
    }
#endif

    bool initialize()
    {
        if (spModule == nullptr)
        {
            static char const * skCompiler = "D3dcompiler_47.dll";

            HMODULE const pModule = LoadLibraryA(skCompiler);
            if (pModule == nullptr)
            {
                printf("Failed to load compiler dll: %s", skCompiler);
                return false;
            }

            TD3DCompile const pD3DCompile = reinterpret_cast<TD3DCompile>(GetProcAddress(pModule, "D3DCompile"));
            if (pD3DCompile == nullptr)
            {
                printf("Failed to find D3DCompile function pointer.");
                return false;
            }

            TD3DReflect const pD3DReflect = reinterpret_cast<TD3DReflect>(GetProcAddress(pModule, "D3DReflect"));
            if (pD3DReflect == nullptr)
            {
                printf("Failed to find D3DReflect function pointer.");
                return false;
            }

            spModule = pModule;
            spD3DCompile = pD3DCompile;
            spD3DReflect = pD3DReflect;
        }

        return true;
    }

    // -----------------------------------------------------------------------------------

    bool compile(std::string const &srcData,
                 std::string const &srcPath,
                 char const * pEntrypoint,
                 char const * pTarget,
                 TMicrosoftComPtr<ID3DBlob> &pShader)
    {
        if (!initialize())
        {
            return false;
        }

        TMicrosoftComPtr<ID3DBlob> pErrorMsgs;
        if (FAILED(spD3DCompile(srcData.c_str(),
                                srcData.size(),
                                srcPath.c_str(),
                                nullptr /*pDefines*/,
                                nullptr /*pIncludes*/,
                                pEntrypoint,
                                pTarget,
                                0 /*Flags1*/,
                                0 /*Flags2*/,
                                pShader.GetAddressOf(),
                                pErrorMsgs.GetAddressOf())))
        {
            char const * const pError = reinterpret_cast<char const *>(pErrorMsgs->GetBufferPointer());
            std::string errorMsg(pError, pError + pErrorMsgs->GetBufferSize()-1);
            printf("%s\n", errorMsg.c_str());
            //log_error_report(errorMsg, srcData, srcPath);
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
}

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

            gWindowWidth = colorDesc.Width;
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
};