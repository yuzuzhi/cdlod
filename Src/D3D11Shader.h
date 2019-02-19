#include <d3d11.h>
#include <d3dx11.h>
#include <string>
#pragma once

class D3D11VertexShader
{
public:
	D3D11VertexShader(const WCHAR* file,const char* szEntryPoint,const D3D11_INPUT_ELEMENT_DESC*, int numElemDsc);
	HRESULT Create(ID3D11Device*);
	void	Destroy();

	virtual HRESULT		OnCreat(ID3D11Device*){ return S_OK; }
	virtual void		OnDestroy(){}

	ID3D11VertexShader* GetShader(){ return m_pVertexShader; }
	ID3D11InputLayout*	GetInputLayout(){ return m_pVertexLayout; }

private:

	WCHAR				m_strFile[255];
	std::string			m_strEntryPoint;
	ID3D11VertexShader*	m_pVertexShader;
	ID3D11InputLayout*	m_pVertexLayout;
	D3D11_INPUT_ELEMENT_DESC*	m_pInputElemDesc;
	int							m_numInputElemDesc;
	const D3D_SHADER_MACRO*	m_shaderMacro;

};

class D3D11PixelShader
{
public:
	D3D11PixelShader(const WCHAR* file,const char* szEntryPoint, const D3D_SHADER_MACRO*macro=NULL):m_strEntryPoint(szEntryPoint),m_shaderMacro(macro){wcscpy_s(m_strFile, file);	}
	HRESULT Create(ID3D11Device*);
	void	Destroy();

	virtual HRESULT		OnCreat(ID3D11Device*){ return S_OK; }
	virtual void		OnDestroy(){}
	ID3D11PixelShader*	GetShader(){ return m_pPixelShader; }

private:
	WCHAR				m_strFile[255];
	std::string			m_strEntryPoint;
	ID3D11PixelShader*	m_pPixelShader;
	const D3D_SHADER_MACRO*	m_shaderMacro;
};


