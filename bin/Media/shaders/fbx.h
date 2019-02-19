
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
	float3 vTangent		: TANGENT;
	float3 vBinormal	: BINORMAL;
	float2 vTexcoord	: TEXCOORD0;
	float3 vLightDir	: TEXCOORD1;
	float3 vCamDir		: TEXCOORD2;
	float3 vWorldPos	: WORLDPOS;
	float3 vPosCS		: POSCS;
	float3 vNormalCS	: NORMALCS;
	float4 vPosLS		: POSITIONLS;
	float4 vPosition	: SV_POSITION;

};

VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition	= mul( Input.vPosition, g_mWorldViewProjection );
	Output.vNormal		= mul( Input.vNormal, (float3x3)g_mWorld );
	Output.vTangent		= mul( Input.vTangent, (float3x3)g_mWorld );
	Output.vBinormal	= mul( Input.vBinormal, (float3x3)g_mWorld );
	Output.vTexcoord	= Input.vTexcoord;

	Output.vPosCS		= mul( Input.vPosition, g_mWorldView );
	Output.vNormalCS	= mul( Input.vNormal, (float3x3)g_mWorldView );

	//float3x3 TangSpace	= float3x3(Input.vTangent, Input.vBinormal, Input.vNormal);
	
	//Output.vLightDir	= mul( TangSpace, g_vLightDir );
	//Output.vCamDir		= mul( TangSpace, g_vEye - Input.vPosition );

	Output.vLightDir	= g_vLightDir;
	Output.vCamDir		= g_vEye - Input.vPosition;	
	Output.vWorldPos	= Input.vPosition.xyz;

	matrix LightVP		= mul(g_mLightView, g_mLightProjection);
	Output.vPosLS		= mul( float4(Output.vWorldPos.xyz, 1), LightVP );
	return Output;

}