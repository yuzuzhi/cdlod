#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <set>

class D3D11EventListener
{
public:
	D3D11EventListener();
	~D3D11EventListener();

private:
	friend class Dx11EventManager;
	virtual HRESULT OnCreateDevice( ID3D11Device*, const DXGI_SURFACE_DESC*){ return S_OK;}
	virtual HRESULT OnResizedSwapChain( ID3D11Device*, IDXGISwapChain*,const DXGI_SURFACE_DESC*){ return S_OK; }
	virtual void	OnReleasingSwapChain(){}
	virtual void	OnDestroyDevice(){}

};

class Dx11EventManager
{
	typedef std::set<D3D11EventListener*> ListenerContiner;
	typedef ListenerContiner::iterator ListenerContinerIter;

	Dx11EventManager(){}
	Dx11EventManager(const Dx11EventManager&);
public:

	static Dx11EventManager& Instance();

	void	Register(D3D11EventListener*);
	void	UnRegister(D3D11EventListener*);

	HRESULT OnCreateDevice( ID3D11Device*, const DXGI_SURFACE_DESC*);
	HRESULT OnResizedSwapChain( ID3D11Device*, IDXGISwapChain*,const DXGI_SURFACE_DESC*);
	void	OnReleasingSwapChain();
	void	OnDestroyDevice();

private:

	ListenerContiner m_listenerList;
};