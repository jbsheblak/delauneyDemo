
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

#if 0
// D3D12
typedef TMicrosoftComPtr<ID3D12CommandAllocator> TD3D12CommandAllocatorPtr;
typedef TMicrosoftComPtr<ID3D12CommandList> TD3D12CommandListPtr;
typedef TMicrosoftComPtr<ID3D12CommandQueue> TD3D12CommandQueuePtr;
typedef TMicrosoftComPtr<ID3D12CommandSignature> TD3D12CommandSignaturePtr;
typedef TMicrosoftComPtr<ID3D12DescriptorHeap> TD3D12DescriptorHeapPtr;
typedef TMicrosoftComPtr<ID3D12Device> TD3D12DevicePtr;
typedef TMicrosoftComPtr<ID3D12Device1> TD3D12Device1Ptr;
typedef TMicrosoftComPtr<ID3D12Device2> TD3D12Device2Ptr;
typedef TMicrosoftComPtr<ID3D12Device3> TD3D12Device3Ptr;
typedef TMicrosoftComPtr<ID3D12Device4> TD3D12Device4Ptr;
typedef TMicrosoftComPtr<ID3D12Device5> TD3D12Device5Ptr;
typedef TMicrosoftComPtr<ID3D12Device5> TD3D12Device5Ptr;
typedef TMicrosoftComPtr<ID3D12Device6> TD3D12Device6Ptr;
typedef TMicrosoftComPtr<ID3D12DeviceChild> TD3D12DeviceChildPtr;
typedef TMicrosoftComPtr<ID3D12Fence> TD3D12FencePtr;
typedef TMicrosoftComPtr<ID3D12GraphicsCommandList> TD3D12GraphicsCommandListPtr;
typedef TMicrosoftComPtr<ID3D12GraphicsCommandList1> TD3D12GraphicsCommandList1Ptr;
typedef TMicrosoftComPtr<ID3D12GraphicsCommandList2> TD3D12GraphicsCommandList2Ptr;
typedef TMicrosoftComPtr<ID3D12GraphicsCommandList3> TD3D12GraphicsCommandList3Ptr;
typedef TMicrosoftComPtr<ID3D12GraphicsCommandList4> TD3D12GraphicsCommandList4Ptr;
typedef TMicrosoftComPtr<ID3D12GraphicsCommandList5> TD3D12GraphicsCommandList5Ptr;
typedef TMicrosoftComPtr<ID3D12GraphicsCommandList6> TD3D12GraphicsCommandList6Ptr;
typedef TMicrosoftComPtr<ID3D12Heap> TD3D12HeapPtr;
typedef TMicrosoftComPtr<ID3D12Object> TD3D12ObjectPtr;
typedef TMicrosoftComPtr<ID3D12Pageable> TD3D12PageablePtr;
typedef TMicrosoftComPtr<ID3D12PipelineState> TD3D12PipelineStatePtr;
typedef TMicrosoftComPtr<ID3D12QueryHeap> TD3D12QueryHeapPtr;
typedef TMicrosoftComPtr<ID3D12Resource> TD3D12ResourcePtr;
typedef TMicrosoftComPtr<ID3D12RootSignature> TD3D12RootSignaturePtr;
typedef TMicrosoftComPtr<ID3D12RootSignatureDeserializer> TD3D12RootSignatureDeserializerPtr;

typedef TMicrosoftComPtr<ID3D12FunctionParameterReflection> TD3D12FunctionParameterReflectionPtr;
typedef TMicrosoftComPtr<ID3D12FunctionReflection> TD3D12FunctionReflectionPtr;
typedef TMicrosoftComPtr<ID3D12LibraryReflection> TD3D12LibraryReflectionPtr;
typedef TMicrosoftComPtr<ID3D12ShaderReflection> TD3D12ShaderReflectionPtr;
typedef TMicrosoftComPtr<ID3D12ShaderReflectionConstantBuffer> TD3D12ShaderReflectionConstantBufferPtr;
typedef TMicrosoftComPtr<ID3D12ShaderReflectionType> TD3D12ShaderReflectionTypePtr;
typedef TMicrosoftComPtr<ID3D12ShaderReflectionVariable> TD3D12ShaderReflectionVariablePtr;

typedef TMicrosoftComPtr<ID3D12Debug> TD3D12DebugPtr;
typedef TMicrosoftComPtr<ID3D12Debug1> TD3D12Debug1Ptr;
typedef TMicrosoftComPtr<ID3D12DebugDevice> TD3D12DebugDevicePtr;
typedef TMicrosoftComPtr<ID3D12DebugCommandList> TD3D12DebugCommandListPtr;
typedef TMicrosoftComPtr<ID3D12DebugCommandQueue> TD3D12DebugCommandQueuePtr;
typedef TMicrosoftComPtr<ID3D12InfoQueue> TD3D12InfoQueuePtr;
#endif

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