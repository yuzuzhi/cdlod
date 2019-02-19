#include "DXUT.h"
#include "LayerShadowMap.h"
#include "AABB.h"
#include "DebugRender.h"

static const D3DXVECTOR3  MIN_VECTOR3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
static const D3DXVECTOR3  MAX_VECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);

template<>
void LayerShadowMap::CalculateViewProj(const Light& light, const ShadowFitArea& args, D3DXMATRIX* view, D3DXMATRIX* proj)
{

	AABB boundingBox;
	boundingBox.Min = args.mMin;
	boundingBox.Max = args.mMax;

	const D3DXVECTOR3 boundingSize = boundingBox.Size();


	D3DXVECTOR3 coners[8];
	boundingBox.GetCornerPoints(coners);
	AABB boxInLV; //
	boxInLV.Min = MAX_VECTOR3;
	boxInLV.Max = MIN_VECTOR3;
	for (int i = 0; i < 8; ++i)
	{
		float dx = D3DXVec3Dot(&light.mRight, &coners[i]);
		float dy = D3DXVec3Dot(&light.mUp, &coners[i]);
		float dz = D3DXVec3Dot(&light.mForward, &coners[i]);
		boxInLV.Min.x = min(boxInLV.Min.x, dx);
		boxInLV.Min.y = min(boxInLV.Min.y, dy);
		boxInLV.Min.z = min(boxInLV.Min.z, dz);
		boxInLV.Max.x = max(boxInLV.Max.x, dx);
		boxInLV.Max.y = max(boxInLV.Max.y, dy);
		boxInLV.Max.z = max(boxInLV.Max.z, dz);
	}

	D3DXVECTOR3 boxCenterInLV = boxInLV.Center();
	D3DXVECTOR3 boxSizeInLV = boxInLV.Size();

	D3DXVECTOR3 eye = light.mRight*boxCenterInLV.x + light.mUp*boxCenterInLV.y + light.mForward*boxInLV.Min.z;
	D3DXVECTOR3 lookat = eye + light.mForward;

	D3DXVECTOR2 size(boundingSize.x, boundingSize.z);
	mProjWidth = mProjHeight = D3DXVec2Length(&size);
	mProjDepth = boxSizeInLV.z;

	D3DXMatrixLookAtLH(view, &eye, &lookat, &light.mUp);
	D3DXMatrixOrthoLH(proj, mProjWidth, mProjHeight, 0, boxSizeInLV.z);
}



template<>
void LayerShadowMap::CalculateViewProj(const Light& light, const shadowfitCamera& args, D3DXMATRIX* view, D3DXMATRIX* proj)
{
	const float* camView = &args.mCamView.m[0][0];
	D3DXVECTOR3 xaxis(camView[0], camView[4], camView[8]);
	D3DXVECTOR3 yaxis(camView[1], camView[5], camView[9]);
	D3DXVECTOR3 zaxis(camView[2], camView[6], camView[10]);
	const D3DXVECTOR3& trans = args.mCamPos;

	D3DXVec3Normalize(&xaxis,&xaxis);
	D3DXVec3Normalize(&yaxis,&yaxis);
	D3DXVec3Normalize(&yaxis,&yaxis);
	float length1 = D3DXVec3LengthSq(&xaxis);
	float length2 = D3DXVec3LengthSq(&yaxis);
	float length3 = D3DXVec3LengthSq(&zaxis);

	
	const float tanX = 1.f / args.mCamProj._11;
	const float tanY = 1.f / args.mCamProj._22;
	const float zn = -args.mCamProj._43 / args.mCamProj._33;
	const float zf = zn * args.mCamProj._33 / (args.mCamProj._33-1.f);

	D3DXVECTOR3 points[] = 
	{
		D3DXVECTOR3(-tanX,-tanY,1),
		D3DXVECTOR3(-tanX, tanY,1),
		D3DXVECTOR3( tanX, tanY,1),
		D3DXVECTOR3( tanX,-tanY,1),

		D3DXVECTOR3(-tanX,-tanY,1),
		D3DXVECTOR3(-tanX, tanY,1),
		D3DXVECTOR3( tanX, tanY,1),
		D3DXVECTOR3( tanX,-tanY,1),
	};

	for (int i=0; i<8; ++i)
	{
		float fPlane = i<4? args.mLevelRange:zn;
		points[i] *= fPlane;
		points[i] = xaxis * points[i].x + yaxis * points[i].y + zaxis * points[i].z + trans;
	}


	D3DXVECTOR3 lightUp = zaxis;
	D3DXVECTOR3 lightRight;
	D3DXVECTOR3 lightForward;
	D3DXVec3Normalize(&lightForward, &light.mForward);
	D3DXVec3Cross(&lightRight, &lightUp, &lightForward);
	D3DXVec3Normalize(&lightRight, &lightRight);
	D3DXVec3Cross(&lightUp, &lightForward, &lightRight);
	D3DXVec3Normalize(&lightUp, &lightUp);

	const D3DXVECTOR3* corners = points;
	AABB boxInLV; //
	boxInLV.Min = MAX_VECTOR3;
	boxInLV.Max = MIN_VECTOR3;
	for (int i = 0; i < 8; ++i)
	{
		float dx = D3DXVec3Dot(&lightRight, &corners[i]);
		float dy = D3DXVec3Dot(&lightUp, &corners[i]);
		float dz = D3DXVec3Dot(&lightForward, &corners[i]);
		boxInLV.Min.x = min(boxInLV.Min.x, dx);
		boxInLV.Min.y = min(boxInLV.Min.y, dy);
		boxInLV.Min.z = min(boxInLV.Min.z, dz);
		boxInLV.Max.x = max(boxInLV.Max.x, dx);
		boxInLV.Max.y = max(boxInLV.Max.y, dy);
		boxInLV.Max.z = max(boxInLV.Max.z, dz);
	}

	D3DXVECTOR3 boxCenterInLV = boxInLV.Center();
	D3DXVECTOR3 boxSizeInLV = boxInLV.Size();

	D3DXVECTOR3 eye = lightRight*boxCenterInLV.x + lightUp*boxCenterInLV.y + lightForward*boxInLV.Min.z;
	D3DXVECTOR3 lookat = eye + lightForward;


	D3DXMatrixLookAtLH(view, &eye, &lookat, &lightUp);
	D3DXMatrixOrthoLH(proj, boxSizeInLV.x, boxSizeInLV.y, 0, boxSizeInLV.z);

}

__ImplShadowFitType(ShadowFitArea);
__ImplShadowFitType(shadowfitCamera);


LayerShadowMap::LayerShadowMap()
{
}


LayerShadowMap::~LayerShadowMap()
{
}


void LayerShadowMap::Update(const Light& light, const ShadowFitType& type)
{
	if (type.GetId()==ShadowFitArea::id)
	{
		CalculateViewProj(light, (const ShadowFitArea&)type, &mView, &mProj);
	}
	else if (type.GetId() == shadowfitCamera::id)
	{
		CalculateViewProj(light, (const shadowfitCamera&)type, &mView, &mProj);
	}

}