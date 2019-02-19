
//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPosition	: POSITION;
	float2 vTexCoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float2 vTexCoord	: TEXCOORD0;
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition	= Input.vPosition;
	Output.vTexCoord	= Input.vTexCoord;
	
	return Output;
}