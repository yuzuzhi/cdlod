#include "shader_include.h"

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPosition	: POSITION;
};

struct VS_OUTPUT
{
	float4 vPosition	: WORLDPOS;
};


struct GS_OUTPUT
{
    float4 vColor			 : COLOR0;
    float4 vPosition         : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition	= Input.vPosition;	
	return Output;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------

[maxvertexcount(4)]
void GSMain(point VS_OUTPUT Input[1], inout TriangleStream<GS_OUTPUT> TriangleOutputStream)
{
	GS_OUTPUT output;
	
	float  w			= 0.1;
	float4 center		= float4(0,0,0,1);
	float4 view_axis	= float4(0,0,0,1);
	view_axis.xyz		= mul(Input[0].vPosition.xyz, (float3x3)g_mView );
	//view_axis.xyz		= Input[0].vPosition.xyz;
	
	output.vColor		= Input[0].vPosition;
	output.vColor		*=  view_axis.z > 0;
	
	view_axis.z			= view_axis.z*0.5 + 1;

	float4 offset = float4(0,0,0,0);
	offset.xyz = cross(view_axis.xyz, float3(0,0,1)) * w;
	output.vPosition = mul( center - offset, g_mProjection );	
	output.vPosition = center - offset;
	TriangleOutputStream.Append(output);
	output.vPosition = mul( center + offset, g_mProjection );
	output.vPosition = center + offset;
	TriangleOutputStream.Append(output);

	output.vColor		= Input[0].vPosition;
	output.vPosition = mul( view_axis - offset, g_mProjection );
	output.vPosition = view_axis - offset;
	TriangleOutputStream.Append(output);
	output.vPosition = mul( view_axis + offset, g_mProjection );
	output.vPosition = view_axis + offset;
	TriangleOutputStream.Append(output);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( GS_OUTPUT Input ) : SV_TARGET
{
	return Input.vColor;
}