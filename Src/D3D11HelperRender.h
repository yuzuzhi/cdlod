#pragma once
#include <D3DX11.h>
#include "D3D11EventManager.h"

//#include "FBXScene.h"
#define DEBUG_NAME(a) DXUT_SetDebugName( a, #a)

class HelpRender : public D3D11EventListener
{
	virtual HRESULT OnCreateDevice( ID3D11Device*, const DXGI_SURFACE_DESC*);
	virtual void	OnDestroyDevice();
public:
	HelpRender();
	~HelpRender();

	void DrawFullScreenQuad( ID3D11DeviceContext* pd3dImmediateContext, 
							 ID3D11PixelShader* pPS );

	void DrawQuad( ID3D11DeviceContext* pd3dImmediateContext, 
				   ID3D11PixelShader* pPS,
				   float x, float y, float size );

	void DrawAxes(ID3D11DeviceContext* pd3dImmediateContext);

#if 0
	unsigned RenderMeshNode(ID3D11DeviceContext* pd3dImmediateContext, FBXScene::DX11MeshNode* node);
#endif
public:
	ID3D11SamplerState*				linearSampler;
	ID3D11SamplerState*				pointSampler;

	ID3D11SamplerState*				linearBorderSampler;

	ID3D11RasterizerState*			nobackCullRS;
	ID3D11DepthStencilState*		zTestDisabledDSS;
	ID3D11BlendState*				alphaBlendBS;
	ID3D11BlendState*				defaultBS;

private:

	ID3D11InputLayout*				pFBXVertexLayout11;

	// Stuff used for drawing the "full screen quad"
	struct SCREEN_VERTEX
	{
		D3DXVECTOR4 pos;
		D3DXVECTOR2 tex;
	};
	ID3D11Buffer*					screenQuadVB;
	ID3D11InputLayout*				quadLayout;
	ID3D11VertexShader*				quadVS;

	// Axis render
	struct AXIS_VERTEX
	{
		D3DXVECTOR4 pos;
	};
	ID3D11Buffer*					axisVB;
	ID3D11InputLayout*				axisLayout;
	ID3D11VertexShader*				axisVS;
	ID3D11GeometryShader*			axisGS;
	ID3D11PixelShader*				axisPS;
};

HelpRender* GetHelpRender();




//--------------------------------------------------------------------------------------
// Helper for creating constant buffers
//--------------------------------------------------------------------------------------
template <class T>
inline HRESULT CreateConstantBuffer(ID3D11Device* pd3dDevice, ID3D11Buffer** ppCB)
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = 0;
    Desc.MiscFlags = 0;
    Desc.ByteWidth = sizeof( T );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, ppCB ) );

    return hr;
}

//--------------------------------------------------------------------------------------
// Helper for creating constant buffers
//--------------------------------------------------------------------------------------
template <class T>
inline HRESULT CreatReadBuffer(ID3D11Device* pd3dDevice, UINT iNumElements, ID3D11Buffer** ppCB)
{
    HRESULT hr = S_OK;

	D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_STAGING;
    Desc.BindFlags = 0;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    Desc.MiscFlags = 0;
    Desc.ByteWidth = iNumElements * sizeof( T );
	Desc.StructureByteStride = sizeof( T );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, ppCB ) );

    return hr;
}


//--------------------------------------------------------------------------------------
// Helper for creating structured buffers with an SRV and UAV
//--------------------------------------------------------------------------------------
template <class T>
inline HRESULT CreateStructuredBuffer(ID3D11Device* pd3dDevice, UINT iNumElements, ID3D11Buffer** ppBuffer, ID3D11ShaderResourceView** ppSRV, ID3D11UnorderedAccessView** ppUAV, const T* pInitialData = NULL, UINT additionalFlags = 0)
{
    HRESULT hr = S_OK;

    // Create SB
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory( &bufferDesc, sizeof(bufferDesc) );
    bufferDesc.ByteWidth = iNumElements * sizeof(T);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | additionalFlags;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(T);

    D3D11_SUBRESOURCE_DATA bufferInitData;
    ZeroMemory( &bufferInitData, sizeof(bufferInitData) );
    bufferInitData.pSysMem = pInitialData;
    V_RETURN( pd3dDevice->CreateBuffer( &bufferDesc, (pInitialData)? &bufferInitData : NULL, ppBuffer ) );

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory( &srvDesc, sizeof(srvDesc) );
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = iNumElements;
    V_RETURN( pd3dDevice->CreateShaderResourceView( *ppBuffer, &srvDesc, ppSRV ) );

    // Create UAV
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    ZeroMemory( &uavDesc, sizeof(uavDesc) );
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = iNumElements;
    V_RETURN( pd3dDevice->CreateUnorderedAccessView( *ppBuffer, &uavDesc, ppUAV ) );

    return hr;
}

inline HRESULT CreateAppendStructuredBuffer(ID3D11Device* pd3dDevice, UINT iNumElements, ID3D11Buffer* ppBuffer, ID3D11UnorderedAccessView** ppUAV)
{
    HRESULT hr = S_OK;

    // Create UAV
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    ZeroMemory( &uavDesc, sizeof(uavDesc) );
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = iNumElements;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    V_RETURN( pd3dDevice->CreateUnorderedAccessView( ppBuffer, &uavDesc, ppUAV ) );

    return hr;
}

inline HRESULT CreateTexture2D(ID3D11Device* pd3dDevice, UINT W, UINT H, DXGI_FORMAT Format, ID3D11Texture2D** ppTexture, ID3D11ShaderResourceView** ppSRV, ID3D11RenderTargetView** ppRTV, UINT MultiSample = 1, UINT additionalFlags = 0, UINT MipLevels = 1)
{
    HRESULT hr = S_OK;

	D3D11_TEXTURE2D_DESC dtd = 
    {
        W,//UINT Width;
        H,//UINT Height;
        MipLevels,//UINT MipLevels;
        1,//UINT ArraySize;
        Format,//DXGI_FORMAT Format;
        MultiSample,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET | additionalFlags,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };
	V_RETURN( pd3dDevice->CreateTexture2D( &dtd, NULL, ppTexture  ) );

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = 
    {
        Format,
		MultiSample == 1 ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS,
        0,
        0
    };
    dsrvd.Texture2D.MipLevels	= MipLevels;
    V_RETURN( pd3dDevice->CreateShaderResourceView( *ppTexture, &dsrvd, ppSRV ) );

	D3D11_RENDER_TARGET_VIEW_DESC rtrvd = 
    {
        Format,
        MultiSample == 1 ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS,
        0
    };
	for(unsigned int i = 0; i < MipLevels; i++)
	{
		rtrvd.Texture2D.MipSlice = i;
		V_RETURN( pd3dDevice->CreateRenderTargetView( *ppTexture, &rtrvd, &ppRTV[i] ) );
	}

    return hr;
}

template <class T>
inline HRESULT CreateTexture3D(ID3D11Device* pd3dDevice, UINT W, UINT H, UINT D, DXGI_FORMAT Format, ID3D11Texture3D** ppTexture, ID3D11ShaderResourceView** ppSRV, ID3D11UnorderedAccessView** ppUAV = NULL, const T* pInitialData = NULL)
{
    HRESULT hr = S_OK;

	D3D11_TEXTURE3D_DESC dtd = 
    {
        W,//UINT Width;
        H,//UINT Height;
		D,//UINT Depth;
        1,//UINT MipLevels;
        Format,//DXGI_FORMAT Format;
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        ppUAV == NULL ? D3D11_BIND_SHADER_RESOURCE : D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_UNORDERED_ACCESS,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };

	D3D11_SUBRESOURCE_DATA bufferInitData;
    ZeroMemory( &bufferInitData, sizeof(bufferInitData) );
    bufferInitData.pSysMem = pInitialData;
	bufferInitData.SysMemPitch = (W) * sizeof(T);
	bufferInitData.SysMemSlicePitch = (W*H) * sizeof(T);

	V_RETURN( pd3dDevice->CreateTexture3D( &dtd, (pInitialData)? &bufferInitData : NULL, ppTexture  ) );

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = 
    {
        Format,
		D3D11_SRV_DIMENSION_TEXTURE3D,
        0,
        0
    };
    dsrvd.Texture2D.MipLevels	= 1;
    V_RETURN( pd3dDevice->CreateShaderResourceView( *ppTexture, &dsrvd, ppSRV ) );

	if(ppUAV)
	{
		 // Create UAV
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory( &uavDesc, sizeof(uavDesc) );
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		uavDesc.Texture3D.WSize = D;
		V_RETURN( pd3dDevice->CreateUnorderedAccessView( *ppTexture, &uavDesc, ppUAV ) );	
	}

    return hr;
}

inline HRESULT CreateDepthStencil(ID3D11Device* pd3dDevice, UINT W, UINT H, DXGI_FORMAT Format, ID3D11Texture2D** ppTexture, ID3D11DepthStencilView** ppDSV, UINT MultiSample = 1, ID3D11ShaderResourceView** ppSRV = NULL)
{
    HRESULT hr = S_OK;

	D3D11_TEXTURE2D_DESC dtd = 
    {
        W,//UINT Width;
        H,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        Format,//DXGI_FORMAT Format;
        MultiSample,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		ppSRV == 0 ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };
	V_RETURN( pd3dDevice->CreateTexture2D( &dtd, NULL, ppTexture  ) );

    D3D11_DEPTH_STENCIL_VIEW_DESC  dsvd = 
    {
        Format != DXGI_FORMAT_R32_TYPELESS ? Format : DXGI_FORMAT_D32_FLOAT,
        MultiSample == 1 ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS ,
        0
    };
    V_RETURN( pd3dDevice->CreateDepthStencilView( *ppTexture, &dsvd,ppDSV ) ); 

	if(ppSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = 
		{
			Format != DXGI_FORMAT_R32_TYPELESS ? Format : DXGI_FORMAT_R32_FLOAT,
			D3D11_SRV_DIMENSION_TEXTURE2D,
			0,
			0
		};
		dsrvd.Texture2D.MipLevels	= 1;
		V_RETURN( pd3dDevice->CreateShaderResourceView( *ppTexture, &dsrvd, ppSRV ) );
	}
	 
	return hr;
}
