#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <vector>
#include "DebugRender.h"
#include "D3D11EventManager.h"
#pragma once
#include "D3D11Shader.h"


class DrawLayerVertexShader : public D3D11VertexShader
{
public:
	struct cbLayerPos
	{
		enum { Register = 1};
		XMFLOAT4 g_layerPos;
	};
	DrawLayerVertexShader();
	virtual HRESULT		OnCreat(ID3D11Device*);
	virtual void		OnDestroy();

	ID3D11Buffer*           m_pConstantBuffer;

private:
	static const D3D11_INPUT_ELEMENT_DESC	m_inputElemDesc[];
	static const int						m_inputElemCount;

};

class D3D11DebugRender : public DebugRender, public D3D11EventListener
{

	static const D3D11_INPUT_ELEMENT_DESC	m_draw3DInputElemDesc[];
	static const int						m_draw3DInputElemCount;

	D3D11DebugRender(const D3D11DebugRender&);
	D3D11DebugRender(const DebugRender&);

public:	
	D3D11DebugRender(void);
	~D3D11DebugRender(void);

	virtual HRESULT OnCreateDevice( ID3D11Device*, const DXGI_SURFACE_DESC*);
	virtual void	OnDestroyDevice();

	virtual void	DrawBox( const D3DXVECTOR3 & min, const D3DXVECTOR3 & max, unsigned int penColor, unsigned int brushColor = 0x000000, const D3DXMATRIX * transform = NULL );
	virtual void	DrawTriangle( const D3DXVECTOR3 & v0, const D3DXVECTOR3 & v1, const D3DXVECTOR3 & v2, unsigned int penColor, unsigned int brushColor = 0x000000, const D3DXMATRIX * transform = NULL );
	virtual void	DrawQuad( const D3DXVECTOR3 & v0, const D3DXVECTOR3 & v1, const D3DXVECTOR3 & v2, const D3DXVECTOR3 & v3, unsigned int penColor, unsigned int brushColor = 0x000000, const D3DXMATRIX * transform = NULL );
	virtual void	DrawOBB(const D3DXVECTOR3&center, const D3DXVECTOR3& forward, const D3DXVECTOR3&right, const D3DXVECTOR3& up);
	//
	void				Render( const XMMATRIX& view, const XMMATRIX& proj );

	virtual void		RenderCamera( const XMMATRIX& view, const XMMATRIX& proj, const XMMATRIX& camViewProj, unsigned int brushColor=0x11FFFF00 );
	void				DrawTexture(ID3D11ShaderResourceView* pTexture, ID3D11SamplerState*,D3DXVECTOR2 tl, D3DXVECTOR2 br);

private:

	struct DrawItem	{

		enum DrawItemType	{
			Triangle,
			Box,
			OBB,
		};

		D3DXVECTOR3       v0;
		D3DXVECTOR3       v1;
		D3DXVECTOR3       v2;
		D3DXVECTOR3       v3;
		unsigned int      penColor;
		unsigned int      brushColor;
		DrawItemType   type;
		DrawItem(){}
		DrawItem( const D3DXVECTOR3 & v0, const D3DXVECTOR3 & v1, const D3DXVECTOR3 & v2, unsigned int penColor, unsigned int brushColor, DrawItemType type ) 
			: v0(v0), v1(v1), v2(v2), penColor(penColor), brushColor(brushColor), type(type) { }
	};


	ID3D11Buffer*           m_pBoxVertexBuffer;
	ID3D11Buffer*           m_pBoxIndexBuffer;
	ID3D11Buffer*           m_pBoxEdgeIndexBuffer;
	ID3D11Buffer*           m_pCameraBoxVertexBuffer;
	ID3D11Buffer*           m_pConstantBuffer;

	ID3D11Buffer*			m_pLayerVertexBuffer;
	DrawLayerVertexShader	m_layerVertexShader;
	D3D11PixelShader		m_layerPixelShader;

	D3D11VertexShader		m_draw3DVertexShader; 
	D3D11PixelShader		m_draw3DPixelShader;

	ID3D11RasterizerState*	m_pRasterizerStateWireframe[2];

	ID3D11BlendState*		m_pBlendState;


	std::vector<DrawItem>	m_drawItems;

	ID3D11BlendState*		m_pOldBlendState;
	float					m_oldBlendFactor[4];
	UINT					m_oldSampMask;

};
