#include "dxut.h"
#include "D3D11HelperRender.h"
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, const D3D_SHADER_MACRO* pDefines = NULL );

HelpRender::HelpRender(){}
HelpRender::~HelpRender(){}

HRESULT HelpRender::OnCreateDevice( ID3D11Device*pd3dDevice, const DXGI_SURFACE_DESC* )
{
	HRESULT hr;
	ID3DBlob* pBlob = NULL;

	///////////////////////////////////////////////////////////////////
	//
	// FBX vertex layout
	//
	//////////////////////////////////////////////////////////////////
	CompileShaderFromFile( L"Media\\shaders\\simpleFBX.hlsl", "VSMain", "vs_4_0", &pBlob );
	// Create our vertex input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	V_RETURN(pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pBlob->GetBufferPointer(),
				pBlob->GetBufferSize(), &pFBXVertexLayout11 ));
	DXUT_SetDebugName( pFBXVertexLayout11, "pFBXVertexLayout11" );

	///////////////////////////////////////////////////////////////////
	//
	// Create a screen quad for render to texture operations
	//
	//////////////////////////////////////////////////////////////////
	// Create the shaders	
	CompileShaderFromFile( L"Media\\shaders\\quadVS.hlsl", "VSMain", "vs_4_0", &pBlob );
	V_RETURN(pd3dDevice->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &quadVS ));
	DXUT_SetDebugName( quadVS, "quadVS" );

	const D3D11_INPUT_ELEMENT_DESC quadlayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	V_RETURN(pd3dDevice->CreateInputLayout( quadlayout, 2, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &quadLayout ));
	DXUT_SetDebugName( quadLayout, "Quad" );
	SAFE_RELEASE(pBlob);


	SCREEN_VERTEX svQuad[4];
	svQuad[0].pos = D3DXVECTOR4( -1.0f, 1.0f, 0.5f, 1.0f );
	svQuad[0].tex = D3DXVECTOR2( 0.0f, 0.0f );
	svQuad[1].pos = D3DXVECTOR4( 1.0f, 1.0f, 0.5f, 1.0f );
	svQuad[1].tex = D3DXVECTOR2( 1.0f, 0.0f );
	svQuad[2].pos = D3DXVECTOR4( -1.0f, -1.0f, 0.5f, 1.0f );
	svQuad[2].tex = D3DXVECTOR2( 0.0f, 1.0f );
	svQuad[3].pos = D3DXVECTOR4( 1.0f, -1.0f, 0.5f, 1.0f );
	svQuad[3].tex = D3DXVECTOR2( 1.0f, 1.0f );

	D3D11_BUFFER_DESC vbdesc =
	{
		4 * sizeof( SCREEN_VERTEX ),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0,
		0
	};
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = svQuad;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	V_RETURN(pd3dDevice->CreateBuffer( &vbdesc, &InitData, &screenQuadVB ));
	DXUT_SetDebugName( screenQuadVB, "ScreenQuad" );

	///////////////////////////////////////////////////////////////////
	//
	// World Axes
	//
	//////////////////////////////////////////////////////////////////

	AXIS_VERTEX axis[3];
	axis[0].pos = D3DXVECTOR4( 1.0f, 0.0f, 0.0f, 1.0f );
	axis[1].pos = D3DXVECTOR4( 0.0f, 1.0f, 0.0f, 1.0f );
	axis[2].pos = D3DXVECTOR4( 0.0f, 0.0f, 1.0f, 1.0f );

	vbdesc.ByteWidth = 3 * sizeof( AXIS_VERTEX );
	InitData.pSysMem = axis;
	V_RETURN(pd3dDevice->CreateBuffer( &vbdesc, &InitData, &axisVB ));
	DXUT_SetDebugName( axisVB, "axisVB" );


	CompileShaderFromFile( L"Media\\shaders\\axis.hlsl", "GSMain", "gs_5_0", &pBlob );
	V_RETURN(pd3dDevice->CreateGeometryShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &axisGS ));
	DEBUG_NAME(axisGS);
	V_RETURN(CompileShaderFromFile( L"Media\\shaders\\axis.hlsl", "PSMain", "ps_5_0", &pBlob ));
	V_RETURN(pd3dDevice->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &axisPS ));
	DEBUG_NAME(axisPS);
	V_RETURN(CompileShaderFromFile( L"Media\\shaders\\axis.hlsl", "VSMain", "vs_5_0", &pBlob ));
	V_RETURN(pd3dDevice->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &axisVS ));
	DEBUG_NAME(axisVS);

	const D3D11_INPUT_ELEMENT_DESC axislayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	V_RETURN(pd3dDevice->CreateInputLayout( axislayout, 1, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &axisLayout ));
	DEBUG_NAME(axisLayout);
	SAFE_RELEASE(pBlob);
	///////////////////////////////////////////////////////////////////
	//
	// Samplers and states
	//
	//////////////////////////////////////////////////////////////////

	// Create a sampler state
	D3D11_SAMPLER_DESC SamDesc;
	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamDesc.MipLODBias = 0.0f;
	SamDesc.MaxAnisotropy = 1;
	SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 1;
	SamDesc.BorderColor[0] = exp(40.0f);
	SamDesc.BorderColor[1] = -exp(-20.0f);
	SamDesc.BorderColor[2] = SamDesc.BorderColor[0] * SamDesc.BorderColor[0];
	SamDesc.BorderColor[3] = SamDesc.BorderColor[1] * SamDesc.BorderColor[1];
	SamDesc.MinLOD = 0;
	SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
	pd3dDevice->CreateSamplerState( &SamDesc, &linearSampler );
	DXUT_SetDebugName( linearSampler, "linearSampler" );

	SamDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	pd3dDevice->CreateSamplerState( &SamDesc, &pointSampler );
	DXUT_SetDebugName( pointSampler, "pointSampler" );

	SamDesc.Filter	 = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	pd3dDevice->CreateSamplerState( &SamDesc, &linearBorderSampler );
	DXUT_SetDebugName( linearBorderSampler, "linearBorderSampler" );

	D3D11_DEPTH_STENCIL_DESC NoZTestDesc; 
	memset(&NoZTestDesc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));
	NoZTestDesc.DepthEnable = FALSE;  
	//NoZTestDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	NoZTestDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	pd3dDevice->CreateDepthStencilState ( &NoZTestDesc , &zTestDisabledDSS );
	DXUT_SetDebugName( zTestDisabledDSS, "zTestDisabledDSS" );

	D3D11_RASTERIZER_DESC rDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	rDesc.CullMode = D3D11_CULL_NONE;
	pd3dDevice->CreateRasterizerState ( &rDesc , &nobackCullRS );
	DXUT_SetDebugName( nobackCullRS, "nobackCullRS" );

	D3D11_BLEND_DESC bDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	pd3dDevice->CreateBlendState ( &bDesc , &defaultBS );
	DEBUG_NAME( defaultBS );

	bDesc.AlphaToCoverageEnable = false;
	bDesc.IndependentBlendEnable = false;
	bDesc.RenderTarget[0].BlendEnable = true;
	bDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	pd3dDevice->CreateBlendState ( &bDesc , &alphaBlendBS );
	DXUT_SetDebugName( alphaBlendBS, "alphaBlendBS" );
	return S_OK;
}

void HelpRender::OnDestroyDevice()
{
	SAFE_RELEASE(quadLayout);
	SAFE_RELEASE(quadVS);
	SAFE_RELEASE(screenQuadVB);
	SAFE_RELEASE(linearSampler);
	SAFE_RELEASE(pointSampler);


	SAFE_RELEASE(linearBorderSampler);

	SAFE_RELEASE(nobackCullRS);
	SAFE_RELEASE(zTestDisabledDSS);
	SAFE_RELEASE(alphaBlendBS);
	SAFE_RELEASE(defaultBS);


	SAFE_RELEASE(pFBXVertexLayout11);


	SAFE_RELEASE(axisVB);
	SAFE_RELEASE(axisLayout);
	SAFE_RELEASE(axisVS);
	SAFE_RELEASE(axisGS);
	SAFE_RELEASE(axisPS);
}

void HelpRender::DrawFullScreenQuad(	ID3D11DeviceContext* pd3dImmediateContext, 
										ID3D11PixelShader* pPS)
{

    UINT strides = sizeof( SCREEN_VERTEX );
    UINT offsets = 0;
    ID3D11Buffer* pBuffers[1] = { screenQuadVB };

    pd3dImmediateContext->IASetInputLayout( quadLayout );
    pd3dImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
    pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	ID3D11DepthStencilState*		prevDSS;
	UINT							ref;
	pd3dImmediateContext->OMGetDepthStencilState(&prevDSS, &ref);
	pd3dImmediateContext->OMSetDepthStencilState(zTestDisabledDSS, 0);

    pd3dImmediateContext->VSSetShader( quadVS, NULL, 0 );
    pd3dImmediateContext->PSSetShader( pPS, NULL, 0 );
    pd3dImmediateContext->Draw( 4, 0 );

	 // Restore the Old viewport
	pd3dImmediateContext->OMSetDepthStencilState(prevDSS, ref);
}

void HelpRender::DrawQuad( ID3D11DeviceContext* pd3dImmediateContext, 
							ID3D11PixelShader* pPS,
							float x, float y, float size )
{
	// Save the old viewport
    D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
    UINT nViewPorts = 1;
    pd3dImmediateContext->RSGetViewports( &nViewPorts, vpOld );

	float s = vpOld[0].Width / vpOld[0].Height;

    // Setup the viewport to match the backbuffer
    D3D11_VIEWPORT vp;
    vp.Width = (float)size;
    vp.Height = (float)size;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
	vp.TopLeftX = x;
	vp.TopLeftY = y;
    pd3dImmediateContext->RSSetViewports( 1, &vp );

    UINT strides = sizeof( SCREEN_VERTEX );
    UINT offsets = 0;
    ID3D11Buffer* pBuffers[1] = { screenQuadVB };

    pd3dImmediateContext->IASetInputLayout( quadLayout );
    pd3dImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
    pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	ID3D11DepthStencilState*		prevDSS;
	UINT							ref;
	pd3dImmediateContext->OMGetDepthStencilState(&prevDSS, &ref);
	pd3dImmediateContext->OMSetDepthStencilState(zTestDisabledDSS, 0);
	

    pd3dImmediateContext->VSSetShader( quadVS, NULL, 0 );
    pd3dImmediateContext->PSSetShader( pPS, NULL, 0 );
    pd3dImmediateContext->Draw( 4, 0 );

    // Restore the Old viewport
    pd3dImmediateContext->RSSetViewports( nViewPorts, vpOld );
	pd3dImmediateContext->OMSetDepthStencilState(prevDSS, ref);
}
#if 0
unsigned HelpRender::RenderMeshNode(ID3D11DeviceContext* pd3dImmediateContext, FBXScene::DX11MeshNode* node)
{
	FBXScene::DX11Mesh* dx11mesh = node->mesh;
	unsigned triangles_count = 0;
	if(dx11mesh)
	{
		UINT Strides[1];
		UINT Offsets[1];
		ID3D11Buffer* pVB[1];
		pVB[0] = dx11mesh->vertexBuffer;
		Strides[0] = dx11mesh->vertexStride;
		Offsets[0] = 0;
		pd3dImmediateContext->IASetInputLayout( pFBXVertexLayout11 );
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
		pd3dImmediateContext->IASetIndexBuffer( dx11mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0 );

		pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		//$TMP
		//if(dx11mesh->indexCount < 19750 && dx11mesh->indexCount > 19500)
		//if(dx11mesh->indexCount > 19500)
		pd3dImmediateContext->DrawIndexed( dx11mesh->indexCount, 0, 0 );
		triangles_count =  dx11mesh->indexCount/3;
	}
	for(unsigned int i = 0; i < node->childs.size(); i++)
		triangles_count += RenderMeshNode(pd3dImmediateContext, node->childs[i]);

	return triangles_count;
}
#endif

void HelpRender::DrawAxes(ID3D11DeviceContext* pd3dImmediateContext)
{
	// Save the old viewport
    D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
    UINT nViewPorts = 1;
    pd3dImmediateContext->RSGetViewports( &nViewPorts, vpOld );

	float s = 128;

    // Setup the viewport to match the backbuffer
    D3D11_VIEWPORT vp;
    vp.Width = 128;
    vp.Height = 128;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
	vp.TopLeftX = vpOld[0].Width - 10 - s;
	vp.TopLeftY = vpOld[0].Height - 10 - s;
    pd3dImmediateContext->RSSetViewports( 1, &vp );

    UINT strides = sizeof( AXIS_VERTEX );
    UINT offsets = 0;
    ID3D11Buffer* pBuffers[1] = { axisVB };

	pd3dImmediateContext->RSSetState(nobackCullRS);
	

    pd3dImmediateContext->IASetInputLayout( axisLayout );
    pd3dImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
    pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

    pd3dImmediateContext->VSSetShader( axisVS, NULL, 0 );
	pd3dImmediateContext->GSSetShader( axisGS, NULL, 0 );
    pd3dImmediateContext->PSSetShader( axisPS, NULL, 0 );
    pd3dImmediateContext->Draw( 3, 0 );

    // Restore the Old viewport
    pd3dImmediateContext->RSSetViewports( nViewPorts, vpOld );
}

HelpRender helperRender;
HelpRender* GetHelpRender()
{
	return &helperRender;
}
