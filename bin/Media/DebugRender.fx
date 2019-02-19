//--------------------------------------------------------------------------------------
// File: Tutorial05.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
    float4 Color;
	
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    output.Color = Color;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
    return input.Color;
}

/////////////////////////////////////////////////////////////////////////////////////
Texture2D<float4>	g_texture	: register(t0);
SamplerState		g_samLinear	: register( s0 );
cbuffer cbLayerPos  : register( b1 )
{
	float4 g_layerPos;
}

struct LAYER_VS_INPUT
{
    float4 Pos		: POSITION;
};

struct LAYER_PS_INPUT
{
    float4 Pos		: SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};
LAYER_PS_INPUT LayerVS( LAYER_VS_INPUT input )
{
    LAYER_PS_INPUT output = (LAYER_PS_INPUT)0;
	float2 lt = g_layerPos.xy*2.f - float2(1,1);
	float2 rb = g_layerPos.zw*2.f - float2(1,1);
	if(input.Pos.x<0.f)
		output.Pos.x =  lt.x;
	else
		output.Pos.x =  rb.x;
	if(input.Pos.y<0.f)
		output.Pos.y = -rb.y;
	else
		output.Pos.y = -lt.y;
	output.Pos.zw = input.Pos.zw;
	output.TexCoord = float2(input.Pos.x,-input.Pos.y);
	output.TexCoord = (output.TexCoord + float2(1.f,1.f))*0.5;
	//output.Pos = input.Pos;
    
    return output;
}
float4 LayerPS( LAYER_PS_INPUT input) : SV_Target
{
    return float4(g_texture.Sample(g_samLinear, input.TexCoord).xyz,1);
}