#include "D3D12_OBJECT.h"
#include "Win32Application.h"

D3D12_OBJECT::D3D12_OBJECT(UINT width, UINT height) :
    m_frameIndex(0),
   // m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
   // m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0)
{
}

void D3D12_OBJECT::OnInit()
{

   // BuildSwapChain();
   // BuildDescriptorHeaps();

    LoadPipeline();
    LoadAssets();
}

void D3D12_OBJECT::BuildSwapChain()
{
    UINT dxgiFactoryFlags = 0;
    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

    ComPtr<IDXGIAdapter> warpAdapter;
    factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

    D3D12CreateDevice(
        warpAdapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device)
    );

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = 1280;
    swapChainDesc.Height = 960;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        m_hwnd,
        // t_hwnd,
      //  Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    );
   
}

void D3D12_OBJECT::BuildDescriptorHeaps()
{
    
}


void D3D12_OBJECT::BuildFrameResource()
{


}

void D3D12_OBJECT::BuildConstanBuffers()
{


}
void D3D12_OBJECT::BuildRootSignature()
{

}
void D3D12_OBJECT::BuildShaderAndInputLayout()
{

}
void D3D12_OBJECT::BuildGeometry()
{

}
void D3D12_OBJECT::BuildPso()
{

}


void D3D12_OBJECT::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;
    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

    ComPtr<IDXGIAdapter> warpAdapter;
    factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

     D3D12CreateDevice(
        warpAdapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device)
    );

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = GetWidth();
    swapChainDesc.Height = GetHeight();
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
         m_hwnd,
        // t_hwnd,
      //  Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    );

    factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
    swapChain.As(&m_swapChain);
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    //Create descriptor 
    {
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

        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
        cbvHeapDesc.NumDescriptors = 1;
        cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap));
    }

    //create frame resource
    {
        //https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/cd3dx12-cpu-descriptor-handle
        //https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/ns-d3d12-d3d12_cpu_descriptor_handle
        //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            // rtvHandle.Offset(1, m_rtvDescriptorSize);
            rtvHandle.ptr = SIZE_T(rtvHandle.ptr + 1 * m_rtvDescriptorSize);
        }
    }
    m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
}

void D3D12_OBJECT::LoadAssets()
{
    {

        // Create UV MAP
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }


        //https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-descriptor-range1
        //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_descriptor_range1

       // CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
       // ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        //regist s0
        D3D12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        ranges[0].NumDescriptors = 1;
        ranges[0].BaseShaderRegister = 0;
        ranges[0].RegisterSpace = 0;
        ranges[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
        ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
        rootParameters[0].DescriptorTable.pDescriptorRanges = ranges;
        rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



        //https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/cd3dx12-root-parameter1
        //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_root_parameter1

        //CD3DX12_ROOT_PARAMETER1 rootParameters[1];
       // rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        //  void static inline InitAsDescriptorTable(D3D12_ROOT_PARAMETER1 &rootParam, UINT numDescriptorRanges, 
        //                                          const D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges,
        //                                          D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);


      //  typedef struct D3D12_ROOT_PARAMETER1 {
       //     D3D12_ROOT_PARAMETER_TYPE ParameterType;
       //     union {
        //        D3D12_ROOT_DESCRIPTOR_TABLE1 DescriptorTable;
       //         D3D12_ROOT_CONSTANTS         Constants;
        //        D3D12_ROOT_DESCRIPTOR1       Descriptor;
        //    };
         //   D3D12_SHADER_VISIBILITY   ShaderVisibility;
      //  } 
    // 

    //https://sites.google.com/site/monshonosuana/directx%E3%81%AE%E8%A9%B1/directx%E3%81%AE%E8%A9%B1-%E7%AC%AC145%E5%9B%9E


    //Try combine cbv with srv
    D3D12_DESCRIPTOR_RANGE1 descRanges[2];
    descRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descRanges[0].NumDescriptors = 1;
    descRanges[0].BaseShaderRegister = 0;
    descRanges[0].RegisterSpace = 0;
    descRanges[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
    descRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    descRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descRanges[1].NumDescriptors = 1;
    descRanges[1].BaseShaderRegister = 0;
    descRanges[1].RegisterSpace = 0;
    descRanges[1].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
    descRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
   



    D3D12_ROOT_PARAMETER1 slotParameters[2];
    //slot t0
    slotParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    slotParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    slotParameters[0].DescriptorTable.pDescriptorRanges = descRanges;

    //slot b0
    slotParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    slotParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    slotParameters[1].DescriptorTable.pDescriptorRanges = &descRanges[1];
    slotParameters[1].DescriptorTable.NumDescriptorRanges = 1;
    slotParameters[1].Descriptor.RegisterSpace = 0;
    slotParameters[1].Descriptor.ShaderRegister = 0;



  //  rootParameters[0].Constants.ShaderRegister = 0;
  //  rootParameters[0].Constants.RegisterSpace = 0;
   // rootParameters[0].Constants.Num32BitValues = 32;

   // rootParameters[0].Descriptor.RegisterSpace = 0;
  //  rootParameters[0].Descriptor.ShaderRegister = 0;
  //  rootParameters[0].Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC;


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


    //https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-versioned-root-signature-desc
    //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_versioned_root_signature_desc

    //typedef struct D3D12_VERSIONED_ROOT_SIGNATURE_DESC {
     //   D3D_ROOT_SIGNATURE_VERSION Version;
     //   union {
     //       D3D12_ROOT_SIGNATURE_DESC  Desc_1_0;
      //      D3D12_ROOT_SIGNATURE_DESC1 Desc_1_1;
      //      D3D12_ROOT_SIGNATURE_DESC2 Desc_1_2;
      //  };
   // }


   // CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
   // rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_root_signature_desc1

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Version = featureData.HighestVersion;
    rootSignatureDesc.Desc_1_1.NumParameters = _countof(rootParameters);
    rootSignatureDesc.Desc_1_1.pParameters = rootParameters;
    rootSignatureDesc.Desc_1_1.NumStaticSamplers = 1;
    rootSignatureDesc.Desc_1_1.pStaticSamplers = &sampler;
    rootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC slotSignatureDesc;
    slotSignatureDesc.Version = featureData.HighestVersion;
    slotSignatureDesc.Desc_1_1.NumParameters = _countof(slotParameters);
    slotSignatureDesc.Desc_1_1.pParameters = rootParameters;
    slotSignatureDesc.Desc_1_1.NumStaticSamplers = 1;
    slotSignatureDesc.Desc_1_1.pStaticSamplers = &sampler;
    slotSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

   
     ComPtr<ID3DBlob> signature;
     ComPtr<ID3DBlob> error;
     D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
     m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

    // D3DX12SerializeVersionedRootSignature(&slotSignatureDesc, featureData.HighestVersion, &signature, &error);
    // m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

        //https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/cd3dx12-root-signature-desc
        //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_root_signature_desc
       // CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
       // void inline Init(UINT numParameters, const D3D12_ROOT_PARAMETER* _pParameters, UINT numStaticSamplers = 0, const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL, D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);
       // rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

       //  D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;

       // rootSignatureDesc.NumParameters = 0;
       // rootSignatureDesc.pParameters = nullptr;
       //  rootSignatureDesc.NumStaticSamplers = 0;
       // rootSignatureDesc.pStaticSamplers = nullptr;
       //  rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


       //  ComPtr<ID3DBlob> signature;
       //  ComPtr<ID3DBlob> error;
       //  D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
       //  m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        D3DCompileFromFile(L"C:\\Users\\luwen.su\\Hello-D3D12\\Hello-D3D12\\Hello-D3D12-NewHire_Project\\shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
        D3DCompileFromFile(L"C:\\Users\\luwen.su\\Hello-D3D12\\Hello-D3D12\\Hello-D3D12-NewHire_Project\\shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
           // ********{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        //https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/cd3dx12-shader-bytecode
        //https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/ns-d3d12-d3d12_shader_bytecode
        //psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        D3D12_SHADER_BYTECODE vertex_BYTECODE;
        vertex_BYTECODE.pShaderBytecode = vertexShader.Get()->GetBufferPointer();
        vertex_BYTECODE.BytecodeLength = vertexShader.Get()->GetBufferSize();
        psoDesc.VS = vertex_BYTECODE;

        //psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        D3D12_SHADER_BYTECODE pixel_BYTECODE;
        pixel_BYTECODE.pShaderBytecode = pixelShader.Get()->GetBufferPointer();
        pixel_BYTECODE.BytecodeLength = pixelShader.Get()->GetBufferSize();
        psoDesc.PS = pixel_BYTECODE;

        //https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/cd3dx12-rasterizer-desc
        //https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/ns-d3d12-d3d12_rasterizer_desc
        //psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

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

        psoDesc.RasterizerState = rasterizerDesc;


        //https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/cd3dx12-blend-desc
        //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_blend_desc
        //psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

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


    // Create the command list.
    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList));

    // Create the vertex buffer.
    {
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.25f , 0.0f }, { 0.5f, 0.0f } },
            { { 0.0f, -0.25f , 0.0f }, { 1.0f, 1.0f } },
            { { -0.25f, -0.25f , 0.0f }, { 0.0f, 1.0f } }
   
        };

        Vertex squareVertices[] =
        {
         //   { { 0.0f, 0.25f , 0.0f }, { 0.5f, 0.0f } },
         //   { { 0.0f, -0.25f , 0.0f }, { 1.0f, 1.0f } },
         //   { { -0.25f, -0.25f , 0.0f }, { 0.0f, 1.0f } },
         //   { { -0.25f, 0.25f , 0.0f }, { 0.0f, 0.5f } }
            //Introdution to D3D Page 385
            //Face Front
            { { 0.25f, 0.25f , 0.0f }, { 0.0f, 1.0f } },
            { { 0.25f, -0.25f , 0.0f }, { 0.0f, 0.0f } },
            { { -0.25f, -0.25f , 0.0f }, { 1.0f, 0.0f } },
            { { -0.25f, 0.25f , 0.0f }, { 1.0f, 1.0f } }
        };

        uint16_t squareIndices[6] = { 
                                      //Face Front  
                                      //Trangle 1
                                      0,1,2,
                                      //Trangle 2
                                      0,2,3 };





        m_viewport.TopLeftX = 0.0f;
        m_viewport.TopLeftY = 0.0;
       // m_viewport.Width = GetWidth();
       // m_viewport.Height = GetHeight();
m_viewport.Width = 1280;
m_viewport.Height = 960;

// m_scissorRect(0, 0, static_cast<LONG>(screen_width), static_cast<LONG>(screen_height));
m_scissorRect.left = 0;
m_scissorRect.top = 0;
m_scissorRect.right = 1280;
m_scissorRect.bottom = 960;

// const UINT vertexBufferSize = sizeof(triangleVertices);
// const UINT vertexBufferSize = sizeof(triangleVertices);


const UINT vertexBufferSize = sizeof(squareVertices);
const UINT indiceBufferSize = sizeof(squareIndices);

// Note: using upload heaps to transfer static data like vert buffers is not 
// recommended. Every time the GPU needs it, the upload heap will be marshalled 
// over. Please read up on Default Heap usage. An upload heap is used here for 
// code simplicity and because there are very few verts to actually transfer.

//https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-heap-properties
//https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_heap_properties
//auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

D3D12_HEAP_PROPERTIES heapProperties;
heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
heapProperties.CreationNodeMask = 1;
heapProperties.VisibleNodeMask = 1;


//https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-resource-desc
//https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_resource_desc
//auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

D3D12_RESOURCE_DESC  resourceDesc;
resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
resourceDesc.Alignment = 0;
resourceDesc.Width = vertexBufferSize;
resourceDesc.Height = 1;
resourceDesc.DepthOrArraySize = 1;
resourceDesc.MipLevels = 1;
resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
resourceDesc.SampleDesc.Count = 1;
resourceDesc.SampleDesc.Quality = 0;
resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

//CreateCommittedResource
//https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createcommittedresource
m_device->CreateCommittedResource(
    &heapProperties,
    D3D12_HEAP_FLAG_NONE,
    &resourceDesc,
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&m_vertexBuffer));

m_device->CreateCommittedResource(
    &heapProperties,
    D3D12_HEAP_FLAG_NONE,
    &resourceDesc,
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&m_indexBuffer));

// Copy the triangle data to the vertex buffer.
UINT8* pVertexDataBegin;
UINT8* pIndexDataBegin;

//https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-range
// CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
D3D12_RANGE readRange;
readRange.Begin = 0;
readRange.End = 0;
m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
// memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
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
m_indexBufferView.SizeInBytes = indiceBufferSize;
m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

    }

    //create constant buffer
    {
        const UINT constantBufferSize = sizeof(SceneConstantBuffer);    // CB size is required to be 256-byte aligned.


        D3D12_HEAP_PROPERTIES heapUpload;
        heapUpload.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapUpload.CreationNodeMask = 1;
        heapUpload.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC  bufferUpload;
        bufferUpload.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferUpload.Alignment = 0;
        bufferUpload.Width = constantBufferSize;
        bufferUpload.Height = 1;
        bufferUpload.DepthOrArraySize = 1;
        bufferUpload.MipLevels = 1;
        bufferUpload.Format = DXGI_FORMAT_UNKNOWN;
        bufferUpload.SampleDesc.Count = 1;
        bufferUpload.SampleDesc.Quality = 0;
        bufferUpload.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferUpload.Flags = D3D12_RESOURCE_FLAG_NONE;


        m_device->CreateCommittedResource(
            &heapUpload,
            D3D12_HEAP_FLAG_NONE,
            &bufferUpload,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_constantBuffer));

        // Describe and create a constant buffer view.
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = constantBufferSize;
        m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

       // CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
       // D3D12_RANGE readRange;
       // readRange.Begin = 0;
       // readRange.End = 0;
      //  m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin));
       // memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

    }

    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.
    ComPtr<ID3D12Resource> textureUploadHeap;

    // Create the texture.
    {
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


        //auto heapDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        //https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-heap-properties
        //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_heap_properties
        //auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        D3D12_HEAP_PROPERTIES heapDefault;
        heapDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapDefault.CreationNodeMask = 1;
        heapDefault.VisibleNodeMask = 1;



        m_device->CreateCommittedResource(
            &heapDefault,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_texture));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

        // Create the GPU upload buffer.
       // auto heapUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_HEAP_PROPERTIES heapUpload;
        heapUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapUpload.CreationNodeMask = 1;
        heapUpload.VisibleNodeMask = 1;



       // auto bufferUpload = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

        D3D12_RESOURCE_DESC  bufferUpload;
        bufferUpload.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferUpload.Alignment = 0;
        bufferUpload.Width = uploadBufferSize;
        bufferUpload.Height = 1;
        bufferUpload.DepthOrArraySize = 1;
        bufferUpload.MipLevels = 1;
        bufferUpload.Format = DXGI_FORMAT_UNKNOWN;
        bufferUpload.SampleDesc.Count = 1;
        bufferUpload.SampleDesc.Quality = 0;
        bufferUpload.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferUpload.Flags = D3D12_RESOURCE_FLAG_NONE;

        m_device->CreateCommittedResource(
            &heapUpload,
            D3D12_HEAP_FLAG_NONE,
            &bufferUpload,
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

       // auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        // auto pBarriers_render = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_RESOURCE_BARRIER resourceBarrier;
        resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        resourceBarrier.Transition.pResource = m_texture.Get();
        resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        m_commandList->ResourceBarrier(1, &resourceBarrier);

        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
    }





    m_commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            HRESULT_FROM_WIN32(GetLastError());
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }
}

std::vector<UINT8> D3D12_OBJECT::GenerateTextureData()
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

void D3D12_OBJECT::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    m_swapChain->Present(1, 0);

    WaitForPreviousFrame();
}

void D3D12_OBJECT::OnUpdate()
{
    const float translationSpeed = 0.005f;
    const float offsetBounds = 1.25f;

    m_constantBufferData.offset.x += translationSpeed;
    if (m_constantBufferData.offset.x > offsetBounds)
    {
        m_constantBufferData.offset.x = -offsetBounds;
    }
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

}

void D3D12_OBJECT::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(m_fenceEvent);
}

void D3D12_OBJECT::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
  // command lists have finished execution on the GPU; apps should use 
  // fences to determine GPU execution progress.
    m_commandAllocator->Reset();

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());

    //srv
   // ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() , m_cbvHeap.Get() };
    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get()}; //crv
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
   // m_commandList->SetGraphicsRootDescriptorTable(1, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());


    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

  

    //m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    //ResrouceBarrier
    //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-resourcebarrier
    //https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/cd3dx12-resource-barrier
    //https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/ns-d3d12-d3d12_resource_barrier
   // auto pBarriers_render = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    D3D12_RESOURCE_BARRIER pBarriers_render;
    pBarriers_render.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    pBarriers_render.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    pBarriers_render.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    pBarriers_render.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    pBarriers_render.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    pBarriers_render.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_commandList->ResourceBarrier(1, &pBarriers_render);

    //https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/cd3dx12-cpu-descriptor-handle
   //https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/ns-d3d12-d3d12_cpu_descriptor_handle
   //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);


    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.ptr = rtvHandle.ptr + m_frameIndex * m_rtvDescriptorSize;



    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    m_commandList->IASetIndexBuffer(&m_indexBufferView);


   // m_commandList->DrawInstanced(6, 1, 0, 0);
    m_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

    // Indicate that the back buffer will now be used to present.
     //ResrouceBarrier
    //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-resourcebarrier
    //https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/cd3dx12-resource-barrier
    //https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/ns-d3d12-d3d12_resource_barrier
    //auto pBarriers_target = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    D3D12_RESOURCE_BARRIER pBarriers_target;
    pBarriers_target.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    pBarriers_target.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    pBarriers_target.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    pBarriers_target.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    pBarriers_target.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    pBarriers_target.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;



    m_commandList->ResourceBarrier(1, &pBarriers_target);

    m_commandList->Close();
}

void D3D12_OBJECT::WaitForPreviousFrame()
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
