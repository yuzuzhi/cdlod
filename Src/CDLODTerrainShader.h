#pragma once
#include <xnamath.h>
#include "D3D11Shader.h"


class CDLODTerrainVShader : public D3D11VertexShader
{
public:
	struct cbPerFrame
	{
		enum { Register = 0};
		XMMATRIX g_viewProjection;
		XMVECTOR g_cameraPos;
		XMVECTOR g_diffuseLightDir;
	};

	struct cbNodeInfo
	{
		enum { Register = 1};

		XMVECTOR	g_quadOffset;
		XMVECTOR	g_quadScale;
	};

	struct cbTerrainInfo
	{
		enum { Register = 2};

		void SetOffset(float x, float y, float z);
		void SetScale(float x, float y, float z);
		void SetHeightMapInfo(int width, int height);
		void SetGridMeshDim(int dim);
		XMVECTOR	g_terrainScale;
		XMVECTOR	g_terrainOffset;
		XMVECTOR	g_heightmapTextureInfo;
		XMVECTOR	g_samplerWorldToTextureScale;
		XMVECTOR	g_gridDim;
	};

	struct cbDebugInfo
	{
		enum { Register = 3,};
		XMFLOAT4 g_debugVars;
	};

	struct cbLodLevelInfo
	{
		enum { Register = 4, };
		void SetLodLevlMorph(float morphStart, float morphEnd);
		XMMATRIX	g_shadowViewProj;
		XMMATRIX	g_shadowViewProj1;
		XMVECTOR	g_lodLevlMorph;
	};

	static const int CONST_BUFFER_COUNT = 5;

	static const D3D11_INPUT_ELEMENT_DESC INPUT_ELEM_DESC[];
	static const int INPUT_ELEM_DESC_COUNT;

	CDLODTerrainVShader(bool bDrawDepth = false);

	virtual HRESULT OnCreat(ID3D11Device*);
	virtual void	OnDestroy();
	template<class T>
	void			UpdateSubresource(ID3D11DeviceContext*pContext, const T& info)
	{
		pContext->UpdateSubresource( m_pConstBuffers[info.Register], 0, NULL, &info, 0, 0 );
	}

	ID3D11Buffer* m_pConstBuffers[CONST_BUFFER_COUNT];
};

class CDLODTerrainPShader : public D3D11PixelShader
{
public:
	CDLODTerrainPShader():D3D11PixelShader(L"Media/CDLODTerrainPS.hlsl","TerrainPS"){}

	struct cbLightInfo
	{
		enum { Register = 0};
		XMVECTOR	g_lightColorDiffuse;
		XMVECTOR	g_lightColorAmbient;
		XMVECTOR	g_fogColor;
	};

	virtual HRESULT OnCreat(ID3D11Device*);
	virtual void	OnDestroy();


	ID3D11Buffer* m_pLightInfo;
};



inline void CDLODTerrainVShader::cbLodLevelInfo::SetLodLevlMorph(float fStart, float fEnd)
{
	float fRange = fEnd - fStart;
	g_lodLevlMorph = XMVectorSet(fStart, 1.f / fRange, fEnd / fRange, 1.f / fRange);
}
inline void CDLODTerrainVShader::cbTerrainInfo::SetOffset(float x, float y, float z)
{
	g_terrainOffset = XMVectorSet(x, y, z, 0.0f);
}
inline void CDLODTerrainVShader::cbTerrainInfo::SetScale(float x, float y, float z)
{
	g_terrainScale = XMVectorSet(x, y, z, 0.0f);
}
inline void CDLODTerrainVShader::cbTerrainInfo::SetHeightMapInfo(int width, int height)
{
	g_heightmapTextureInfo = XMVectorSet(width, height, 1.f / (float)width, 1.f / (float)height);
	g_samplerWorldToTextureScale = XMVectorSet((width - 1.f) / (float)width, (height - 1.f) / (float)height, 0.f, 0.f);
}
inline void CDLODTerrainVShader::cbTerrainInfo::SetGridMeshDim(int dim)
{
	g_gridDim = XMVectorSet((float)dim, dim*0.5f, 2.0f / dim, 0.0f);
}

