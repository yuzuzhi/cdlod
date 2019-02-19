#include "CDLODTerrainCommon.sh"


cbuffer cbFrame : register( b0 )
{
	matrix		g_viewProjection;
	float4		g_cameraPos;
	float4		g_diffuseLightDir;
};


cbuffer cbNodeInfo : register(b1)
{
	float4		g_quadOffset;
	float4		g_quadScale;
};

cbuffer cbTerrainInfo : register(b2)
{
	float4		g_terrainScale;
	float4		g_terrainOffset;
	float4		g_heightmapTextureInfo;
	float4		g_samplerWorldToTextureScale;
	float4		g_gridDim;	
}


cbuffer cbDebugInfo : register(b3)
{
	float4		g_debugVars;
}

cbuffer cbLodLevelInfo : register(b4)
{
	matrix		g_shadowViewProj;
	matrix		g_shadowViewProj1;
	float4		g_lodLevlMorph;

}
float2 GetGlobalUV(float2 vPosInWorld)
{
	float2 GlobalUV = (vPosInWorld.xy-g_terrainOffset.xz) / g_terrainScale.xz;
	GlobalUV *= g_samplerWorldToTextureScale.xy;
	GlobalUV += g_heightmapTextureInfo.zw * 0.5;
	return GlobalUV;
}

float2 MorphVertex( float3 inPos, float2 vertex, float morphLerpK )
{
   float2 fracPart = ( frac(inPos.xz*float2(g_gridDim.y,g_gridDim.y)) * float2(g_gridDim.z, g_gridDim.z) ) * g_quadScale.xz;
   return vertex.xy - fracPart * morphLerpK;
}



void SampleTerrainVertex(in float3 vPos, out float4 vPosInWorld, out float2 GlobalUV,out float morphLerpK)
{
	vPosInWorld = float4(vPos,0) * g_quadScale + g_quadOffset;

   //根据摄像机位置计算插值因子 (mEnd-eyeDist) / (mEnd-mStart)   ==>  mEnd/(mEnd-mStart) - eyeDist*(1.0f/(mEnd-mStart))
   float eyeDist = distance( float4(vPosInWorld.x,g_terrainOffset.y,vPosInWorld.z,0), g_cameraPos ); 
   if(g_debugVars.x<0.5f)
	   eyeDist = distance( vPosInWorld, g_cameraPos ); 
   morphLerpK  = 1.0f - clamp( g_lodLevlMorph.z - eyeDist * g_lodLevlMorph.w, 0.0, 1.0 );   
   
   //插值处理水平位置
   vPosInWorld.xz = MorphVertex( vPos, vPosInWorld.xz, morphLerpK );
      
   //计算水平位置和高度
   GlobalUV = GetGlobalUV(vPosInWorld.xz);
   vPosInWorld.y = g_terrainHMVertexTexture.SampleLevel( g_samLinear, GlobalUV, 0).x*g_terrainScale.y + g_terrainOffset.y;
}


VS_OUTPUT TerrainVS(float3 vPos : POSITION)
{
   VS_OUTPUT output;
   float4 vPosInWorld;
   float2 GlobalUV;
   float  morphLerpK;
   SampleTerrainVertex(vPos, vPosInWorld, GlobalUV,morphLerpK);

   //MVP转换
   vPosInWorld.w = 1;
   output.Position = mul(vPosInWorld, g_viewProjection);
   output.globalUV_detUV = float4(GlobalUV,0,0);
   output.lightDir	= float4(g_diffuseLightDir.xyz,0.0);
   output.eyeDir	= float4(normalize(g_cameraPos.xyz - vPosInWorld.xyz), 0);
   output.PosInShadowView = mul(vPosInWorld, g_shadowViewProj);
   output.PosInShadowView.x = output.PosInShadowView.x * 0.5f + 0.5f;
   output.PosInShadowView.y = -output.PosInShadowView.y * 0.5f + 0.5f;
   output.PosInShadowView.w = 1.0f-morphLerpK;
   
   output.PosInShadowView1 = mul(vPosInWorld, g_shadowViewProj1);
   output.PosInShadowView1.x = output.PosInShadowView1.x * 0.5f + 0.5f;
   output.PosInShadowView1.y = -output.PosInShadowView1.y * 0.5f + 0.5f;
   output.PosInShadowView1.w = morphLerpK;

   return output;   
}



SHADOW_VS_OUTPUT TerrainShadowVS(float3 vPos : POSITION)
{
	SHADOW_VS_OUTPUT output;

	float4 vPosInWorld;
	float2 GlobalUV;
	float  morphLerpK;
	SampleTerrainVertex(vPos, vPosInWorld, GlobalUV, morphLerpK);	
	vPosInWorld.w = 1;
	output.Position	= mul(vPosInWorld,g_viewProjection);
	output.Depth = vPosInWorld.zw;
	return output;
}