//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

//struct PSInput
//{
//    float4 position : SV_POSITION;
//    float4 color : COLOR;
//};

//PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
//{
//    PSInput result;

 //   result.position = position;
//    result.color = color;

//    return result;
//}

//float4 PSMain(PSInput input) : SV_TARGET
//{
//    return input.color;
//}

//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
    float4 padding[15];
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

PSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD)
{
    PSInput result;

    result.position = position;
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.uv);
}