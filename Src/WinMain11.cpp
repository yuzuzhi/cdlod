//--------------------------------------------------------------------------------------
// File: BasicHLSL11.cpp
//
// This sample shows a simple example of the Microsoft Direct3D's High-Level 
// Shader Language (HLSL) using the Effect interface. 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "resource.h"
#include "D3D11DebugRender.h"
#include "CDLODTerrainRender.h"
#include "D3D11HelperRender.h"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CBaseCamera*				g_pActiveCamera;
CFirstPersonCamera g_Camera;
CDXUTDirectionWidget        g_LightControl;
CD3DSettingsDlg             g_D3DSettingsDlg;       // Device settings dialog
CDXUTDialog                 g_HUD;                  // manages the 3D   
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls
D3DXMATRIXA16               g_mCenterMesh;
float                       g_fLightScale;
int                         g_nNumActiveLights;
int                         g_nActiveLight;
bool                        g_bShowHelp = false;    // If true, it renders the UI control text
bool						g_bCtrlDown = false;

// Direct3D9 resources
CDXUTTextHelper*            g_pTxtHelper = NULL;


ID3D11ShaderResourceView*g_pTerrainHeightmapTexture = NULL;

CDLODTerrainRender		g_quadtreeRender;


struct CameraInfo
{
	D3DXVECTOR3 InitEyePos;
	D3DXVECTOR3 InitLookAt;
};

CameraInfo CAMERA_INFO[3] = 
{
	{D3DXVECTOR3(0.0f, 500.0f,-1000.0f),D3DXVECTOR3(0,0,0)},
	{D3DXVECTOR3(0.0f, 500.0f,-1000.0f),D3DXVECTOR3(0,0,0)},
	{D3DXVECTOR3(10.f, 10.f,10.f),		D3DXVECTOR3(0,0,0)},
};

int g_cameraIndex = 0;
CFirstPersonCamera g_cameras[2];
const int CAMERA_COUNT = sizeof(g_cameras) / sizeof(CFirstPersonCamera);


struct CB_VS_PER_OBJECT
{
    D3DXMATRIX m_WorldViewProj;
    D3DXMATRIX m_World;
};
UINT                        g_iCBVSPerObjectBind = 0;

struct CB_PS_PER_OBJECT
{
    D3DXVECTOR4 m_vObjectColor;
};
UINT                        g_iCBPSPerObjectBind = 0;

struct CB_PS_PER_FRAME
{
    D3DXVECTOR4 m_vLightDirAmbient;
};
UINT                        g_iCBPSPerFrameBind = 1;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

extern bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                             bool bWindowed, void* pUserContext );
extern HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice,
                                            const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
extern HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                           void* pUserContext );
extern void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime,
                                        void* pUserContext );
extern void CALLBACK OnD3D9LostDevice( void* pUserContext );
extern void CALLBACK OnD3D9DestroyDevice( void* pUserContext );

bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                  float fElapsedTime, void* pUserContext );

void InitApp();
void RenderText();



//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D11) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );


    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );


    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"CDLODTerrain" );
    DXUTCreateDevice (D3D_FEATURE_LEVEL_9_2, true, 800, 600 );
    //DXUTCreateDevice(true, 640, 480);
    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    D3DXVECTOR3 vLightDir( -1, 1, -1 );
    D3DXVec3Normalize( &vLightDir, &vLightDir );
    g_LightControl.SetLightDirection( vLightDir );

    // Initialize dialogs
    g_D3DSettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 23 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += 26, 170, 23, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += 26, 170, 23, VK_F2 );

    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;


	for (int i=0; i<sizeof(g_cameras)/sizeof(CFirstPersonCamera); ++i)
		g_cameras[i].SetScalers(0.01f,500);

	g_pActiveCamera = &g_cameras[g_cameraIndex];

}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    // Uncomment this to get debug information from D3D11
    //pDeviceSettings->d3d11.CreateFlags |= D3D11_CREATE_DEVICE_DEBUG;

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( ( DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
              pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE ) )
        {
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
        }
    }

    return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
	//g_Camera.FrameMove( fElapsedTime );
	//g_Camera1.FrameMove( fElapsedTime );
	if (g_pActiveCamera)
		g_pActiveCamera->FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
    UINT nBackBufferHeight = ( DXUTIsAppRenderingWithD3D9() ) ? DXUTGetD3D9BackBufferSurfaceDesc()->Height :
            DXUTGetDXGIBackBufferSurfaceDesc()->Height;

    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 2, 0 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

    // Draw help
    if( g_bShowHelp )
    {
        g_pTxtHelper->SetInsertionPos( 2, nBackBufferHeight - 20 * 6 );
        g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        g_pTxtHelper->DrawTextLine( L"Controls:" );

        g_pTxtHelper->SetInsertionPos( 20, nBackBufferHeight - 20 * 5 );
        g_pTxtHelper->DrawTextLine( L"Rotate model: Left mouse button\n"
                                    L"Rotate light: Right mouse button\n"
                                    L"Rotate camera: Middle mouse button\n"
                                    L"Zoom camera: Mouse wheel scroll\n" );

        g_pTxtHelper->SetInsertionPos( 550, nBackBufferHeight - 20 * 5 );
        g_pTxtHelper->DrawTextLine( L"Hide help: F1\n"
                                    L"Quit: ESC\n" );
    }
    else
    {
        g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        g_pTxtHelper->DrawTextLine( L"Press F1 for help" );
    }

    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    g_LightControl.HandleMessages( hWnd, uMsg, wParam, lParam );

    // Pass all remaining windows messages to camera so it can respond to user input
	//if(!g_bCtrlDown)
	//	g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
	//else
	//	g_Camera1.HandleMessages( hWnd, uMsg, wParam, lParam );
	if (g_pActiveCamera)
		g_pActiveCamera->HandleMessages( hWnd, uMsg, wParam, lParam );
    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1:
                g_bShowHelp = !g_bShowHelp; break;
			case 84:		//T
				g_cameraIndex++;
				g_cameraIndex %= CAMERA_COUNT;
				g_pActiveCamera = &g_cameras[g_cameraIndex];
				break;
			case 82:		//R
				{
					TerrainRendFillMode::Type fillMode = g_quadtreeRender.GetRendFillMode()!=TerrainRendFillMode::WireFrame?
						TerrainRendFillMode::WireFrame:TerrainRendFillMode::Solid;
					g_quadtreeRender.SetRendFillMode(fillMode);

				}
				break;
			case 85:
				g_quadtreeRender.SetDebugDraw(!g_quadtreeRender.GetDebugDraw());
				break;
        }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:
            g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() ); break;
    }

}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, const D3D_SHADER_MACRO* pDefines = NULL )
{
	HRESULT hr = S_OK;

	// find the file
	WCHAR str[MAX_PATH];
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, szFileName ) );

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// compile the shader
	ID3DBlob* pErrorBlob = NULL;
	hr = D3DX11CompileFromFile( str, pDefines, NULL, szEntryPoint, szShaderModel, dwShaderFlags, NULL, NULL, ppBlobOut, &pErrorBlob, NULL );
	if( FAILED(hr) )
	{
		if( pErrorBlob != NULL )
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
		SAFE_RELEASE( pErrorBlob );
		return hr;
	}
	SAFE_RELEASE( pErrorBlob );

	return S_OK;   
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
    HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );




    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye( -500, 500.0f,-1000.0f);
    D3DXVECTOR3 vecAt ( 0.0f,  0.0f,  0.0f );

	D3DXVECTOR3 vecEye1( 0.0f, 500.0f,-1000.0f);
	for (int i=0; i<sizeof(g_cameras)/sizeof(CFirstPersonCamera); ++i)
	{
		g_cameras[i].SetViewParams( &CAMERA_INFO[i].InitEyePos, &CAMERA_INFO[i].InitLookAt);
		g_cameras[i].SetRotateButtons( true, false, false );
		g_cameras[i].SetEnablePositionMovement( true );
	}

	V_RETURN(Dx11EventManager::Instance().OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	for (int i = 0; i < sizeof(g_cameras) / sizeof(CFirstPersonCamera); ++i)
	{
		float farPlane = (i == 0 ? 5000 : 40000);
		g_cameras[i].SetProjParams(D3DX_PI / 4, fAspectRatio, 1.0f, farPlane);

	}
    //g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    //g_Camera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300 );
    g_SampleUI.SetSize( 170, 300 );

	V_RETURN(Dx11EventManager::Instance().OnResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

    return S_OK;
}
//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                  float fElapsedTime, void* pUserContext )
{
    //HRESULT hr;

    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.OnRender( fElapsedTime );
        return;
    }

    // Clear the render target and depth stencil
    float ClearColor[4] = { 0, 0, 0, 0.55f };
    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	//RendRole(pd3dImmediateContext);


	g_quadtreeRender.Update(g_cameras[0]);
	g_quadtreeRender.Render(*g_pActiveCamera);

	XMMATRIX viewMtrix1 = XMLoadFloat4x4((const XMFLOAT4X4*)g_pActiveCamera->GetViewMatrix());
	XMMATRIX projMtrix1 = XMLoadFloat4x4((const XMFLOAT4X4*)g_pActiveCamera->GetProjMatrix());
	D3D11DebugRender& debugRender = (D3D11DebugRender&)GetDebugRender();
	debugRender.Render(viewMtrix1,projMtrix1);
	GetHelpRender()->DrawAxes(pd3dImmediateContext);

    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    g_HUD.OnRender( fElapsedTime );
    g_SampleUI.OnRender( fElapsedTime );
    RenderText();

    DXUT_EndPerfEvent();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();

	Dx11EventManager::Instance().OnReleasingSwapChain();

}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_D3DSettingsDlg.OnD3D11DestroyDevice();
    //CDXUTDirectionWidget::StaticOnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE( g_pTxtHelper );



	Dx11EventManager::Instance().OnDestroyDevice();

}



