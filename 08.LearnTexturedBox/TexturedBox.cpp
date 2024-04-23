#include "TexturedBox.h"

#include "DirectXColors.h"

//#include "d3dx12.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
    XMFLOAT2 Tex;
};

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before mhMainWnd is valid.
    return TexturedBox::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

TexturedBox* TexturedBox::mApp = nullptr;
TexturedBox* TexturedBox::GetApp()
{
    return mApp;
}

TexturedBox::TexturedBox(HINSTANCE hInstance)
    : mhAppInst(hInstance)
{
    // Only one D3DApp can be constructed.
    assert(mApp == nullptr);
    mApp = this;
}

LRESULT TexturedBox::MsgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        OnUpdate();
        OnRender();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    }
    // Handle any messages the switch statement didn't.
    return DefWindowProc(hwnd, message, wParam, lParam);
}

bool TexturedBox::InitMainWindow()
{
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = MainWndProc;
    windowClass.hInstance = mhAppInst;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"LearnBox";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, 1280, 720 };

    mhMainWnd = CreateWindow(
        windowClass.lpszClassName,
        L"07.LearnBox",
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


void TexturedBox::LoadPipeLine()
{

        CreateDXGI();
        CreateFence();
        CreateCommandObjects();
        CreateSwapChain();
        CreateDescriptorHeaps();
        CreateRTV();
}

void TexturedBox::CreateDXGI()
{
    UINT dxgiFactoryFlags = 0;
    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
    D3D12CreateDevice(
        nullptr,  //Default adapter
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device));

}

void TexturedBox::CreateFence()
{
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    m_fenceValue = 1;
}

void TexturedBox::CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList));
    m_commandList->Close();
}

void TexturedBox::CreateSwapChain()
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
        mhMainWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    );

    // This sample does not support fullscreen transitions.
   // factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    swapChain.As(&m_swapChain);
}

void TexturedBox::CreateDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap));

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

}

void TexturedBox::CreateRTV()
{
    // CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
   //https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-cpu-descriptor-handle
   //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_cpu_descriptor_handle

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());


    // Create a RTV for each frame.
    for (UINT n = 0; n < FrameCount; n++)
    {
        m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.ptr = SIZE_T(rtvHandle.ptr + 1 * m_rtvDescriptorSize);
        //  rtvHandle.Offset(1, m_rtvDescriptorSize);
    }

}

void TexturedBox::LoadAssets()
{

        BuildRootSignature();
        BuildShadersAndInputLayout();
        BuildGeometry();
        BuildCamera();
        BuildConstangBuffer();
        BuildPSO();

    

}
void TexturedBox::BuildRootSignature()
{
   


    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

   // CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
   // ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    /*D3D12_DESCRIPTOR_RANGE1 ranges[1];
    ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    ranges[0].NumDescriptors = 1;
    ranges[0].BaseShaderRegister = 0;
    ranges[0].RegisterSpace = 0;
    ranges[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
    ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;*/

    D3D12_DESCRIPTOR_RANGE1 cbvTable;
    cbvTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cbvTable.NumDescriptors = 1;
    cbvTable.BaseShaderRegister = 0;
    cbvTable.RegisterSpace = 0;
    cbvTable.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
    cbvTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE1 srvTable;
    cbvTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    cbvTable.NumDescriptors = 1;
    cbvTable.BaseShaderRegister = 0;
    cbvTable.RegisterSpace = 0;
    cbvTable.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
    cbvTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

   // CD3DX12_ROOT_PARAMETER1 rootParameters[1];
   // rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

    /*D3D12_ROOT_PARAMETER1 slotParameters[1];
    slotParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    slotParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    slotParameters[0].DescriptorTable.pDescriptorRanges = &cbvTable;
    slotParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;*/

   D3D12_ROOT_PARAMETER1 slotParameters[2];
   //CBV 
   slotParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
   slotParameters[0].DescriptorTable.NumDescriptorRanges = 1;
   slotParameters[0].DescriptorTable.pDescriptorRanges = &cbvTable;
   slotParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
   //SRV
   slotParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
   slotParameters[1].DescriptorTable.NumDescriptorRanges = 1;
   slotParameters[1].DescriptorTable.pDescriptorRanges = &srvTable;
   slotParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


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
  //  CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
   // rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSignatureDesc.Desc_1_1.NumParameters = _countof(slotParameters);
    rootSignatureDesc.Desc_1_1.pParameters = slotParameters;
    rootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
   // rootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
    rootSignatureDesc.Desc_1_1.pStaticSamplers = &sampler;
    rootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    /*D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pParameters = rootParameters[0];
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;*/




    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
   // D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
        signature.GetAddressOf(), error.GetAddressOf());
    
    m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

}

void TexturedBox::BuildShadersAndInputLayout()
{
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
    D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

    // Define the vertex input layout.
   /* inputElementDescs =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };*/
     inputElementDescs =
   {
       { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
       { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
       { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }

   };

}

void TexturedBox::BuildGeometry()
{
 /*   Vertex TexturedBoxVertices[] =
    {
        Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
        Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
        Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
        Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
        Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
        Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
        Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
        Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
    };*/

   Vertex TexturedBoxVertices[] =
   {
       Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White)  ,XMFLOAT2()}),
       Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black)  ,XMFLOAT2()}),
       Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red)    ,XMFLOAT2()}),
       Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green)  ,XMFLOAT2()}),
       Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue)   ,XMFLOAT2()}),
       Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) ,XMFLOAT2()}),
       Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan)   ,XMFLOAT2()}),
       Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta),XMFLOAT2()})
   };                                                                   

 

    uint16_t TexturedBoxIndices[] =
    {
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7
    };

   // const UINT vertexBufferSize = sizeof(triangleVertices);
    const UINT vertexBufferSize = sizeof(TexturedBoxVertices);
    const UINT indexBufferSize = sizeof(TexturedBoxIndices);


    // Note: using upload heaps to transfer static data like vert buffers is not 
    // recommended. Every time the GPU needs it, the upload heap will be marshalled 
    // over. Please read up on Default Heap usage. An upload heap is used here for 
    // code simplicity and because there are very few verts to actually transfer.

   // auto heapUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_HEAP_PROPERTIES heapUpload;
    heapUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapUpload.CreationNodeMask = 1;
    heapUpload.VisibleNodeMask = 1;

  //  auto buffer = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
    D3D12_RESOURCE_DESC  buffer;
    buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    buffer.Alignment = 0;
    buffer.Width = vertexBufferSize;
    buffer.Height = 1;
    buffer.DepthOrArraySize = 1;
    buffer.MipLevels = 1;
    buffer.Format = DXGI_FORMAT_UNKNOWN;
    buffer.SampleDesc.Count = 1;
    buffer.SampleDesc.Quality = 0;
    buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    buffer.Flags = D3D12_RESOURCE_FLAG_NONE;
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
    memcpy(pVertexDataBegin, TexturedBoxVertices, sizeof(TexturedBoxVertices));
    m_vertexBuffer->Unmap(0, nullptr);

    m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin));
    memcpy(pIndexDataBegin, TexturedBoxIndices, sizeof(TexturedBoxIndices));
    m_indexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;

    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_indexBufferView.SizeInBytes = indexBufferSize;

}

void TexturedBox::BuildCamera()
{
    XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
    XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
    XMVECTOR cUp = XMLoadFloat4(&cameraUp);

    XMMATRIX tmpProjMat = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)1280 / (float)720, 0.1f, 1000.0f);
    XMStoreFloat4x4(&cameraProjMat, tmpProjMat);

    XMMATRIX tmpViewMat = XMMatrixLookAtLH(cPos, cTarg, cUp);
    XMStoreFloat4x4(&cameraViewMat, tmpViewMat);

    XMMATRIX worldMat = XMLoadFloat4x4(&mWorld);
     XMMATRIX worldViewProj = worldMat * tmpViewMat * tmpProjMat;
   // XMMATRIX worldViewProj = tmpProjMat * tmpViewMat * worldMat;

    XMStoreFloat4x4(&m_constantBufferData.WorldViewProj, XMMatrixTranspose(worldViewProj));
    
}

void TexturedBox::BuildConstangBuffer()
{
    const UINT constantBufferSize = sizeof(m_constantBufferData);    // CB size is required to be 256-byte aligned.

    D3D12_HEAP_PROPERTIES heapUpload;
    heapUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapUpload.CreationNodeMask = 1;
    heapUpload.VisibleNodeMask = 1;

    //  auto buffer = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
    D3D12_RESOURCE_DESC  buffer;
    buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    buffer.Alignment = 0;
    buffer.Width = constantBufferSize;
    buffer.Height = 1;
    buffer.DepthOrArraySize = 1;
    buffer.MipLevels = 1;
    buffer.Format = DXGI_FORMAT_UNKNOWN;
    buffer.SampleDesc.Count = 1;
    buffer.SampleDesc.Quality = 0;
    buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    buffer.Flags = D3D12_RESOURCE_FLAG_NONE;

    m_device->CreateCommittedResource(
        &heapUpload,
        D3D12_HEAP_FLAG_NONE,
        &buffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer));

    // Describe and create a constant buffer view.
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = constantBufferSize;
    m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

    // Map and initialize the constant buffer. We don't unmap this until the
    // app closes. Keeping things mapped for the lifetime of the resource is okay.
   // We do not intend to read from this resource on the CPU.

    D3D12_RANGE readRange;
    readRange.Begin = 0;
    readRange.End = 0;
    m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin));
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

 
}


void TexturedBox::BuildPSO()
{
    D3D12_SHADER_BYTECODE vertex_BYTECODE;
    vertex_BYTECODE.pShaderBytecode = vertexShader.Get()->GetBufferPointer();
    vertex_BYTECODE.BytecodeLength = vertexShader.Get()->GetBufferSize();

    D3D12_SHADER_BYTECODE pixel_BYTECODE;
    pixel_BYTECODE.pShaderBytecode = pixelShader.Get()->GetBufferPointer();
    pixel_BYTECODE.BytecodeLength = pixelShader.Get()->GetBufferSize();


    D3D12_RASTERIZER_DESC rasterizerDesc;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    D3D12_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;

    D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlenDesc =
    {
        FALSE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        blendDesc.RenderTarget[i] = defaultRenderTargetBlenDesc;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs.data(), (UINT)inputElementDescs.size() };
    psoDesc.pRootSignature = m_rootSignature.Get();
   // psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.VS = vertex_BYTECODE;
  //  psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.PS = pixel_BYTECODE;
   // psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState = rasterizerDesc;
  //  psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
}

void TexturedBox::OnUpdate()
{
    UpdateCamera();
}

void TexturedBox::UpdateCamera()
{
    /*auto cameraPosition = XMFLOAT4(0.0f, 5.0f, -4.0f, 0.0f);
    auto cameraTarget = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    auto cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);*/
    //A simple method
    //https://gamedev.stackexchange.com/questions/152654/moving-camera-in-3d-directx12
    XMMATRIX cameraViewMatrix_tmp = XMLoadFloat4x4(&cameraViewMat);

    if (GetAsyncKeyState('W')) {
        cameraViewMatrix_tmp *= XMMatrixTranslation(0.0f, 0.0f, -0.01f);
    }
    else if (GetAsyncKeyState('S')) {
        cameraViewMatrix_tmp *= XMMatrixTranslation(0.0f, 0.0f, 0.01f);
    }

    //Move Right and Left
    if (GetAsyncKeyState('A')) {
        cameraViewMatrix_tmp *= XMMatrixTranslation(0.01f, 0.0f, 0.0f);
    }
    else if (GetAsyncKeyState('D')) {
        cameraViewMatrix_tmp *= XMMatrixTranslation(-0.01f, 0.0f, 0.0f);
    }

    XMStoreFloat4x4(&cameraViewMat, cameraViewMatrix_tmp);

    XMMATRIX tmpProjMat = XMLoadFloat4x4(&cameraProjMat);
    XMMATRIX worldMat = XMLoadFloat4x4(&mWorld);
    XMMATRIX worldViewProj = worldMat * cameraViewMatrix_tmp * tmpProjMat;

    XMStoreFloat4x4(&m_constantBufferData.WorldViewProj, XMMatrixTranspose(worldViewProj));
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
}

void TexturedBox::OnRender()
{
    ResetCommandList();
    SetCommandList();
    DrawCommandList();
    ExecuteCommandList();
    WaitForPreviousFrame();
}
void TexturedBox::ResetCommandList()
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
void TexturedBox::SetCommandList()
{
    // Set necessary state.
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.Width = static_cast<float>(1280);
    m_viewport.Height = static_cast<float>(720);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    m_scissorRect = { 0, 0, 1280, 720 };

    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_cbvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());




    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
   // auto trans = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    //m_commandList->ResourceBarrier(1, &trans);
    D3D12_RESOURCE_BARRIER useBackBuffer;
    useBackBuffer.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    useBackBuffer.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    useBackBuffer.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    useBackBuffer.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    useBackBuffer.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    useBackBuffer.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &useBackBuffer);

    //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.ptr = rtvHandle.ptr + m_frameIndex * m_rtvDescriptorSize;
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr); //Screen backTexturedBox color will be black if remove this
}
void TexturedBox::DrawCommandList()
{
    // Record commands.
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->IASetIndexBuffer(&m_indexBufferView);
   // m_commandList->DrawIndexedInstanced(6, 1, 0, 0,0);

    m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

    // Indicate that the back buffer will now be used to present.
   // auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &barrier);

    m_commandList->Close();
}
void TexturedBox::ExecuteCommandList()
{
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    m_swapChain->Present(1, 0);
}
void TexturedBox::WaitForPreviousFrame()
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

std::vector<UINT8> TexturedBox::GenerateTextureData() 
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


