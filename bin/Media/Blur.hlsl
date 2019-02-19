
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