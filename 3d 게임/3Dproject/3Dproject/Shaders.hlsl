struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
	matrix					gmtxView : packoffset(c0);
	matrix					gmtxProjection : packoffset(c4);
	float3					gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix					gmtxGameObject : packoffset(c0);
	MATERIAL				gMaterial : packoffset(c4);
	
};

#include "Light.hlsl"


Texture2D gDiffuseMap : register(t0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

//#define _WITH_VERTEX_LIGHTING

struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
    float2 TexC : TEXCOORD;
};

struct VS_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
    float2 TexC : TEXCOORD;
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

    output.TexC.x = input.TexC.x;
    output.TexC.y = (1 - input.TexC.y);
#ifdef _WITH_VERTEX_LIGHTING
	output.normalW = normalize(output.normalW);
	output.color = Lighting(output.positionW, output.normalW);
#endif
	return(output);
}

float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
#ifdef _WITH_VERTEX_LIGHTING
	return(input.color);
#else
    
    float4 texColor = gDiffuseMap.Sample(gsamAnisotropicWrap, input.TexC);
    //float4 texColor = float4(input.TexC.x, input.TexC.y, 0, 1.0f);
    return texColor;

	input.normalW = normalize(input.normalW);
    
	float4 color = texColor+Lighting(input.positionW, input.normalW);
	return(color);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_BOUNDINGBOX_INPUT
{
    float3 position : POSITION;
};

struct VS_BOUNDINGBOX_OUTPUT
{
    float4 positionH : SV_POSITION;
};

VS_BOUNDINGBOX_OUTPUT VSBoundingBox(VS_BOUNDINGBOX_INPUT input)
{
    VS_BOUNDINGBOX_OUTPUT output;
    output.positionH = mul(mul(mul(float4(input.position, 1.0f),gmtxGameObject), gmtxView),gmtxProjection);

    return (output);
}

float4 PSBoundingBox(VS_BOUNDINGBOX_OUTPUT input) : SV_TARGET
{
    return (float4(1.0f, 0.0f, 0.0f, 1.0f));
}

/////////////////////////////////////////////////////////////////////////////////////
struct VS_UIRECT_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 TexC : TEXCOORD;
};

struct VS_UIRECT_OUTPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 TexC : TEXCOORD;
};

VS_UIRECT_OUTPUT VSUiRect(VS_UIRECT_INPUT input)
{
    VS_UIRECT_OUTPUT output;
    //output.position = mul(float4(input.position, 1.0f), gmtxProjection);
 
    output.position = float4(input.position, 1.0f);
    output.normal = input.normal;
    output.TexC.x = input.TexC.x;
    output.TexC.y = input.TexC.y;
	
    return (output);
}

float4 PSUiRect(VS_UIRECT_OUTPUT input) : SV_TARGET
{    
    float4 texColor = gDiffuseMap.Sample(gsamPointClamp, input.TexC);
 
    return texColor;
	
   // return (float4(input.TexC.x, input.TexC.y, 0.0f, 0.5f));
}