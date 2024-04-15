//The Linker shoulde be Win32 Type
//Check project configuration. Linker -> System -> SubSystem should be Windows ¨C Michael Nastenko May 3, 2016 at 3:25
//https://stackoverflow.com/questions/33400777/error-lnk2019-unresolved-external-symbol-main-referenced-in-function-int-cde

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>
#include "d3dx12.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

HWND hwnd = nullptr;
HINSTANCE mhAppInst = nullptr;
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


// Pipeline objects.
CD3DX12_VIEWPORT m_viewport(0.0f,0.0f,1280.0f,720.0f, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);

//https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-viewport
//https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_viewport
//D3D12_VIEWPORT viewport;

//https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-rect
//https://learn.microsoft.com/en-us/windows/win32/direct3d12/d3d12-rect
CD3DX12_RECT m_scissorRect(0, 0, 1280, 720);
//D3D12_RECT scissorRect;

static const UINT FrameCount = 2;

ComPtr<IDXGIFactory4> factory;
ComPtr<IDXGISwapChain3> m_swapChain;
ComPtr<ID3D12Device> m_device;
ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
ComPtr<ID3D12CommandAllocator> m_commandAllocator;
ComPtr<ID3D12CommandQueue> m_commandQueue;
ComPtr<ID3D12RootSignature> m_rootSignature;
ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
ComPtr<ID3D12PipelineState> m_pipelineState;
ComPtr<ID3D12GraphicsCommandList> m_commandList;
UINT m_rtvDescriptorSize = 0;

// App resources.
ComPtr<ID3D12Resource> m_vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

// Synchronization objects.
UINT m_frameIndex = 0;;
HANDLE m_fenceEvent;
ComPtr<ID3D12Fence> m_fence;
UINT64 m_fenceValue;

void OnRender();
void LoadPipeLine();

void CreateDXGI();
void CreateFence();
void CreateCommandObjects();
void CreateSwapChain();
void CreateDescriptorHeaps();
void CreateRTV();


void LoadAssets();

void InitMainWindow();


//void BuildDescriptorHeaps();
//void BuildConstantBuffers();
void BuildRootSignature();

ComPtr<ID3DBlob> vertexShader;
ComPtr<ID3DBlob> pixelShader;
std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;
void BuildShadersAndInputLayout();
void BuildGeometry();
void BuildPSO();


void WaitForPreviousFrame();


void ResetCommandList();
void SetCommandList();
void DrawCommandList();
void ExecuteCommandList();
void PopulateCommandList();

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT4 color;
};

#pragma comment(lib,"D3Dcompiler.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    InitMainWindow();

    LoadPipeLine();
    LoadAssets();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }


}

// Main message handler for the sample.
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    case WM_PAINT:      
            OnRender();
            return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    }
    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void InitMainWindow()
{
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = mhAppInst;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"LearnSingleTriangle";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, 1280, 720 };

    hwnd = CreateWindow(
        windowClass.lpszClassName,
        L"02.LearnSingleTriangle",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        mhAppInst,
        nullptr);

    ShowWindow(hwnd, SW_SHOW);
}

void CreateDXGI()
{
    UINT dxgiFactoryFlags = 0;

    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

    D3D12CreateDevice(
        nullptr,  //Default adapter
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device));
}

void CreateFence()
{
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    m_fenceValue = 1;
}

void CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList));
    m_commandList->Close();

}

void CreateSwapChain()
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = 1280;
    swapChainDesc.Height = 720;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    );

    // This sample does not support fullscreen transitions.
   // factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    swapChain.As(&m_swapChain);
}

void CreateDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void CreateRTV()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    //https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-cpu-descriptor-handle
    //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_cpu_descriptor_handle

   // D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
   // rtvHandle.ptr = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

    // Create a RTV for each frame.
    for (UINT n = 0; n < FrameCount; n++)
    {
        m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }
}

void LoadPipeLine()
{
        CreateDXGI();
        CreateFence();
        CreateCommandObjects();
        CreateSwapChain();
        CreateDescriptorHeaps();
        CreateRTV();

}

void BuildRootSignature()
{
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
}

void BuildShadersAndInputLayout()
{

    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
    D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

    // Define the vertex input layout.
    inputElementDescs =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void BuildGeometry()
{
    Vertex triangleVertices[] =
    {
        { { 0.0f, 0.25f , 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.25f, -0.25f , 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.25f, -0.25f , 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    const UINT vertexBufferSize = sizeof(triangleVertices);

    // Note: using upload heaps to transfer static data like vert buffers is not 
    // recommended. Every time the GPU needs it, the upload heap will be marshalled 
    // over. Please read up on Default Heap usage. An upload heap is used here for 
    // code simplicity and because there are very few verts to actually transfer.

    auto heapUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto buffer = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
    m_device->CreateCommittedResource(
        &heapUpload,
        D3D12_HEAP_FLAG_NONE,
        &buffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer));

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
}

//PSO : Pipeline State Objects
void BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs.data(), (UINT)inputElementDescs.size() };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
}

void LoadAssets()
{

    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildGeometry();
    BuildPSO();

}

void WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    m_commandQueue->Signal(m_fence.Get(), fence);
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
        m_fence->SetEventOnCompletion(fence, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void OnRender()
{
    ResetCommandList();
    SetCommandList();
    DrawCommandList();
    ExecuteCommandList();

    //PopulateCommandList();

    //// Execute the command list.
    //ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    //m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    //// Present the frame.
    //m_swapChain->Present(1, 0);

    WaitForPreviousFrame();
}

void ResetCommandList()
{
    // Command list allocators can only be reset when the associated 
// command lists have finished execution on the GPU; apps should use 
// fences to determine GPU execution progress.
    m_commandAllocator->Reset();

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());
}

void SetCommandList()
{
    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);
    
    // Indicate that the back buffer will be used as a render target.
    auto trans = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &trans);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

     const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
     m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr); //Screen background color will be black if remove this

}

void DrawCommandList()
{
    // Record commands.
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->DrawInstanced(3, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &barrier);

    m_commandList->Close();
}

void ExecuteCommandList()
{
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    m_swapChain->Present(1, 0);
}

void PopulateCommandList()
{

   // // Command list allocators can only be reset when the associated 
   //// command lists have finished execution on the GPU; apps should use 
   //// fences to determine GPU execution progress.
   // m_commandAllocator->Reset();

   // // However, when ExecuteCommandList() is called on a particular command 
   // // list, that command list can then be reset at any time and must be before 
   // // re-recording.
   // m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());
    ResetCommandList();

    //// Set necessary state.
    //m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    //m_commandList->RSSetViewports(1, &m_viewport);
    //m_commandList->RSSetScissorRects(1, &m_scissorRect);

    //// Indicate that the back buffer will be used as a render target.
    //auto trans = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    //m_commandList->ResourceBarrier(1, &trans);

    //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    //m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    SetCommandList();

    // Record commands.
   // const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
   // m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->DrawInstanced(3, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &barrier);

    m_commandList->Close();

}