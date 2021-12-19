
#include "Demo.h"

#include "D3D11Renderer.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <optional>
#include <DirectXMath.h>
#include "Delaunay.h"
#include "imguiwrapper.h"

// =================================================================
// NDemo
// =================================================================

namespace NDemo
{
    namespace
    {
        // --------------------------------------------------------------

        struct SVector3f { float mX, mY, mZ; };
        struct SColor { float mR, mG, mB, mA; };

        // --------------------------------------------------------------

        struct SPositionColorVertex
        {
            SVector3f mPosition;
            SColor mColor;
        };

        // --------------------------------------------------------------

        D3D11_INPUT_ELEMENT_DESC const skPositionColorIED [] =
        {
            { "POSITION", 0 /*SemanticIndex*/, DXGI_FORMAT_R32G32B32_FLOAT   , 0 /*InputSlot*/,  0 /*ByteOffset*/, D3D11_INPUT_PER_VERTEX_DATA, 0 /*InstanceStepRate*/ },
            { "COLOR"   , 0 /*SemanticIndex*/, DXGI_FORMAT_R32G32B32A32_FLOAT, 0 /*InputSlot*/, 12 /*ByteOffset*/, D3D11_INPUT_PER_VERTEX_DATA, 0 /*InstanceStepRate*/ },
        };
        constexpr uint32_t const skPositionColorStride = 28;
        constexpr uint32_t const skPositionColorOffset = 0;

        // --------------------------------------------------------------

        struct SShader
        {
            TMicrosoftComPtr<ID3DBlob>  mpVSShaderBlob;
            TMicrosoftComPtr<ID3DBlob>  mpPSShaderBlob;
            TD3D11VertexShaderPtr       mpVSShader;
            TD3D11PixelShaderPtr        mpPSShader;
        };

        // --------------------------------------------------------------

        std::string read_file_to_string(char const * path)
        {
            std::ifstream t(path);
            std::stringstream buffer;
            buffer << t.rdbuf();
            return buffer.str();
        }

        // --------------------------------------------------------------

        bool build_shader(std::string const &path, SShader &shader)
        {   
            std::string shaderSrc = read_file_to_string(path.c_str());
            std::string shaderVS = "#define VERTEX_SHADER\n" + shaderSrc;
            std::string shaderPS = "#define PIXEL_SHADER\n"  + shaderSrc;

            if (!NFXC::compile(shaderVS, path, "main", "vs_4_1", shader.mpVSShaderBlob) ||
                !NFXC::compile(shaderPS, path, "main", "ps_4_1", shader.mpPSShaderBlob))
            {
                return false;
            }

            if (FAILED(gpDevice->CreateVertexShader(shader.mpVSShaderBlob->GetBufferPointer(), shader.mpVSShaderBlob->GetBufferSize(), nullptr /*pClassLinkage*/, shader.mpVSShader.GetAddressOf())))
            {
                return false;
            }

            if (FAILED(gpDevice->CreatePixelShader(shader.mpPSShaderBlob->GetBufferPointer(), shader.mpPSShaderBlob->GetBufferSize(), nullptr /*pClassLinkage*/, shader.mpPSShader.GetAddressOf())))
            {
                return false;
            }

            return true;
        }

        // --------------------------------------------------------------

        TD3D11BufferPtr build_vertex_buffer(SPositionColorVertex const *pVerts, uint32_t const vertCount)
        {
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = sizeof(SPositionColorVertex) * vertCount;
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA initialData = {};
            initialData.pSysMem = pVerts;

            TD3D11BufferPtr pVertexBuffer;
            if (FAILED(gpDevice->CreateBuffer(&bufferDesc, &initialData, pVertexBuffer.ReleaseAndGetAddressOf())))
            {
                return TD3D11BufferPtr();
            }
            
            return pVertexBuffer;
        }

        // --------------------------------------------------------------

        TD3D11BufferPtr build_index_buffer(uint32_t const *pIndices, UINT32 const indexCount)
        {
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = sizeof(uint32_t) * indexCount;
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA initialData = {};
            initialData.pSysMem = pIndices;

            TD3D11BufferPtr pIndexBuffer;
            if (FAILED(gpDevice->CreateBuffer(&bufferDesc, &initialData, pIndexBuffer.ReleaseAndGetAddressOf())))
            {
                return TD3D11BufferPtr();
            }
            
            return pIndexBuffer;
        }

        // --------------------------------------------------------------

        TD3D11BufferPtr build_constant_buffer(uint32_t const size)
        {
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = size;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            TD3D11BufferPtr pConstantBuffer;
            if (FAILED(gpDevice->CreateBuffer(&bufferDesc, nullptr /*initialData*/, pConstantBuffer.ReleaseAndGetAddressOf())))
            {
                return TD3D11BufferPtr();
            }
            
            return pConstantBuffer;
        }

        // ==============================================================
        // SMeshData
        // ==============================================================
        
        struct SMeshData
        {
            std::vector<SPositionColorVertex>           mVertices;
            std::vector<uint32_t>                       mIndices;
            std::vector<uint32_t>                       mTriangulatedIndices;

            TD3D11BufferPtr                             mpVertexBuffer;
            TD3D11BufferPtr                             mpIndexBuffer;
        };

        // --------------------------------------------------------------

        void rebuild_index_buffer_from(SMeshData &mesh, std::vector<uint32_t> const &indices)
        {
            if (indices.empty())
            {
                mesh.mpIndexBuffer.Reset();
            }
            else
            {
                mesh.mpIndexBuffer = build_index_buffer(indices.data(), UINT32(indices.size()));
            }
        }

        // --------------------------------------------------------------

        void retriangulate(SMeshData &mesh)
        {
            mesh.mTriangulatedIndices = mesh.mIndices;
            
            // round the indices down to a multiple of 3
            uint32_t const indexCount = uint32_t(mesh.mTriangulatedIndices.size());
            uint32_t const indicesToTriangulate = (indexCount/3)*3;

        #if 1
            if (indicesToTriangulate > 0)
            {
                NDelaunay::retriangulate_with_edge_flip(mesh.mTriangulatedIndices.data(),
                                         indicesToTriangulate,
                                         mesh.mVertices.data(),
                                         uint32_t(sizeof(SPositionColorVertex)));
            }
        #endif
        }

        // ==============================================================
        // SDemoData
        // ==============================================================

        struct SDemoData
        {
            SShader                                     mShader;
            TD3D11InputLayoutPtr                        mpInputLayout;            
            TD3D11BufferPtr                             mpConstantBuffer;
            TMicrosoftComPtr<ID3D11RasterizerState>     mpRasterState;
            SMeshData                                   mMesh;
        };

        SDemoData sData;

        // --------------------------------------------------------------
    }

    // --------------------------------------------------------------

    bool initialize()
    {
        // setup shader
        {
            if (!build_shader("shader.hlsl", sData.mShader))
            {
                return false;
            }

            if FAILED(gpDevice->CreateInputLayout(skPositionColorIED, 
                                                  sizeof(skPositionColorIED) / sizeof(skPositionColorIED[0]),
                                                  sData.mShader.mpVSShaderBlob->GetBufferPointer(),
                                                  sData.mShader.mpVSShaderBlob->GetBufferSize(),
                                                  sData.mpInputLayout.GetAddressOf()))
            {
                return false;
            }
        }

        // setup state
        {
            D3D11_RASTERIZER_DESC rasterDesc = {};
            rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
            rasterDesc.CullMode = D3D11_CULL_NONE;

            if (FAILED(gpDevice->CreateRasterizerState(&rasterDesc, sData.mpRasterState.GetAddressOf())))
            {
                return false;
            }
        }

        // setup buffers
        {
            sData.mpConstantBuffer = build_constant_buffer(sizeof(float) * 16);
        }

        return true;
    }

    // --------------------------------------------------------------

    void shutdown()
    {
    }

    // --------------------------------------------------------------

    void simulate()
    {
    }

    // --------------------------------------------------------------

    void render()
    {
        imguiwrapper::new_frame();

        // setup data
        {
            D3D11_MAPPED_SUBRESOURCE mapping;
            gpDeviceCtx->Map(sData.mpConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapping);
            {
                DirectX::XMMATRIX const proj = DirectX::XMMatrixOrthographicOffCenterRH(0 /*viewLeft*/,
                                                                                        float(gWindowWidth) /*viewRight*/,
                                                                                        0 /*viewBottom*/,
                                                                                        float(gWindowHeight) /*viewTop*/,
                                                                                        -1.0f /*zNear*/,
                                                                                        +1.0f /*zFar*/);

                memcpy(mapping.pData, &proj, sizeof(proj));
            }
            gpDeviceCtx->Unmap(sData.mpConstantBuffer.Get(), 0);
        }

        // clear screen
        {
            float clearColor [] = {0.0f, 0.0f, 0.0f, 1.0f};
            gpDeviceCtx->ClearRenderTargetView(gpTargetColor.Get(), clearColor);
            gpDeviceCtx->ClearDepthStencilView(gpTargetDepth.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        }

        // draw
        {
            D3D11_VIEWPORT viewport = {};
            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            viewport.Width = float(gWindowWidth);
            viewport.Height = float(gWindowHeight);
            gpDeviceCtx->RSSetViewports(1, &viewport);
            gpDeviceCtx->RSSetState(sData.mpRasterState.Get());

            gpDeviceCtx->OMSetRenderTargets(1, gpTargetColor.GetAddressOf(), gpTargetDepth.Get());
            gpDeviceCtx->IASetInputLayout(sData.mpInputLayout.Get());            
            gpDeviceCtx->IASetVertexBuffers(0 /*startSlot*/, 1 /*numBuffers*/, sData.mMesh.mpVertexBuffer.GetAddressOf(), &skPositionColorStride, &skPositionColorOffset);
            gpDeviceCtx->IASetIndexBuffer(sData.mMesh.mpIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

            gpDeviceCtx->VSSetConstantBuffers(0 /*startSlot*/, 1 /*numBuffers*/, sData.mpConstantBuffer.GetAddressOf());
            gpDeviceCtx->VSSetShader(sData.mShader.mpVSShader.Get(), nullptr, 0);

            gpDeviceCtx->PSSetConstantBuffers(0 /*startSlot*/, 1 /*numBuffers*/, sData.mpConstantBuffer.GetAddressOf());
            gpDeviceCtx->PSSetShader(sData.mShader.mpPSShader.Get(), nullptr, 0);

            assert((sData.mMesh.mIndices.size() == sData.mMesh.mTriangulatedIndices.size()) && "invalid index count");
            uint32_t const vertCount = uint32_t(sData.mMesh.mIndices.size());
            uint32_t const triCount = vertCount/3;
            uint32_t const remCount = vertCount%3;

            // render any full triangles
            if (triCount > 0)
            {
                gpDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                gpDeviceCtx->DrawIndexed(triCount*3, 0 /*startIndexLocation*/, 0 /*baseVertexLocation*/);
            }

            // render the remaining wireframe
            if (remCount >= 2)
            {
                gpDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
                gpDeviceCtx->DrawIndexed(remCount, triCount*3, 0 /*baseVertexLocation*/);
            }

            // render all the points
            if (vertCount >= 1)
            {
                gpDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
                gpDeviceCtx->DrawIndexed(vertCount, 0, 0 /*baseVertexLocation*/);
            }
        }

        ImGui::Begin("Hello World");
        ImGui::Text("Hello, world %d", 123);
        ImGui::End();

        imguiwrapper::render();

        gpSwapChain->Present(1, 0);
    }

    // --------------------------------------------------------------

    void add_mouse_click(int32_t const mouseX, int32_t const mouseY)
    {
        static float const skColors [] =
        {
            0.5f, 0.5f, 0.5f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f,
        };

        uint32_t const clickIdx = uint32_t(sData.mMesh.mVertices.size()) % 3;
        float const *pColor = &skColors[0] + (clickIdx * 4);

        float const mouseXF = float(mouseX);
        float const mouseYF = float(gWindowHeight - mouseY);

        int32_t index = -1;
        for (int32_t i = 0; i < sData.mMesh.mVertices.size(); ++i)
        {
            static constexpr float const skMaxDistance = 20;
            static constexpr float const skMaxDistanceSqr = skMaxDistance * skMaxDistance;

            SPositionColorVertex const &v = sData.mMesh.mVertices[i];
            float const distX = v.mPosition.mX - mouseXF;
            float const distY = v.mPosition.mY - mouseYF;

            if ( (distX * distX + distY * distY) <= skMaxDistanceSqr )
            {
                index = i;
                break;
            }
        }

        if (index == -1)
        {
            SPositionColorVertex vtx;
            vtx.mPosition = {mouseXF, mouseYF, 0.0f};
            vtx.mColor = {pColor[0], pColor[1], pColor[2], pColor[3]};
            sData.mMesh.mVertices.push_back(vtx);
            sData.mMesh.mpVertexBuffer = build_vertex_buffer(sData.mMesh.mVertices.data(), uint32_t(sData.mMesh.mVertices.size()));
            index = int32_t(sData.mMesh.mVertices.size()-1);
        }

        sData.mMesh.mIndices.push_back(index);

        retriangulate(sData.mMesh);
        rebuild_index_buffer_from(sData.mMesh, sData.mMesh.mTriangulatedIndices);
    }

    // --------------------------------------------------------------

    void clear_mouse_clicks()
    {
        sData.mMesh = SMeshData();
    }

    // --------------------------------------------------------------

    void remove_last_triangle()
    {
        // undo last primitive if possible
        if (!sData.mMesh.mIndices.empty())
        {
            sData.mMesh.mIndices.pop_back();
        }
        // otherwise, clear the data fully
        else
        {
            sData.mMesh.mIndices.clear();
            sData.mMesh.mVertices.clear();
        }

        retriangulate(sData.mMesh);
        rebuild_index_buffer_from(sData.mMesh, sData.mMesh.mTriangulatedIndices);
    }

    // --------------------------------------------------------------

    void save_data()
    {
        std::ofstream outStream("mesh.bin", std::ios::out);
        if (!outStream)
        {
            return;
        }

        constexpr uint32_t const skVersion = 1;
        outStream << skVersion << std::endl;

        outStream << uint32_t(sData.mMesh.mIndices.size()) << std::endl;
        for (uint32_t const index : sData.mMesh.mIndices)
        {
            outStream << index << std::endl;
        }

        outStream << uint32_t(sData.mMesh.mVertices.size()) << std::endl;
        for (SPositionColorVertex const& v : sData.mMesh.mVertices)
        {
            outStream << v.mPosition.mX << " " << v.mPosition.mY << " " << v.mPosition.mZ << std::endl;
            outStream << v.mColor.mR << " " << v.mColor.mG << " " << v.mColor.mB << " " << v.mColor.mA << std::endl;
        }
    }

    // --------------------------------------------------------------

    void read_data()
    {
        std::ifstream inStream("mesh.bin", std::ios::in);
        if (!inStream)
        {
            return;
        }

        uint32_t version = 0;
        inStream >> version;

        uint32_t indexCount = 0;
        inStream >> indexCount;

        sData.mMesh.mIndices.clear();
        sData.mMesh.mIndices.resize(indexCount);
        for (uint32_t &idx : sData.mMesh.mIndices)
        {
            inStream >> idx;
        }

        uint32_t vertexCount = 0;
        inStream >> vertexCount;

        sData.mMesh.mVertices.clear();
        sData.mMesh.mVertices.resize(vertexCount);
        for (SPositionColorVertex &v : sData.mMesh.mVertices)
        {   
            inStream >> v.mPosition.mX >> v.mPosition.mY >> v.mPosition.mZ;
            inStream >> v.mColor.mR >> v.mColor.mG >> v.mColor.mB >> v.mColor.mA;
        }

        retriangulate(sData.mMesh);

        if (!sData.mMesh.mVertices.empty())
        {
            sData.mMesh.mpVertexBuffer = build_vertex_buffer(sData.mMesh.mVertices.data(), uint32_t(sData.mMesh.mVertices.size()));
        }

        if (!sData.mMesh.mIndices.empty())
        {
            rebuild_index_buffer_from(sData.mMesh, sData.mMesh.mTriangulatedIndices);
        }
    }

    // --------------------------------------------------------------
}