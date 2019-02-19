#pragma once
#include <xnamath.h>
#include <d3d11.h>
#include <d3dx11.h>
#include "DXUTcamera.h"
#include "CDLODTerrainQuadTree.h"
#include "D3D11EventManager.h"
#include "TiledBitmap.h"
#include "CDLODTerrainShader.h"
#include "CDLODTerrainShadow.h"

class D3D11CDLODTerrainGridMesh : public CDLODTerrainGridMesh<XMFLOAT3>
{
public:
	D3D11CDLODTerrainGridMesh():CDLODTerrainGridMesh(32){}

	HRESULT Create(ID3D11Device*);
	void	Destroy();

	ID3D11Buffer*           GetVertexBuffer(){ return m_pVertexBuffer; }
	ID3D11Buffer*           GetIndexBuffer(){ return m_pIndexBuffer; }
	DXGI_FORMAT				GetIndexFormat(){ return DXGI_FORMAT_R16_UINT;}

private:

	ID3D11Buffer*           m_pVertexBuffer;
	ID3D11Buffer*           m_pIndexBuffer;
};
struct TerrainRendFillMode{
	enum Type
	{
		WireFrame,
		Solid,
	};
};

class SimpleHeightmapSource;
class CDLODTerrainRender : public D3D11EventListener
{
	template<int Count>
	struct GroundTextureInfo
	{
		GroundTextureInfo(){memset(mTexture, 0, sizeof(mTexture));}
		ID3D11ShaderResourceView*	mTexture[Count];
		std::string					mFileName[Count];
		static const int			nTextureCount = Count;
	};
public:

	CDLODTerrainRender(void);
	~CDLODTerrainRender(void);

	void	Update(const CBaseCamera& camera);
	HRESULT Render(const CBaseCamera& camera);

	void	DrawSelector(ID3D11DeviceContext* pImmediateContext, bool drawDepth, const CDLODTerrainQuadTree::NodeSlector& selector);
	void	DebugDrawSelector( const CDLODTerrainQuadTree::NodeSlector& selected );
	void	SetDebugDraw(bool bDebugDraw){ m_bDebugDraw=bDebugDraw; }
	bool	GetDebugDraw() const { return m_bDebugDraw; }

	void						SetRendFillMode(TerrainRendFillMode::Type raster) { m_rendRasterType = raster; }
	TerrainRendFillMode::Type	GetRendFillMode() const { return m_rendRasterType; }


private:
	virtual HRESULT OnCreateDevice( ID3D11Device*, const DXGI_SURFACE_DESC*);
	virtual void	OnDestroyDevice();
	void			RendTerrainDepthMap(ID3D11DeviceContext* pImmediateContext);

	CDLODTerrainQuadTree::NodeSlector	m_selector;

	bool						m_bDebugDraw;
	CDLODTerrainQuadTree		m_quadTree;
	D3D11CDLODTerrainGridMesh	m_gridMesh;

	CDLODTerrainVShader		m_VertexShader;
	CDLODTerrainPShader		m_PixelShader;

	ID3D11RasterizerState*		m_pRasterizerState[2];
	ID3D11Texture2D*			m_HeightMapTexture;
	ID3D11Texture2D*			m_NormalMapTexture;
	ID3D11ShaderResourceView*	m_HeightMapShaderResView;
	ID3D11ShaderResourceView*	m_NormalMapShaderResView;
	ID3D11SamplerState*			m_pTerrainHMapSS;
	ID3D11SamplerState*			m_pGroundTextureSS;

	ID3D11ShaderResourceView*	m_TestShaderResView;
	VertexAsylum::TiledBitmap *	m_heightmap;
	SimpleHeightmapSource*		m_heightmapSrc;

	TerrainRendFillMode::Type	m_rendRasterType;
	GroundTextureInfo<6>		m_groundTextures;

	CDLODTerrainShadow			m_terrainShadow[CDLODTerrainQuadTree::MAX_LOD_LEVELS];

	Light						m_lightInfo;
	D3D11PixelShader			m_genVsmPS;

	ID3D11Texture2D*			m_pCascadedShadowMapVarianceTextureArray;
	ID3D11Texture2D*			m_pCascadedShadowMapTempBlurTexture;
	ID3D11RenderTargetView*		m_pCascadedShadowMapTempBlurRTV;
	ID3D11ShaderResourceView*	m_pCascadedShadowMapTempBlurSRV;

	D3D11PixelShader			m_blurHPixelShader;
	D3D11PixelShader			m_blurVPixelShader;

	int							m_shadowmapWidth;
	int							m_shadowmapHeight;

	static const DXGI_FORMAT	m_ShadowBufferFormat = DXGI_FORMAT_R32G32_FLOAT;




};
