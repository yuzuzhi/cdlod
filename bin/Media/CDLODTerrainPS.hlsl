#include "CDLODTerrainCommon.sh"

cbuffer LightInfo : register( b0 )
{
	float4	g_lightColorDiffuse;// = float4(0.65f, 0.65f, 0.65f, 1.0f);    // actually diffuse and specular, but nevermind...
	float4	g_lightColorAmbient;// = float4(0.35f, 0.35f, 0.35f, 1.0f);
	float4	g_fogColor;// = float4(0.0f, 0.5f, 0.5f, 1.0f);
}


float3 UncompressDXT5_NM(float4 normPacked)
{
   float3 norm = float3( normPacked.w * 2.0 - 1.0, normPacked.x * 2.0 - 1.0, 0 );
   norm.z = sqrt( 1 - norm.x * norm.x - norm.y * norm.y );
   return norm;
}

float CalculateDiffuseStrength( float3 normal, float3 lightDir )
{
   return saturate( -dot( normal, lightDir ) );
}

float CalculateSpecularStrength( float3 normal, float3 lightDir, float3 eyeDir )
{
   float3 diff    = saturate( dot(normal, -lightDir) );
   float3 reflect = normalize( 2 * diff * normal + lightDir ); 
   
   return saturate( dot( reflect, eyeDir ) );
}

float CalculateDirectionalLight( float3 normal, float3 lightDir, float3 eyeDir, float specularPow, float specularMul )
{
   float3 light0 = normalize( lightDir );

   return CalculateDiffuseStrength( normal, light0 ) + specularMul * pow( CalculateSpecularStrength( normal, light0, eyeDir ), specularPow );
}


void CalculateVarianceShadow (in Texture2D varianceMap,  in float4 vShadowTexCoord,out float fPercentLit ) 
{
    fPercentLit = 0.0f;
	        
    float2 mapDepth = 0;    
    mapDepth += varianceMap.Sample( g_samLinear, vShadowTexCoord.xy);
	//float3 vShadowTexCoordDDX = ddx(vShadowTexCoord);
	//vShadowTexCoordDDX *= m_vCascadeScale[iCascade].xyz;
	//float3 vShadowTexCoordDDY = ddy(vShadowTexCoord);
	//vShadowTexCoordDDY *= m_vCascadeScale[iCascade].xyz;

	//mapDepth += varianceMap.SampleGrad(g_samLinear, vShadowTexCoord.xyz,
	//	vShadowTexCoordDDX,
	//	vShadowTexCoordDDY);
		        
    float  fAvgZ  = mapDepth.x;
    float  fAvgZ2 = mapDepth.y;
    
	if (vShadowTexCoord.z <= fAvgZ)
    {
        fPercentLit = 1;
	}
	else 
	{
	    float variance = ( fAvgZ2 ) - ( fAvgZ * fAvgZ );
        variance       = min( 1.0f, max( 0.0f, variance + 0.00001f ) );
    
        float mean     = fAvgZ;
		float d			= vShadowTexCoord.z - mean; // We put the z value in w so that we can index the texture array with Z.
        float p_max    = variance / ( variance + d*d );

        // To combat light-bleeding, experiment with raising p_max to some power
        // (Try values from 0.1 to 100.0, if you like.)
        fPercentLit = pow( p_max, 4 );
		//fPercentLit = 1;
	    
	}
    
}



float4 TerrainPS( VS_OUTPUT Input ) : SV_TARGET
{
	//
	float3 normal = g_terrainNMTexture.Sample(g_samLinear, Input.globalUV_detUV.xy).xzy;
	//normal = normalize(normal);


	// Calculate the first bump map using the first normal map.
	//float4 bumpMap = g_GroundNormalTexture1.Sample(g_groundTextureSS, Input.globalUV_detUV.xy);
	//bumpMap = (bumpMap * 2.0f) - 1.0f;
	//bumpNormal = input.normal + bumpMap.x * input.tangent + bumpMap.y * input.binormal;
	//bumpNormal = normalize(bumpNormal);
	//
	//return float4(Input.PosInShadowView.www,1);
	float directionalLight = CalculateDirectionalLight( normal, normalize( Input.lightDir.xyz ), normalize( Input.eyeDir.xyz ), 16.0, 0.0 );
	float3 color = g_lightColorAmbient.xyz;
#if 1
	float fPercentLit;
	float fPercentLit1;	
	CalculateVarianceShadow(g_terrainShadowMap, Input.PosInShadowView,fPercentLit);
	CalculateVarianceShadow(g_terrainShadowMap1, Input.PosInShadowView1,fPercentLit1);
	color += g_lightColorDiffuse.xyz * directionalLight * (fPercentLit*Input.PosInShadowView.w+fPercentLit1*Input.PosInShadowView1.w);
	//color += g_lightColorDiffuse.xyz * directionalLight * fPercentLit;
#else
	float depthInTexture = g_terrainShadowMap.Sample(g_samLinear, Input.PosInShadowView.xy).r * Input.PosInShadowView.w
						 + g_terrainShadowMap1.Sample(g_samLinear, Input.PosInShadowView1.xy).r * Input.PosInShadowView1.w;
	float depthInShadowView = Input.PosInShadowView.z*Input.PosInShadowView.w + Input.PosInShadowView1.z*Input.PosInShadowView1.w;
	if (depthInShadowView-0.00001f < depthInTexture)
		color += g_lightColorDiffuse.xyz * directionalLight;
	color = saturate(color);
#endif


	float2 groundTexCoord = float2(Input.globalUV_detUV.x*800.f, Input.globalUV_detUV.y*800.f);
		float4 textureColor = float4(1,1,1,1);// g_GroundTexture1.Sample(g_groundTextureSS, groundTexCoord);

	return textureColor*float4(color,1);
}


//shadow the terrain
float2 GenTerrainVSMPS(float4 Position : SV_POSITION) : SV_TARGET
{
	//return float2(1,1);
	return float2( Position.z,Position.z*Position.z);
}