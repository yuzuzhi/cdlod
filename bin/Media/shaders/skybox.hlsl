#include "shader_include.h"

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float3 vTangent		: TEXCOORD1;
	float3 vBinormal	: TEXCOORD2;
	float2 vTexcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float3 vNormal		: NORMAL;
	float3 vTexcoord	: TEXCOORD0;
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vTexcoord	= Input.vPosition.xyz;
	Input.vPosition.xyz *= 10;
	Output.vPosition	= mul( Input.vPosition, g_mWorldViewProjection );
	Output.vNormal		= mul( Input.vNormal, (float3x3)g_mWorld );

	return Output;
}

TextureCube		g_colorTexture		: register( t0 );
SamplerState	g_samLinear			: register( s0 );
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT Input ): SV_TARGET
{
	float4 color = g_colorTexture.SampleLevel(g_samLinear, Input.vTexcoord, 0);
	return float4(0.4 * color.rgb, 1);
}