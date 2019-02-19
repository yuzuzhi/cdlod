
static float2	g_EVSMExponents		= float2(40.0f, 20.0f);
static float	g_EVSM_Derivation	= 0.00001f;
static float	g_FarPlane			= 300;


#define BLOCKER_SEARCH_NUM_SAMPLES 32 

#define NEAR_PLANE 0.01 
#define LIGHT_WORLD_SIZE .5 
#define LIGHT_FRUSTUM_WIDTH 3.75 
// Assuming that LIGHT_FRUSTUM_WIDTH == LIGHT_FRUSTUM_HEIGHT 
#define LIGHT_SIZE_UV (LIGHT_WORLD_SIZE / LIGHT_FRUSTUM_WIDTH)


float PenumbraSize(float zReceiver, float zBlocker) //Parallel plane estimation
{ 
    return (zReceiver - zBlocker) / zBlocker; 
} 
void FindBlocker(	out float avgBlockerDepth,  
					out float numBlockers, 
					float2 uv, float zReceiver,
					Texture2DMS<float, SHADOW_MAP_MS>	g_txDepthMap, 
					SamplerState	samPoint) 
{ 
	float2 poissonDisk[BLOCKER_SEARCH_NUM_SAMPLES] = { 
		/*float2( 0.1283428f, -0.4163534f),
		float2( 0.2019598f, -0.6032039f),
		float2( -0.08217768f, -0.4495787f),
		float2( -0.1279103f, -0.1763608f),
		float2( 0.4426644f, -0.541037f),
		float2( 0.2397046f, -0.2000998f),
		float2( 0.4343313f, -0.3287322f),
		float2( 0.02333182f, -0.6686942f),
		float2( 0.07630143f, -0.07298536f),
		float2( 0.6329507f, -0.6551171f),
		float2( 0.7171306f, -0.4372012f),
		float2( 0.4124776f, -0.8303053f),
		float2( 0.3945699f, 0.07816131f),
		float2( 0.1318415f, 0.1316341f),
		float2( 0.5673237f, -0.1772097f),
		float2( 0.6433504f, 0.0195518f),
		float2( 0.772647f, -0.2265944f),
		float2( 0.8655438f, -0.05106729f),
		float2( 0.2013993f, -0.9364453f),
		float2( 0.9531593f, -0.2420532f),
		float2( -0.2495192f, -0.6976171f),
		float2( -0.2815191f, -0.3736431f),
		float2( 0.009289342f, -0.8765978f),
		float2( 0.762243f, 0.2734051f),
		float2( 0.9826543f, 0.08805846f),
		float2( -0.5893894f, -0.520806f),
		float2( -0.4276442f, -0.1834782f),
		float2( -0.4227597f, -0.6021217f),
		float2( -0.2037458f, -0.975957f),
		float2( -0.3921187f, -0.8668526f),
		float2( -0.5662786f, -0.7740741f),
		float2( -0.3303084f, -0.0208855f),*/
		float2( -0.1037585f, 0.0115833f),
		float2( 0.7777091f, 0.5593235f),
		float2( 0.4697187f, 0.346343f),
		float2( 0.6227756f, 0.4536554f),
		float2( 0.5818148f, 0.1979623f),
		float2( 0.9146139f, 0.3989194f),
		float2( -0.1306217f, 0.2166821f),
		float2( 0.4712948f, 0.6810601f),
		float2( 0.6581924f, 0.7140562f),
		float2( -0.8172901f, -0.4193728f),
		float2( -0.6340067f, -0.229261f),
		float2( -0.7657465f, -0.6227145f),
		float2( 0.1721802f, 0.7806895f),
		float2( 0.2592959f, 0.5589375f),
		float2( 0.2902026f, 0.935185f),
		float2( 0.1638738f, 0.3138856f),
		float2( -0.05907057f, 0.5168375f),
		float2( -0.5930279f, 0.1201888f),
		float2( -0.749431f, -0.06279725f),
		float2( -0.1989045f, 0.6920061f),
		float2( -0.3935556f, 0.4419762f),
		float2( -0.1018889f, 0.8485776f),
		float2( -0.2152431f, 0.4090094f),
		float2( -0.7953848f, 0.1821126f),
		float2( -0.9892063f, 0.003228206f),
		float2( -0.9117538f, -0.2051272f),
		float2( -0.3501289f, 0.1869117f),
		float2( 0.02450336f, 0.9877323f),
		float2( -0.6391328f, 0.3946131f),
		float2( -0.550604f, 0.654435f),
		float2( -0.3973971f, 0.7763853f),
		float2( -0.9380306f, 0.3272279f)
		/*
		float2( -0.94201624, -0.39906216 ), 
		float2( 0.94558609, -0.76890725 ), 
		float2( -0.094184101, -0.92938870 ), 
		float2( 0.34495938, 0.29387760 ), 
		float2( -0.91588581, 0.45771432 ), 
		float2( -0.81544232, -0.87912464 ), 
		float2( -0.38277543, 0.27676845 ), 
		float2( 0.97484398, 0.75648379 ), 
		float2( 0.44323325, -0.97511554 ), 
		float2( 0.53742981, -0.47373420 ), 
		float2( -0.26496911, -0.41893023 ), 
		float2( 0.79197514, 0.19090188 ), 
		float2( -0.24188840, 0.99706507 ), 
		float2( -0.81409955, 0.91437590 ), 
		float2( 0.19984126, 0.78641367 ), 
		float2( 0.14383161, -0.14100790 ) 
		*/
	}; 
    //This uses similar triangles to compute what  
    //area of the shadow map we should search 
	float searchWidth = 0.04;//LIGHT_SIZE_UV * (zReceiver - NEAR_PLANE) / zReceiver; 
	float blockerSum = 0; 
    numBlockers = 0; 
     
    for( int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i ) 
    { 
	   float shadowMapDepth = g_txDepthMap.Load(
						(uv + poissonDisk[i] * searchWidth) * SHADOW_MAP_SIZE, 
						0); 

        if ( shadowMapDepth + 0.01 < zReceiver ) { 
                blockerSum += shadowMapDepth; 
                numBlockers++; 
            } 
     } 
    avgBlockerDepth = blockerSum / numBlockers; 

}

// Convert depth to EVSM coefficients
// Input depth should be in [0, 1]
float2 WarpDepth(float depth)
{
    // Rescale depth into [-1, 1]
    depth = 2.0f * depth - 1.0f;
    float pos =  exp( g_EVSMExponents.x * depth);
    float neg = -exp(-g_EVSMExponents.y * depth);
    return float2(pos, neg);
}


// Purpose: Compute the One-sided chebyshev equality
// Chebyshev inequalities give upper bounds on the probability of a set based on known moments
// Formula: Pr(X - mu >= k sigma) <= 1 / ( 1 + k*k)
// where mu = mean, sigma = standard deviation, and k is a value > 0
// X is the distribution
//
float ChebyshevUpperBound(float2 moments, float mean, float minVariance)
{
    // Compute variance
    float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, minVariance);
    
    // Compute probabilistic upper bound
    float d = mean - moments.x;
    float pMax = variance / (variance + (d * d));
    
    // One-tailed Chebyshev
    return (mean <= moments.x ? 1.0f : pMax);
}

// Purpose: Perform the exponential variance shadow map
float IsNotInShadow(float4 posLS, Texture2DMS<float, SHADOW_MAP_MS>	g_txDepthMap, Texture2D	g_txShadowMap, SamplerState	samLinear, SamplerState	samPoint)
{
	float2 texcoord = posLS.xy / posLS.w * 0.5 + 0.5;
	texcoord.y = 1 - texcoord.y;
	float depth = posLS.z / g_FarPlane;
	//depth -= 0.001;

	float2 TexDD;
	TexDD.x = ddx(posLS.x);
	TexDD.y = ddx(posLS.y);

	//return g_shadowMapTexture.Sample(g_samPoint, texcoord, int2(0,0)).r > z;
	
    float2 exponents = g_EVSMExponents;
    float2 warpedDepth = WarpDepth(depth);

	float avgBlockerDepth = 0; 
	float numBlockers = 0; 
	FindBlocker( avgBlockerDepth, numBlockers, texcoord, depth,  g_txDepthMap, samPoint); 

	//return numBlockers / BLOCKER_SEARCH_NUM_SAMPLES;
	if( numBlockers < 1 )   
		//There are no occluders so early out
		avgBlockerDepth = depth;
	//else return 0;
	float penumbraRatio = PenumbraSize(depth, avgBlockerDepth);    

	//if( numBlockers < 1 ) penumbraRatio = 0.0;
	//return penumbraRatio;

	float filterRadiusUV = penumbraRatio * LIGHT_SIZE_UV * 5/* * NEAR_PLANE*/ / depth; 

	
	// Perform the linear filtering
	float mip = smoothstep(0.05, 0.25, penumbraRatio);
	float4 occluder = g_txShadowMap.SampleLevel(samLinear, texcoord, g_vSettings.y + (1.0-g_vSettings.y) * mip/*filterRadiusUV*/);

	// Derivative of warping at depth
    float2 depthScale = g_EVSM_Derivation * exponents * warpedDepth;
    float2 minVariance = depthScale * depthScale;

	// Compute the upper bounds of the visibility function both for x and y
    float posContrib = ChebyshevUpperBound(occluder.xz, warpedDepth.x, minVariance.x);
    float negContrib = ChebyshevUpperBound(occluder.yw, warpedDepth.y, minVariance.y);

    return min(posContrib, negContrib);
}