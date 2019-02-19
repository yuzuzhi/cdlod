
Texture2D<float>	g_terrainHMVertexTexture	: register(t0);
Texture2D			g_terrainNMTexture			: register(t1);
Texture2D			g_terrainShadowMap			: register(t2);
Texture2D			g_terrainShadowMap1			: register(t3);
Texture2D	g_GroundTexture1			: register(t4);
Texture2D	g_GroundTexture2			: register(t5);
Texture2D	g_GroundTexture3			: register(t6);
Texture2D	g_GroundTexture4			: register(t7);
Texture2D	g_GroundNormalTexture1		: register(t8);
Texture2D	g_GroundNormalTexture2		: register(t9);

SamplerState g_samLinear			: register( s0 );
SamplerState g_groundTextureSS		: register( s1 );


struct VS_OUTPUT
{
	float4 Position			: SV_POSITION;
	float4 globalUV_detUV	: TEXCOORD0;
	float4 lightDir			: TEXCOORD1;	// .xyz
	float4 eyeDir			: TEXCOORD2;	// .xyz = eyeDir, .w = eyeDist
	float4 PosInShadowView	: TEXCOORD3;
	float4 PosInShadowView1	: TEXCOORD4;
};

struct SHADOW_VS_OUTPUT
{
	float4 Position			: SV_POSITION;
	float2 Depth			: TEXCOORD0;
};