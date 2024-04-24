#pragma once
#include <windows.h>
#include <string>
#include <iostream>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include "d3dx12.h"

#include <wrl/client.h> // ComPtr

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class D3D12_OBJECT
{
public:
	D3D12_OBJECT(UINT width, UINT height);

    virtual void OnInit();
    virtual void OnRender();
    virtual void OnUpdate();
    virtual void OnDestroy();

    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    struct SceneConstantBuffer
    {
        XMFLOAT4 offset;
        float padding[60]; // Padding so the constant buffer is 256-byte aligned.
    };

protected:
    UINT m_width;
    UINT m_height;

private:
    static const UINT FrameCount = 2;
    static const UINT TextureWidth = 256;
    static const UINT TextureHeight = 256;
    static const UINT TexturePixelSize = 4;    // The number of bytes used to represent a pixel in the texture.

    

    // Pipeline objects.
    // https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-viewport
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_viewport



   // CD3DX12_VIEWPORT m_viewport;
    D3D12_VIEWPORT m_viewport;
   // CD3DX12_RECT m_scissorRect;
    D3D12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    
    ComPtr<ID3D12Resource> m_constantBuffer;
    SceneConstantBuffer m_constantBufferData;
    UINT8* m_pCbvDataBegin;

    ComPtr<ID3D12Resource> m_texture;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;
public:
    HWND m_hwnd;
    //not static m_hwnd !!!!!!!!
private:


    //Introduction to 3D Game Programming with d3d12 style
    //TO DO
    void LoadPipeline();
    void LoadAssets();
    std::vector<UINT8> GenerateTextureData();
    void PopulateCommandList();
    void WaitForPreviousFrame();

};