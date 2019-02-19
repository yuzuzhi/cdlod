#include "shader_include.h"
#include "shadowMap.h"

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
	float3 vWorldPos	: WORLDPOS;
	float4 vPosLS		: POSITIONLS;
	float4 vPosition	: SV_POSITION;
};


VS_OUTPUT ShadowMapVS( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	matrix MVP			= mul(g_mLightView, g_mLightProjection);
	Output.vPosition	= mul( Input.vPosition,  MVP);
	
	Output.vWorldPos	= mul( Input.vPosition,  (float3x3)g_mWorld);	
	Output.vPosLS		= mul( float4(Output.vWorldPos.xyz, 1), MVP );

	return Output;
}

struct PS_OUTPUT
{
	float	depth		: SV_TARGET0;
	//float4	depthEVSM	: SV_TARGET1;
};

PS_OUTPUT ShadowMapPS(VS_OUTPUT Input)
{
	float z = Input.vPosLS.z / g_FarPlane;
	//z += 0.001;

	float2 warpedDepth = WarpDepth(z);

	PS_OUTPUT o;
	o.depth		= z;
	//o.depthEVSM = float4(warpedDepth.xy, warpedDepth.xy * warpedDepth.xy);
	return  o;
}

Texture2DMS<float, SHADOW_MAP_MS>	g_txShadowMap	: register( t0 );
float4 mip0PS(float2 Tex : TEXCOORD0, float4 pos	: SV_POSITION) : SV_TARGET
{
	float4 output = float4(0,0,0,0);

	for(int i = 0; i < SHADOW_MAP_MS; i++)
	{
		float2 warpedDepth = WarpDepth(g_txShadowMap.Load( pos.xy, i).x);
		output += float4(warpedDepth.xy, warpedDepth.xy * warpedDepth.xy);
	}

	return output / SHADOW_MAP_MS;
}



//////////////////////////////////////////////////////////////
// Blur shader
//////////////////////////////////////////////////////////////
Texture2D		g_txTexture	: register( t0 );
SamplerState	g_samLinear : register( s0 );

float4 BlurMip( float2 Tex : TEXCOORD0 ) : SV_TARGET
{
	float4 color = 0;
	float steps = 0;
	for(int y = -1; y <=1; y++)
	for(int x = -1; x <=1; x++)
	{
		color += g_txTexture.Sample(g_samLinear, Tex, int2(x,y));
		steps += 1;
	}
	color /= steps;

    return color;
}

float4 Blur( float2 Tex : TEXCOORD0 ) : SV_TARGET
{

	float BlurWeights[13] = 
	{
		0.002216,
		0.008764,
		0.026995,
		0.064759,
		0.120985,
		0.176033,
		0.199471,
		0.176033,
		0.120985,
		0.064759,
		0.026995,
		0.008764,
		0.002216,
	};

	int2 bias = 0;
	float4 color = 0;
#ifdef BLUR_V
	bias = float2(0,1);
#else
	bias = float2(1,0);
#endif

	float boxConstant = 1/13.0;

    for (int i = -6; i <= 6; i++)
		color += g_txTexture.Sample(g_samLinear, Tex, bias * i) * boxConstant /*BlurWeights[6 + i]*/;

    return color;
}