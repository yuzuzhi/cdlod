//--------------------------------------------------------------------------------------
// File: shader_include.hlsl
//
// Include file for common shader definitions and functions.
//--------------------------------------------------------------------------------------

#define M_PI 3.141562
#include "settings.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer cbMain : register( b0 )
{
	matrix g_mWorld;                            // World matrix
	matrix g_mView;                             // View matrix
	matrix g_mProjection;                       // Projection matrix
    matrix g_mWorldViewProjection;              // WVP matrix
    matrix g_mViewProjection;                   // VP matrix
	matrix g_mWorldView;		                // WV matrix
    matrix g_mInvView;                          // Inverse of view matrix
	matrix g_mLightView;                        // Light View matrix
	matrix g_mLightProjection;                  // Light Projection matrix
    float4 g_vScreenResolution;                 // Screen resolution    	
	float4 g_vEye;					    		// Camera's location
	float4 g_vLightDirVS;						// Default light's direction at view space
	float4 g_vBackLightDirVS;					// Back light's direction at view space
	float4 g_vLightDir;							// Default light's direction at world space
	float4 g_vBackLightDir;						// Back light's direction at world space
	float4 g_vSettings;
};

struct VS_QUAD_OUTPUT
{
	float2 vTexcoord	: TEXCOORD0;
	float4 vPosition	: SV_POSITION;
};

