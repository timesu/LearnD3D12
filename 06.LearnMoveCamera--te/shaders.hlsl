// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float3 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    //result.position = mul(position, gWorldViewProj);
    result.position = mul(float4(position, 1.0f), gWorldViewProj);
    result.color = color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}

//cbuffer cbPerObject : register(b0)
//{
//	float4x4 gWorldViewProj;
//};
//
//struct VertexIn
//{
//	float3 PosL : POSITION;
//	float4 Color : COLOR;
//};
//
//struct VertexOut
//{
//	float4 PosH : SV_POSITION;
//	float4 Color : COLOR;
//};
//
//VertexOut VS(VertexIn vin)
//{
//	VertexOut vout;
//
//	// Transform to homogeneous clip space.
//	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
//
//	// Just pass vertex color into the pixel shader.
//	vout.Color = vin.Color;
//
//	return vout;
//}
//
//float4 PS(VertexOut pin) : SV_Target
//{
//	return pin.Color;
//}