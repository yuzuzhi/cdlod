#include <d3dcompiler.h>
#include "DXUT.h"
#include "D3D11DebugRender.h"
struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};
struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMVECTOR f4Color;
};



// Create vertex buffer
SimpleVertex BOX_VERTICES[] =
{
	{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
	{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
	{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) },
	{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
	{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 0.0f, 1.0f, 1.0f ) },
	{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) },
	{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) },
	{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ) },
};
SimpleVertex CAMERABOX_VERTICES[] =
{
	{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
};

// Create index buffer
WORD BOX_INDICES[] =
{
	3,1,0,	2,1,3,
	0,5,4,	1,5,0,
	3,4,7,	0,4,3,
	1,6,5,	2,6,1,
	2,7,6,	3,7,2,
	6,4,5,	7,4,6,
};

WORD BOX_EDGE_INDICES[] = 
{
	0,1,
	1,2,
	2,3,
	0,3,
	4,5,
	5,6,
	6,7,
	4,7,
	0,4,
	1,5,
	2,6,
	3,7,
};



const D3D11_INPUT_ELEMENT_DESC DrawLayerVertexShader::m_inputElemDesc[] = {
	{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
const int DrawLayerVertexShader::m_inputElemCount = ARRAYSIZE( m_inputElemDesc );



const D3D11_INPUT_ELEMENT_DESC D3D11DebugRender::m_draw3DInputElemDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
const int D3D11DebugRender::m_draw3DInputElemCount = ARRAYSIZE( m_draw3DInputElemDesc );

DrawLayerVertexShader::DrawLayerVertexShader()
	:D3D11VertexShader(L"Media/DebugRender.fx","LayerVS", m_inputElemDesc,m_inputElemCount)
	,m_pConstantBuffer(NULL){}
HRESULT	DrawLayerVertexShader::OnCreat(ID3D11Device* pd3dDevice)
{
	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(cbLayerPos);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &m_pConstantBuffer));
	return S_OK;
}
void DrawLayerVertexShader::OnDestroy()
{
	SAFE_RELEASE(m_pConstantBuffer);
}

D3D11DebugRender::D3D11DebugRender(void):
	m_layerVertexShader(),
	m_layerPixelShader(L"Media/DebugRender.fx","LayerPS"),
	m_draw3DVertexShader(L"Media/DebugRender.fx","VS",m_draw3DInputElemDesc,m_draw3DInputElemCount),
	m_draw3DPixelShader(L"Media/DebugRender.fx","PS")
{
}

D3D11DebugRender::~D3D11DebugRender(void)
{
}


HRESULT D3D11DebugRender::OnCreateDevice( ID3D11Device*, const DXGI_SURFACE_DESC*)
{ 
	HRESULT hr;

	ID3D11Device*pd3dDevice = DXUTGetD3D11Device();

	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( SimpleVertex ) * 8;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = BOX_VERTICES;
	V_RETURN(pd3dDevice->CreateBuffer( &bd, &InitData, &m_pBoxVertexBuffer));
	
	InitData.pSysMem = CAMERABOX_VERTICES;
	V_RETURN(pd3dDevice->CreateBuffer(&bd, &InitData, &m_pCameraBoxVertexBuffer));

	XMFLOAT3 QuadLayerVer[] = 
	{
		XMFLOAT3(-1,-1,0),XMFLOAT3(-1, 1,0),XMFLOAT3( 1, 1,0),
		XMFLOAT3(-1,-1,0),XMFLOAT3( 1, 1,0),XMFLOAT3( 1,-1,0),
	};
	bd.ByteWidth = sizeof( QuadLayerVer );
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = &QuadLayerVer[0];
	V_RETURN(pd3dDevice->CreateBuffer( &bd, &InitData, &m_pLayerVertexBuffer));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( WORD ) * 36;        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = BOX_INDICES;
	V_RETURN(pd3dDevice->CreateBuffer( &bd, &InitData, &m_pBoxIndexBuffer));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( BOX_EDGE_INDICES );
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = BOX_EDGE_INDICES;
	V_RETURN(pd3dDevice->CreateBuffer( &bd, &InitData, &m_pBoxEdgeIndexBuffer));

	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &m_pConstantBuffer));


	// Create solid and wireframe rasterizer state objects
	D3D11_RASTERIZER_DESC RasterDesc;
	ZeroMemory( &RasterDesc, sizeof( D3D11_RASTERIZER_DESC ) );
	RasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	RasterDesc.CullMode = D3D11_CULL_NONE;
	RasterDesc.DepthClipEnable = TRUE;
	V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &m_pRasterizerStateWireframe[0] ) );
	DXUT_SetDebugName( m_pRasterizerStateWireframe[0], "Wireframe" );

	RasterDesc.FillMode = D3D11_FILL_SOLID;
	RasterDesc.CullMode = D3D11_CULL_BACK;
	V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &m_pRasterizerStateWireframe[1] ) );
	DXUT_SetDebugName( m_pRasterizerStateWireframe[1], "Wireframe" );


	D3D11_BLEND_DESC blendDesc;
	//
	blendDesc.AlphaToCoverageEnable = false;	//¹Ø±Õ
	blendDesc.IndependentBlendEnable = false;	//
	//
	blendDesc.RenderTarget[0].BlendEnable	= true;
	blendDesc.RenderTarget[0].SrcBlend		= D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend		= D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp		= D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha	= D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha= D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha	= D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;
	pd3dDevice->CreateBlendState(&blendDesc, &m_pBlendState);

	V_RETURN(m_layerVertexShader.Create(pd3dDevice));
	V_RETURN(m_layerPixelShader.Create(pd3dDevice));
	V_RETURN(m_draw3DVertexShader.Create(pd3dDevice));
	V_RETURN(m_draw3DPixelShader.Create(pd3dDevice));

	return S_OK;
}
void D3D11DebugRender::OnDestroyDevice()
{
	SAFE_RELEASE(m_pBoxVertexBuffer);
	SAFE_RELEASE(m_pCameraBoxVertexBuffer);
	SAFE_RELEASE(m_pBoxIndexBuffer);
	SAFE_RELEASE(m_pBoxEdgeIndexBuffer);
	SAFE_RELEASE(m_pConstantBuffer);
	SAFE_RELEASE(m_pRasterizerStateWireframe[0]);
	SAFE_RELEASE(m_pRasterizerStateWireframe[1]);
	SAFE_RELEASE(m_pBlendState);
	SAFE_RELEASE(m_pLayerVertexBuffer);


	m_layerVertexShader.Destroy();
	m_layerPixelShader.Destroy();
	m_draw3DVertexShader.Destroy();
	m_draw3DPixelShader.Destroy();

}

void  D3D11DebugRender::DrawBox( const D3DXVECTOR3 & min, const D3DXVECTOR3 & max, unsigned int penColor, unsigned int brushColor, const D3DXMATRIX * transform)
{
	m_drawItems.push_back( DrawItem( min, max, D3DXVECTOR3(0,0,0), penColor, brushColor, DrawItem::Box ) );
}

void D3D11DebugRender::DrawOBB(const D3DXVECTOR3&center, const D3DXVECTOR3& forward, const D3DXVECTOR3&right, const D3DXVECTOR3& up)
{
	DrawItem item;
	item.type = DrawItem::OBB;
	item.v0 = center;
	item.v1 = forward;
	item.v2 = right;
	item.v3 = up;
}

void D3D11DebugRender::DrawTriangle( const D3DXVECTOR3 & v0, const D3DXVECTOR3 & v1, const D3DXVECTOR3 & v2, unsigned int penColor, unsigned int brushColor /*= 0x000000*/, const D3DXMATRIX * transform /*= NULL */ )
{

}

void D3D11DebugRender::DrawQuad( const D3DXVECTOR3 & v0, const D3DXVECTOR3 & v1, const D3DXVECTOR3 & v2, const D3DXVECTOR3 & v3, unsigned int penColor, unsigned int brushColor /*= 0x000000*/, const D3DXMATRIX * transform /*= NULL */ )
{

}

void D3D11DebugRender::Render( const XMMATRIX& view, const XMMATRIX& proj )
{
	ID3D11DeviceContext*pImmediateContext = DXUTGetD3D11DeviceContext();


	ID3D11BlendState*	pOldBlendState;
	float				oldBlendFactor[4];
	UINT				oldSampMask = 0;
	pImmediateContext->OMGetBlendState(&pOldBlendState, oldBlendFactor, &oldSampMask);

	UINT stride = sizeof( SimpleVertex );
	UINT offset = 0;
	pImmediateContext->IASetInputLayout(m_draw3DVertexShader.GetInputLayout());//pImmediateContext->IASetInputLayout( m_pVertexLayout );

	float factor[4] = {1.f,1.f,1.f,1.f};
	pImmediateContext->RSSetState( m_pRasterizerStateWireframe[1]);
	pImmediateContext->OMSetBlendState(m_pBlendState, factor, 0xFFFFFFFF);
	pImmediateContext->VSSetShader( m_draw3DVertexShader.GetShader(), NULL, 0 );
	pImmediateContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );
	pImmediateContext->PSSetShader( m_draw3DPixelShader.GetShader(), NULL, 0 );

	XMMATRIX matrixTrans;
	XMMATRIX matrixScale;
	XMMATRIX matrixWorld;
	D3DXVECTOR3 vPos;
	D3DXVECTOR3 vSize;
	for( size_t i = 0; i < m_drawItems.size(); i++ )
	{
		DrawItem & item = m_drawItems[i];
		if (item.type==DrawItem::Box)
		{
			vPos = (item.v1 + item.v0)*0.5f;
			vSize = (item.v1 - item.v0)*0.5f;
			matrixTrans = XMMatrixTranslation(vPos.x, vPos.y, vPos.z);
			matrixScale = XMMatrixScaling(vSize.x, vSize.y, vSize.z);
			matrixWorld = XMMatrixMultiply(matrixScale,matrixTrans);

			ConstantBuffer cb1;
			cb1.mWorld = XMMatrixTranspose( matrixWorld );
			cb1.mView = XMMatrixTranspose( view );
			cb1.mProjection = XMMatrixTranspose( proj );
			XMCOLOR color(item.penColor);
			cb1.f4Color = XMLoadColor(&color);
			pImmediateContext->UpdateSubresource( m_pConstantBuffer, 0, NULL, &cb1, 0, 0 );

			//
			pImmediateContext->OMSetBlendState(m_pBlendState, factor, 0xFFFFFFFF);
			pImmediateContext->IASetVertexBuffers( 0, 1, &m_pBoxVertexBuffer, &stride, &offset );
			pImmediateContext->IASetIndexBuffer( m_pBoxIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
			pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			pImmediateContext->DrawIndexed( 36, 0, 0 );

			pImmediateContext->OMSetBlendState(pOldBlendState, oldBlendFactor, oldSampMask);
			pImmediateContext->IASetVertexBuffers( 0, 1, &m_pBoxVertexBuffer, &stride, &offset );
			pImmediateContext->IASetIndexBuffer( m_pBoxEdgeIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
			pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
			pImmediateContext->DrawIndexed( sizeof(BOX_EDGE_INDICES)/sizeof(WORD), 0, 0 );
		}

		if (item.type==DrawItem::OBB)
		{

		}
	}

	pImmediateContext->OMSetBlendState(pOldBlendState, oldBlendFactor, oldSampMask);

	m_drawItems.clear();
}

void D3D11DebugRender::RenderCamera( const XMMATRIX& view, const XMMATRIX& proj, const XMMATRIX& camViewProj, unsigned int brushColor )
{
	ID3D11DeviceContext*pImmediateContext = DXUTGetD3D11DeviceContext();

	ID3D11BlendState*	pOldBlendState;
	float				oldBlendFactor[4];
	UINT				oldSampMask = 0;
	pImmediateContext->OMGetBlendState(&pOldBlendState, oldBlendFactor, &oldSampMask);
	UINT stride = sizeof( SimpleVertex );
	UINT offset = 0;
	pImmediateContext->IASetInputLayout( m_draw3DVertexShader.GetInputLayout() );


	XMVECTOR Determin;
	XMMATRIX invCamerViewProj = XMMatrixInverse(&Determin, camViewProj);//XMLoadFloat4x4((XMFLOAT4X4*)&invViewProj);

	ConstantBuffer cb1;
	cb1.mWorld = XMMatrixTranspose( invCamerViewProj );
	cb1.mView = XMMatrixTranspose( view );
	cb1.mProjection = XMMatrixTranspose( proj );
	XMCOLOR color(brushColor);
	
	cb1.f4Color = XMLoadColor(&color);

	pImmediateContext->UpdateSubresource( m_pConstantBuffer, 0, NULL, &cb1, 0, 0 );



	pImmediateContext->VSSetShader( m_draw3DVertexShader.GetShader(), NULL, 0 );
	pImmediateContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );
	pImmediateContext->PSSetShader( m_draw3DPixelShader.GetShader(), NULL, 0 );
	pImmediateContext->IASetVertexBuffers(0, 1, &m_pCameraBoxVertexBuffer, &stride, &offset);

	//draw frustum edge
	pImmediateContext->RSSetState( m_pRasterizerStateWireframe[0]);
	pImmediateContext->IASetIndexBuffer( m_pBoxEdgeIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
	pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
	pImmediateContext->DrawIndexed( sizeof(BOX_EDGE_INDICES)/sizeof(WORD), 0, 0 );

	//
	float factor[4] = {1.f,1.f,1.f,1.f};
	pImmediateContext->OMSetBlendState(m_pBlendState, factor, 0xFFFFFFFF);
	pImmediateContext->RSSetState( m_pRasterizerStateWireframe[1]);
	pImmediateContext->IASetIndexBuffer( m_pBoxIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
	pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	//pImmediateContext->DrawIndexed( 36, 0, 0 );

	pImmediateContext->OMSetBlendState(pOldBlendState, oldBlendFactor, oldSampMask);

}

void D3D11DebugRender::DrawTexture( ID3D11ShaderResourceView* pTexture, ID3D11SamplerState*pSampler,D3DXVECTOR2 tl, D3DXVECTOR2 br )
{
	ID3D11DeviceContext*pImmediateContext = DXUTGetD3D11DeviceContext();

	UINT stride = sizeof( XMFLOAT3 );
	UINT offset = 0;

	DrawLayerVertexShader::cbLayerPos pos;
	pos.g_layerPos = XMFLOAT4(tl.x,tl.y, br.x,br.y);
	pImmediateContext->UpdateSubresource( m_layerVertexShader.m_pConstantBuffer, 0, NULL, &pos, 0, 0 );
	pImmediateContext->VSSetConstantBuffers(DrawLayerVertexShader::cbLayerPos::Register,1,&m_layerVertexShader.m_pConstantBuffer);
	pImmediateContext->IASetVertexBuffers( 0, 1, &m_pLayerVertexBuffer, &stride, &offset );
	pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	pImmediateContext->IASetInputLayout( m_layerVertexShader.GetInputLayout() );
	pImmediateContext->VSSetShader( m_layerVertexShader.GetShader(), NULL, 0 );
	pImmediateContext->PSSetShader( m_layerPixelShader.GetShader(), NULL, 0 );
	pImmediateContext->PSSetShaderResources(0,1,&pTexture);
	pImmediateContext->PSSetSamplers(0,1,&pSampler);
	pImmediateContext->RSSetState( m_pRasterizerStateWireframe[1]);
	pImmediateContext->Draw(6,0);

}

static D3D11DebugRender d3d11Render;
DebugRender&GetDebugRender()
{
	return d3d11Render;
}