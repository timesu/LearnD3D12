#include "Square.h"

#include "DirectXColors.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

#include  "d3dx12.h"
#pragma comment(lib, "dxguid.lib")

struct Vertex
{
    XMFLOAT3 Pos;
    //XMFLOAT4 Color;
    XMFLOAT2 uv;
};

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before mhMainWnd is valid.
    return Square::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

Square* Square::mApp = nullptr;
Square* Square::GetApp()
{
    return mApp;
}

Square::Square(HINSTANCE hInstance)
    : mhAppInst(hInstance)
{
    // Only one D3DApp can be constructed.
    assert(mApp == nullptr);
    mApp = this;
}

LRESULT Square::MsgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
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
    return DefWindowProc(hwnd, message, wParam, lParam);
}

bool Square::InitMainWindow()
{
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = MainWndProc;
    windowClass.hInstance = mhAppInst;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"Test";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, 1280, 720 };

    mhMainWnd = CreateWindow(
        windowClass.lpszClassName,
        L"000.Test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        mhAppInst,
        nullptr);

    ShowWindow(mhMainWnd, SW_SHOW);
    return true;

}

void Square::LoadPipeLine()
{

      /*  CreateDXGI();
        CreateFence();
        CreateCommandObjects();
        CreateSwapChain();
        CreateDescriptorHeaps();
        CreateRTV();*/
        TestCreate();

}

void Square::TestCreate()
{
    UINT dxgiFactoryFlags = 0;
    ComPtr<IDXGIFactory4> factory;

    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

    ComPtr<IDXGIAdapter> warpAdapter;
    factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

    D3D12CreateDevice(
        warpAdapter.Get(),  //Default adapter
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device));

    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    m_fenceValue = 1;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    //Swap Chain
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
        mhMainWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    );

    // This sample does not support fullscreen transitions.
   // factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    factory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_ALT_ENTER);
    swapChain.As(&m_swapChain);
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    //
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

    //
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    // Create a RTV for each frame.
    for (UINT n = 0; n < FrameCount; n++)
    {
        m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.ptr = SIZE_T(rtvHandle.ptr + 1 * m_rtvDescriptorSize);
        //  rtvHandle.Offset(1, m_rtvDescriptorSize);
    }
    m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));

}

void Square::LoadAssets()
{
        BuildRootSignature();
        BuildShadersAndInputLayout();
        BuildPSO();
        BuildGeometry();    

}

void Square::BuildRootSignature()
{

    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
}

void Square::BuildShadersAndInputLayout()
{
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    D3DCompileFromFile(L"000.test.shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
    D3DCompileFromFile(L"000.test.shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

    // Define the vertex input layout.
    inputElementDescs =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void Square::BuildPSO()
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
    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList));

    //????????
    /*m_commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);*/
}

void Square::BuildGeometry()
{
    Vertex squareVertices[] =
    {

        { { 0.25f, 0.25f , 0.0f }, { 0.0f, 1.0f } },
            { { 0.25f, -0.25f , 0.0f }, { 0.0f, 0.0f } },
            { { -0.25f, -0.25f , 0.0f }, { 1.0f, 0.0f } },
            { { -0.25f, 0.25f , 0.0f }, { 1.0f, 1.0f } }
    };

    uint16_t squareIndices[] =
    {
        0, 1, 2,
        2,3,0
    };

   // const UINT vertexBufferSize = sizeof(triangleVertices);
    const UINT vertexBufferSize = sizeof(squareVertices);
    const UINT indexBufferSize = sizeof(squareIndices);

    auto heapUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto buffer = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
    m_device->CreateCommittedResource(
        &heapUpload,
        D3D12_HEAP_FLAG_NONE,
        &buffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer));

    D3D12_RESOURCE_DESC  indexBuffer;
    indexBuffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    indexBuffer.Alignment = 0;
    indexBuffer.Width = indexBufferSize;
    indexBuffer.Height = 1;
    indexBuffer.DepthOrArraySize = 1;
    indexBuffer.MipLevels = 1;
    indexBuffer.Format = DXGI_FORMAT_UNKNOWN;
    indexBuffer.SampleDesc.Count = 1;
    indexBuffer.SampleDesc.Quality = 0;
    indexBuffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    indexBuffer.Flags = D3D12_RESOURCE_FLAG_NONE;

    m_device->CreateCommittedResource(
        &heapUpload,
        D3D12_HEAP_FLAG_NONE,
        &indexBuffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_indexBuffer));

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin;
    UINT8* pIndexDataBegin;
   // CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    D3D12_RANGE readRange;
    readRange.Begin = 0;
    readRange.End = 0;
    m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
    memcpy(pVertexDataBegin, squareVertices, sizeof(squareVertices));
    m_vertexBuffer->Unmap(0, nullptr);

    m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin));
    memcpy(pIndexDataBegin, squareIndices, sizeof(squareIndices));
    m_indexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;

    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_indexBufferView.SizeInBytes = indexBufferSize;


    ComPtr<ID3D12Resource> textureUploadHeap;
    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = TextureWidth;
    textureDesc.Height = TextureHeight;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    m_device->CreateCommittedResource(
        &heap,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_texture));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);
    auto upload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto buffers = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    // Create the GPU upload buffer.
    m_device->CreateCommittedResource(
        &upload,
        D3D12_HEAP_FLAG_NONE,
        &buffers,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap));

    // Copy data to the intermediate upload heap and then schedule a copy 
    // from the upload heap to the Texture2D.
    std::vector<UINT8> texture = GenerateTextureData();

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = &texture[0];
    textureData.RowPitch = TextureWidth * TexturePixelSize;
    textureData.SlicePitch = textureData.RowPitch * TextureHeight;

    UpdateSubresources(m_commandList.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);

    auto trans = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    m_commandList->ResourceBarrier(1, &trans);

    // Describe and create a SRV for the texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());



    m_commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


}

void Square::OnRender()
{
    TestCommandList();
}

void Square::WaitForPreviousFrame()
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

void Square::TestCommandList()
{
    m_commandAllocator->Reset();
    m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());

    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.Width = static_cast<float>(1280);
    m_viewport.Height = static_cast<float>(720);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_scissorRect = { 0, 0, 1280, 720 };
    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    auto useBackBuffer = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
   
    m_commandList->ResourceBarrier(1, &useBackBuffer);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr); //Screen background color will be black if remove this
    // Record commands.
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->IASetIndexBuffer(&m_indexBufferView);
    m_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

    // Indicate that the back buffer will now be used to present.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &barrier);
    m_commandList->Close();

    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    m_swapChain->Present(1, 0);

    WaitForPreviousFrame();
}
// Generate a simple black and white checkerboard texture.
std::vector<UINT8> Square::GenerateTextureData()
{
    const UINT rowPitch = TextureWidth * TexturePixelSize;
    const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
    const UINT cellHeight = TextureWidth >> 3;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * TextureHeight;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT n = 0; n < textureSize; n += TexturePixelSize)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n] = 0x00;        // R
            pData[n + 1] = 0x00;    // G
            pData[n + 2] = 0x00;    // B
            pData[n + 3] = 0xff;    // A
        }
        else
        {
            pData[n] = 0xff;        // R
            pData[n + 1] = 0xff;    // G
            pData[n + 2] = 0xff;    // B
            pData[n + 3] = 0xff;    // A
        }
    }

    return data;
}
