#include "DXUT.h"
#include "CDLODTerrainShadow.h"

const int SHADOW_MAP_MS = 8;
CDLODTerrainShadow::CDLODTerrainShadow(void)
{
	m_shadowmapWidth = 4096;
	m_shadowmapHeight = 4096;
}


CDLODTerrainShadow::~CDLODTerrainShadow(void)
{
}

HRESULT CDLODTerrainShadow::OnCreateDevice( ID3D11Device*device, const DXGI_SURFACE_DESC* , ID3D11Texture2D* textureArray,int index)
{
	HRESULT hr;


	D3D11_TEXTURE2D_DESC desc;
	textureArray->GetDesc(&desc);

	m_shadowmapWidth = desc.Width;
	m_shadowmapHeight = desc.Height;


	DXGI_FORMAT texturefmt = DXGI_FORMAT_R32_TYPELESS;
	DXGI_FORMAT SRVfmt = DXGI_FORMAT_R32_FLOAT;
	DXGI_FORMAT DSVfmt = DXGI_FORMAT_D32_FLOAT;


	D3D11_TEXTURE2D_DESC depthBufferDesc = {
		m_shadowmapWidth,
		m_shadowmapHeight,
		1,//UINT MipLevels;
		1,//UINT ArraySize;
		texturefmt,//DXGI_FORMAT Format;
		1,//DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};
	V_RETURN(device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthMapTexture));


	D3D11_DEPTH_STENCIL_VIEW_DESC  depthStencilViewDesc = {
		DSVfmt,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0
	};
	V_RETURN(device->CreateDepthStencilView(m_depthMapTexture, &depthStencilViewDesc, &m_depthMapDSV));


	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {
		SRVfmt,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	V_RETURN(device->CreateShaderResourceView(m_depthMapTexture, &shaderResourceViewDesc, &m_depthMapSRV));

	//V_RETURN(m_shadowDepth.Create(device, m_shadowmapWidth, m_shadowmapHeight, DXGI_FORMAT_R32G32_FLOAT, SHADOW_MAP_MS));
	assert(textureArray);
	{
		D3D11_RENDER_TARGET_VIEW_DESC drtvd = {
			desc.Format,
			D3D11_RTV_DIMENSION_TEXTURE2DARRAY
		};
		drtvd.Texture2DArray.MipSlice = 0;
		drtvd.Texture2DArray.FirstArraySlice = index;
		drtvd.Texture2DArray.ArraySize = 1;
		V_RETURN( device->CreateRenderTargetView ( textureArray, &drtvd, &m_renderTarView) );

		D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = {
			desc.Format,
			D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
		};
		dsrvd.Texture2DArray.ArraySize = 1;
		dsrvd.Texture2DArray.FirstArraySlice = index;
		dsrvd.Texture2DArray.MipLevels = 1;
		dsrvd.Texture2DArray.MostDetailedMip = 0;
		V_RETURN( device->CreateShaderResourceView( textureArray, &dsrvd, &m_shaderResView ) );
	}


	return S_OK;
}

void CDLODTerrainShadow::OnDestroyDevice()
{
	SAFE_RELEASE(m_depthMapSRV);
	SAFE_RELEASE(m_depthMapTexture);
	SAFE_RELEASE(m_depthMapDSV);
	SAFE_RELEASE(m_depthMapDSV);
	SAFE_RELEASE(m_renderTarView);
	SAFE_RELEASE(m_shaderResView);
	//m_shadowDepth.Release();
}

HRESULT CDLODTerrainShadow::BeginRender(ID3D11DeviceContext* deviceContext)
{
	
	// Save the old viewport
	deviceContext->RSGetViewports( &nViewPorts, vpOld );

	m_viewport.Width = (float)m_shadowmapWidth;
	m_viewport.Height = (float)m_shadowmapHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	deviceContext->RSSetViewports(1, &m_viewport);
	ID3D11RenderTargetView* renderTar[1] = {m_renderTarView};
	deviceContext->OMSetRenderTargets(1, renderTar, m_depthMapDSV);
	float clearColor[4] = {1,1,1,1};
	deviceContext->ClearRenderTargetView(m_renderTarView, clearColor);
	deviceContext->ClearDepthStencilView(m_depthMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return S_OK;
}

void CDLODTerrainShadow::EndRender(ID3D11DeviceContext* deviceContext)
{
	ID3D11RenderTargetView* nullTarget[1] = {0};
	deviceContext->OMSetRenderTargets(1,nullTarget, NULL);
	deviceContext->RSSetViewports(nViewPorts, &vpOld[0]);
}


XMMATRIX CDLODTerrainShadow::GetViewProj() const
{
	D3DXMATRIX viewProj = mView * mProj;

	return XMMatrixSet(
		viewProj._11, viewProj._12, viewProj._13, viewProj._14,
		viewProj._21, viewProj._22, viewProj._23, viewProj._24,
		viewProj._31, viewProj._32, viewProj._33, viewProj._34,
		viewProj._41, viewProj._42, viewProj._43, viewProj._44);
}