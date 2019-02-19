#include "shader_include.h"
#include "fbx.h"
#include "shadowMap.h"


Texture2D		g_colorTexture	: register( t0 );
Texture2D		g_EVSM			: register( t1 );
SamplerState	g_samLinear		: register( s0 );
SamplerState	g_samPoint		: register( s1 );


Texture2DMS<float, SHADOW_MAP_MS>		g_DepthMap		: register( t2 );
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------


float4 PSMain( VS_OUTPUT Input ): SV_TARGET
{
	float shadow = IsNotInShadow(Input.vPosLS, g_DepthMap, g_EVSM, g_samLinear, g_samPoint);
	//return saturate(shadow.xxxx);

	float4 color = g_colorTexture.Sample(g_samLinear, Input.vTexcoord);
	
	if(g_vSettings.x == 0)
		color = float4(1,1,1,1);

	float3 N = normalize(Input.vNormal);
	float3 L = normalize(g_vLightDir);
	float  diffuse = saturate(dot(N, L)) + 0.0;
	return float4(pow(color.rgb * (0.3 + 0.7 * diffuse.xxx * shadow), 1 / 2.2), 1);
}