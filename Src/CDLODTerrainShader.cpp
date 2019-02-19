#include "DXUT.h"
#include "CDLODTerrainShader.h"

const D3D11_INPUT_ELEMENT_DESC CDLODTerrainVShader::INPUT_ELEM_DESC[] = {
	{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
const int CDLODTerrainVShader::INPUT_ELEM_DESC_COUNT = ARRAYSIZE( INPUT_ELEM_DESC );

CDLODTerrainVShader::CDLODTerrainVShader( bool bDrawDepth ):
	D3D11VertexShader(L"Media/CDLODTerrainVS.hlsl",bDrawDepth?"TerrainShadowVS":"TerrainVS",
	INPUT_ELEM_DESC,INPUT_ELEM_DESC_COUNT){}
HRESULT CDLODTerrainVShader::OnCreat( ID3D11Device* pd3dDevice)
{
	HRESULT hr;

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.ByteWidth = sizeof(cbPerFrame);
	V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &m_pConstBuffers[cbPerFrame::Register]));
	bd.ByteWidth = sizeof(cbNodeInfo);
	V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &m_pConstBuffers[cbNodeInfo::Register]));
	bd.ByteWidth = sizeof(cbTerrainInfo);
	V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &m_pConstBuffers[cbTerrainInfo::Register]));
	bd.ByteWidth = sizeof(cbDebugInfo);
	V_RETURN(pd3dDevice->CreateBuffer(&bd, NULL, &m_pConstBuffers[cbDebugInfo::Register]));
	bd.ByteWidth = sizeof(cbLodLevelInfo);
	V_RETURN(pd3dDevice->CreateBuffer(&bd, NULL, &m_pConstBuffers[cbLodLevelInfo::Register]));



	return S_OK;	
}

void CDLODTerrainVShader::OnDestroy()
{
	for(int i=0; i<CONST_BUFFER_COUNT; ++i)
		SAFE_RELEASE(m_pConstBuffers[i]);
}

HRESULT CDLODTerrainPShader::OnCreat( ID3D11Device* pd3dDevice )
{
	HRESULT hr;

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(cbLightInfo);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &m_pLightInfo));
	return S_OK;
}

void CDLODTerrainPShader::OnDestroy()
{
	SAFE_RELEASE(m_pLightInfo);
}

