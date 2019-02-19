#pragma once
#include "D3D11Shader.h"
#include "D3D11EventManager.h"
#include "LayerShadowMap.h"




class CDLODTerrainShadow : public LayerShadowMap
{
public:
	struct Settins
	{

	};

	CDLODTerrainShadow(void);
	~CDLODTerrainShadow(void);

	HRESULT			BeginRender(ID3D11DeviceContext* deviceContext);
	void			EndRender(ID3D11DeviceContext* deviceContext);
	XMMATRIX		GetViewProj() const;
	ID3D11ShaderResourceView* GetDepthMapShaderResView() { return m_shaderResView/*m_depthMapSRV*/; }
	ID3D11ShaderResourceView* GetRendTarShaderResView() { return m_shaderResView; }
	ID3D11RenderTargetView*		GetRenderTargetView() const { return m_renderTarView;}
	virtual HRESULT OnCreateDevice( ID3D11Device*, const DXGI_SURFACE_DESC*, ID3D11Texture2D* textureArray,int index);
	virtual void	OnDestroyDevice();
	ID3D11ShaderResourceView*	m_depthMapSRV;
	ID3D11Texture2D*			m_depthMapTexture;
	ID3D11DepthStencilView*		m_depthMapDSV;

	ID3D11ShaderResourceView*	m_shaderResView;
	ID3D11RenderTargetView*		m_renderTarView;
	
	D3D11_VIEWPORT				m_viewport;


	int m_shadowmapWidth;
	int m_shadowmapHeight;
	D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
	UINT nViewPorts;

};

