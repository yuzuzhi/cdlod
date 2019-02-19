#include <d3dx9math.h>
#include <xnamath.h>
#pragma once
class DebugRender
{
public:
	DebugRender(void);
	~DebugRender(void);
	virtual void	DrawBox( const D3DXVECTOR3 & v0, const D3DXVECTOR3 & v1, unsigned int penColor, unsigned int brushColor = 0x000000, const D3DXMATRIX * transform = 0 ) = 0;
	virtual void	DrawTriangle( const D3DXVECTOR3 & v0, const D3DXVECTOR3 & v1, const D3DXVECTOR3 & v2, unsigned int penColor, unsigned int brushColor = 0x000000, const D3DXMATRIX * transform = NULL ) = 0;
	virtual void	DrawQuad( const D3DXVECTOR3 & v0, const D3DXVECTOR3 & v1, const D3DXVECTOR3 & v2, const D3DXVECTOR3 & v3, unsigned int penColor, unsigned int brushColor = 0x000000, const D3DXMATRIX * transform = NULL ) = 0;
	virtual void	RenderCamera( const XMMATRIX& view, const XMMATRIX& proj, const XMMATRIX& camViewProj, unsigned int brushColor ) = 0;
	//
};


DebugRender&GetDebugRender();
