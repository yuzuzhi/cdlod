#include "DXUT.h"
#include "D3D11Shader.h"
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, const D3D_SHADER_MACRO* pDefines = NULL );

D3D11VertexShader::D3D11VertexShader(const WCHAR* file,const char* szEntryPoint,const D3D11_INPUT_ELEMENT_DESC*elems,int numElemDsc):m_strEntryPoint(szEntryPoint)
{
	wcscpy_s(m_strFile, file);
	m_pInputElemDesc = new D3D11_INPUT_ELEMENT_DESC[numElemDsc];
	memcpy(m_pInputElemDesc, elems, sizeof(D3D11_INPUT_ELEMENT_DESC)*numElemDsc);
	m_numInputElemDesc = numElemDsc;
}

HRESULT D3D11VertexShader::Create(ID3D11Device*pd3dDevice)
{
	HRESULT hr;
	ID3DBlob* pVertexShaderBuffer = NULL;
	V_RETURN( CompileShaderFromFile( m_strFile, m_strEntryPoint.c_str(), "vs_5_0", &pVertexShaderBuffer, m_shaderMacro) );

	V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader ) );


	//ARRAYSIZE( layout )
	V_RETURN( pd3dDevice->CreateInputLayout( m_pInputElemDesc, m_numInputElemDesc, pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(), &m_pVertexLayout ) );

	SAFE_RELEASE(pVertexShaderBuffer);

	V_RETURN(OnCreat(pd3dDevice));

	return S_OK;
}

void D3D11VertexShader::Destroy()
{
	OnDestroy();
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pVertexLayout);
}

HRESULT D3D11PixelShader::Create(ID3D11Device*pd3dDevice)
{
	HRESULT hr;
	ID3DBlob* pPixelShaderBuffer = NULL;
	V_RETURN( CompileShaderFromFile( m_strFile, m_strEntryPoint.c_str(), "ps_5_0", &pPixelShaderBuffer ) );

	V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
		pPixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader ) );
	DXUT_SetDebugName( m_pPixelShader, m_strEntryPoint.c_str() );

	V_RETURN(OnCreat(pd3dDevice));

	return S_OK;
}
void D3D11PixelShader::Destroy()
{
	OnDestroy();
	SAFE_RELEASE(m_pPixelShader);	
}


