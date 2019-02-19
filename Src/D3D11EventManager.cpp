#include "DXUT.h"
#include "D3D11EventManager.h"

D3D11EventListener::D3D11EventListener(){Dx11EventManager::Instance().Register(this);}
D3D11EventListener::~D3D11EventListener(){Dx11EventManager::Instance().UnRegister(this);}


void Dx11EventManager::Register( D3D11EventListener* obj)
{
	m_listenerList.insert(obj); 
}

void Dx11EventManager::UnRegister( D3D11EventListener* obj)
{
	m_listenerList.erase(obj); 

}

Dx11EventManager& Dx11EventManager::Instance()
{
	static Dx11EventManager Dx11EventMgr;
	return Dx11EventMgr;
}

HRESULT Dx11EventManager::OnCreateDevice( ID3D11Device*device, const DXGI_SURFACE_DESC*desc)
{
	HRESULT hr;
	ListenerContinerIter it = m_listenerList.begin();
	for ( ; it!=m_listenerList.end(); ++it )
		V_RETURN((*it)->OnCreateDevice(device, desc));

	return S_OK;
}

HRESULT Dx11EventManager::OnResizedSwapChain( ID3D11Device*device, IDXGISwapChain*chain, const DXGI_SURFACE_DESC*desc)
{
	HRESULT hr;
	ListenerContinerIter it = m_listenerList.begin();
	for ( ; it!=m_listenerList.end(); ++it )
		(*it)->OnResizedSwapChain(device, chain, desc);

	return S_OK;
}

void Dx11EventManager::OnReleasingSwapChain()
{
	ListenerContinerIter it = m_listenerList.begin();
	for ( ; it!=m_listenerList.end(); ++it )
		(*it)->OnReleasingSwapChain();
}

void Dx11EventManager::OnDestroyDevice()
{
	ListenerContinerIter it = m_listenerList.begin();
	for ( ; it!=m_listenerList.end(); ++it )
		(*it)->OnDestroyDevice();
}