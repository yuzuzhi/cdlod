#include "DXUT.h"
#include "CDLODTerrainRender.h"
#include "TiledBitmap.h"
#include "DebugRender.h"
#include "common.h"
#include "D3D11DebugRender.h"
#include "D3D11HelperRender.h"

//地形纹理
const char* GROUND_TEXTURES[] = {
	"Media\\dirt001.dds",
	"Media\\dirt002.dds",
	"Media\\dirt004.dds",
	"Media\\stone001.dds",
	"Media\\normal001.dds",
	"Media\\normal002.dds",
};


static const XMFLOAT4 FRUSTUM_CORD[] =
{
	XMFLOAT4( -1, -1, 0, 1 ),
	XMFLOAT4( -1, 1, 0, 1 ),
	XMFLOAT4( 1, -1, 0, 1 ),
	XMFLOAT4( 1, 1, 0, 1 ),
	XMFLOAT4( -1, -1, 1, 1) ,
	XMFLOAT4( -1, 1, 1, 1 ),
	XMFLOAT4( 1, -1, 1, 1 ),
	XMFLOAT4( 1, 1, 1, 1 ),
};

static void CreateNormalMap( int sizeX, int sizeY, const MapDimensions& mapDims, float * heightmapData, int heightmapDataPitch, XMFLOAT3* normalmapData, int normalmapDataPitch, bool wrapEdges );

class SimpleHeightmapSource : public IHeightmapSource
{
private:
	float *                    m_pData;
	int                        m_pitch;    // in bytes

	int                        m_width;
	int                        m_height;

public:
	SimpleHeightmapSource(int width, int height)
	{
		m_pData = new float[width*height];
		m_pitch = sizeof(*m_pData) * width;
		m_width = width;
		m_height = height;
	}

	~SimpleHeightmapSource()
	{
		if(m_pData) 
			delete m_pData;
	}

	const float*			GetData() const { return m_pData; }
	int						GetPitch() const { return m_pitch; }
	virtual int             GetSizeX( )    { return m_width; }
	virtual int             GetSizeY( )    { return m_height; }
	void					SetHeightAt(int x, int y, unsigned short height)
	{
		m_pData[ m_width*y + x ] = (float)height / 65535.0f;
	}

	virtual unsigned short  GetHeightAt( int x, int y )
	{
		return (unsigned short)(m_pData[ m_width*y + x ] * 65535.0f + 0.5f);
	}

	virtual void            GetAreaMinMaxH( int x, int y, int sizeX, int sizeY, unsigned short & minH, unsigned short & maxH )
	{
		assert( x >= 0 && y >= 0 && (x+sizeX) <= m_width && (y+sizeY) <= m_height );
		minH = 65535;
		maxH = 0;

		for( int ry = 0; ry < sizeY; ry++ )
		{
			//if( (ry + y) >= rasterSizeY )
			//   break;
			float * scanLine = &m_pData[ x + (ry+y) * (m_pitch/sizeof(*m_pData)) ];
			//int sizeX = ::min( rasterSizeX - x, size );
			for( int rx = 0; rx < sizeX; rx++ )
			{
				unsigned short h = (unsigned short)(scanLine[rx] * 65535.0f + 0.5f);
				minH = min( minH, h );
				maxH = max( maxH, h );
			}
		}

	}
};

static unsigned int LEVEL_COLOR[] = 
{
	0x22000000,	//black
	0x220000FF,	//blue
	0x2200FF00,	//green
	0x2200FFFF,
	0x22FF0000,	//red
	0x22FF00FF,
	0x22FFFF00,
	0x22FFFFFF,
};

HRESULT D3D11CDLODTerrainGridMesh::Create(ID3D11Device*pd3dDevice)
{
	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( Vertex ) * GetVertexCount();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = GetVertexData();
	hr = pd3dDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );
	if( FAILED( hr ) ) 
		return hr;



	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(unsigned short)*GetIndicesCount();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = GetIndicesData();
	hr = pd3dDevice->CreateBuffer( &bd, &InitData, &m_pIndexBuffer );
	if( FAILED( hr ) ) 
		return false;


	return S_OK;
}

void D3D11CDLODTerrainGridMesh::Destroy()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
}


D3D10_SHADER_MACRO Shader_Defines[2] = { "BLUR_V", "1", NULL,0  };

CDLODTerrainRender::CDLODTerrainRender(void):
	m_bDebugDraw(false),
	m_genVsmPS(L"CDLODTerrainPS.hlsl","GenTerrainVSMPS"),
	m_blurHPixelShader(L"Blur.hlsl","Blur"),
	m_blurVPixelShader(L"Blur.hlsl","Blur",&Shader_Defines[0])
{
	m_shadowmapWidth = 4096;
	m_shadowmapHeight = 4096;
}

CDLODTerrainRender::~CDLODTerrainRender(void)
{
	if(m_heightmap) delete m_heightmap;
	if(m_heightmapSrc) delete m_heightmapSrc;
}

HRESULT CDLODTerrainRender::OnCreateDevice( ID3D11Device*pd3dDevice, const DXGI_SURFACE_DESC* surface_desc)
{
	HRESULT hr;

	const char* path = "Media\\maintestdata\\";
	std::string strFile = path; strFile+="maintestdata.ini";
	char szBuff[512];
	GetPrivateProfileStringA("main","HeightmapPath","",szBuff,sizeof(szBuff),strFile.c_str());
	std::string strHeightMapFile = path; strHeightMapFile+=szBuff;

	CDLODTerrainQuadTree::CreateDesc desc;
	desc.LeafRenderNodeSize = 8;
	desc.LODLevelCount = 5;
	desc.MapDims.MinX = (GetPrivateProfileStringA("main","MapDims_MinX","",szBuff,sizeof(szBuff),strFile.c_str()),atof(szBuff));
	desc.MapDims.MinY = (GetPrivateProfileStringA("main","MapDims_MinY","",szBuff,sizeof(szBuff),strFile.c_str()),atof(szBuff));
	desc.MapDims.MinH = (GetPrivateProfileStringA("main","MapDims_MinZ","",szBuff,sizeof(szBuff),strFile.c_str()),atof(szBuff));
	desc.MapDims.SizeX= (GetPrivateProfileStringA("main","MapDims_SizeX","",szBuff,sizeof(szBuff),strFile.c_str()),atof(szBuff));
	desc.MapDims.SizeY= (GetPrivateProfileStringA("main","MapDims_SizeY","",szBuff,sizeof(szBuff),strFile.c_str()),atof(szBuff));
	desc.MapDims.SizeH= (GetPrivateProfileStringA("main","MapDims_SizeZ","",szBuff,sizeof(szBuff),strFile.c_str()),atof(szBuff));

	//读取并读取高度数据
	m_heightmap = VertexAsylum::TiledBitmap::Open( strHeightMapFile.c_str(), true );
	int m_rasterWidth   = m_heightmap->Width();
	int m_rasterHeight  = m_heightmap->Height();
	m_heightmapSrc = new SimpleHeightmapSource(m_rasterWidth, m_rasterHeight);

	for( int y = 0; y < m_rasterHeight; y++ )
		for( int x = 0; x < m_rasterWidth; x++ )
		{
			unsigned short pixel;
			m_heightmap->GetPixel( x, y, &pixel );
			m_heightmapSrc->SetHeightAt(x,y,pixel);
		}	
	//创建地型四叉树
	desc.pHeightmap = m_heightmapSrc;
	m_quadTree.Create(desc);


	//
	V_RETURN(m_gridMesh.Create(pd3dDevice));
	V_RETURN(m_VertexShader.Create(pd3dDevice));
	V_RETURN(m_PixelShader.Create(pd3dDevice));

	//
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_SUBRESOURCE_DATA  textureInitData;
	memset(&textureDesc, 0, sizeof(D3D11_TEXTURE2D_DESC));
	memset(&textureInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
	textureDesc.Width				= m_rasterWidth;
	textureDesc.Height				= m_rasterHeight;
	textureDesc.MipLevels			= 1;
	textureDesc.ArraySize			= 1;
	textureDesc.Format				= DXGI_FORMAT_R32_FLOAT;
	textureDesc.SampleDesc.Count	= 1;
	//textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage				= D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags		= 0;
	textureDesc.MiscFlags			= 0;
	textureInitData.pSysMem			= m_heightmapSrc->GetData();
	textureInitData.SysMemPitch		= m_heightmapSrc->GetPitch();
	V_RETURN(pd3dDevice->CreateTexture2D(&textureDesc,&textureInitData, &m_HeightMapTexture));

	XMFLOAT3* normalMapData = new XMFLOAT3[textureDesc.Width*textureDesc.Height];
	textureDesc.Format				= DXGI_FORMAT_R32G32B32_FLOAT;
	textureInitData.pSysMem			= normalMapData;
	textureInitData.SysMemPitch		= textureDesc.Width*sizeof(XMFLOAT3);
	const MapDimensions& mapdim		= m_quadTree.GetWorldMapDims();
	CreateNormalMap( m_rasterWidth, m_rasterHeight, mapdim, (float *)m_heightmapSrc->GetData(), m_heightmapSrc->GetPitch(), (XMFLOAT3*)textureInitData.pSysMem, textureInitData.SysMemPitch, false );
	V_RETURN(pd3dDevice->CreateTexture2D(&textureDesc,&textureInitData, &m_NormalMapTexture));

	CD3D11_SHADER_RESOURCE_VIEW_DESC heightmap_dsrvd(m_HeightMapTexture,D3D11_SRV_DIMENSION_TEXTURE2D);
	V_RETURN(pd3dDevice->CreateShaderResourceView(m_HeightMapTexture,&heightmap_dsrvd, &m_HeightMapShaderResView));

	CD3D11_SHADER_RESOURCE_VIEW_DESC normalmap_dsrvd(m_NormalMapTexture,D3D11_SRV_DIMENSION_TEXTURE2D);
	V_RETURN(pd3dDevice->CreateShaderResourceView(m_NormalMapTexture,&normalmap_dsrvd, &m_NormalMapShaderResView));

	for (int i=0; i<sizeof(GROUND_TEXTURES)/sizeof(GROUND_TEXTURES[0]);++i)
	{
		m_groundTextures.mFileName[i] = GROUND_TEXTURES[i];
		V_RETURN(D3DX11CreateShaderResourceViewFromFileA(pd3dDevice, m_groundTextures.mFileName[i].c_str(), NULL, NULL, &m_groundTextures.mTexture[i], NULL));
	}


	D3D11_SAMPLER_DESC SSDesc;
	ZeroMemory( &SSDesc, sizeof( D3D11_SAMPLER_DESC ) );
	SSDesc.Filter =         D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SSDesc.AddressU =       D3D11_TEXTURE_ADDRESS_CLAMP;
	SSDesc.AddressV =       D3D11_TEXTURE_ADDRESS_CLAMP;
	SSDesc.AddressW =       D3D11_TEXTURE_ADDRESS_CLAMP;
	SSDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SSDesc.MaxAnisotropy =  16;
	SSDesc.MinLOD =         0;
	SSDesc.MaxLOD =         D3D11_FLOAT32_MAX;
	V_RETURN( pd3dDevice->CreateSamplerState( &SSDesc, &m_pTerrainHMapSS) );

	SSDesc.AddressU =       D3D11_TEXTURE_ADDRESS_WRAP;
	SSDesc.AddressV =       D3D11_TEXTURE_ADDRESS_WRAP;
	SSDesc.AddressW =       D3D11_TEXTURE_ADDRESS_WRAP;
	V_RETURN( pd3dDevice->CreateSamplerState( &SSDesc, &m_pGroundTextureSS) );

	//
	D3D11_RASTERIZER_DESC RasterDesc;
	ZeroMemory( &RasterDesc, sizeof( D3D11_RASTERIZER_DESC ) );
	RasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	RasterDesc.CullMode = D3D11_CULL_NONE;
	RasterDesc.DepthClipEnable = TRUE;
	V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &m_pRasterizerState[0] ) );
	RasterDesc.FillMode = D3D11_FILL_SOLID;
	V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &m_pRasterizerState[1] ) );

	//
	ID3D11DeviceContext*pImmediateContext = DXUTGetD3D11DeviceContext();
	const MapDimensions& mapDims = m_quadTree.GetWorldMapDims();
	CDLODTerrainVShader::cbTerrainInfo terrainInfo;
	terrainInfo.SetOffset(mapDims.MinX, mapDims.MinH, mapDims.MinY);
	terrainInfo.SetScale(mapDims.SizeX, mapDims.SizeH, mapDims.SizeY);
	terrainInfo.SetHeightMapInfo(m_quadTree.GetRasterSizeX(), m_quadTree.GetRasterSizeY());
	terrainInfo.SetGridMeshDim(m_gridMesh.GetDimentions());
	m_VertexShader.UpdateSubresource(pImmediateContext, terrainInfo);

	//
	D3D11_TEXTURE2D_DESC shadowTDesc = {
		m_shadowmapWidth,//UINT Width;
		m_shadowmapHeight,//UINT Height;
		1,//UINT MipLevels;
		m_quadTree.GetLODLevelCount(),//UINT ArraySize;
		m_ShadowBufferFormat,//DXGI_FORMAT Format;
		1,//DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};

	V_RETURN( pd3dDevice->CreateTexture2D( &shadowTDesc, NULL, &m_pCascadedShadowMapVarianceTextureArray  ) );
	shadowTDesc.ArraySize = 1;
	V_RETURN( pd3dDevice->CreateTexture2D( &shadowTDesc, NULL, &m_pCascadedShadowMapTempBlurTexture  ) );

	D3D11_RENDER_TARGET_VIEW_DESC drtvd = {
		m_ShadowBufferFormat,
		D3D11_RTV_DIMENSION_TEXTURE2D
	};
	drtvd.Texture2D.MipSlice = 0;
	V_RETURN( pd3dDevice->CreateRenderTargetView ( m_pCascadedShadowMapTempBlurTexture, &drtvd, &m_pCascadedShadowMapTempBlurRTV) );

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = {
		m_ShadowBufferFormat,
		D3D11_SRV_DIMENSION_TEXTURE2D,
	};
	dsrvd.Texture2D.MipLevels = 1;
	dsrvd.Texture2D.MostDetailedMip = 0;
	V_RETURN( pd3dDevice->CreateShaderResourceView( m_pCascadedShadowMapTempBlurTexture, &dsrvd, &m_pCascadedShadowMapTempBlurSRV ) );


	for (int i = 0; i < m_quadTree.GetLODLevelCount(); ++i)
		m_terrainShadow[i].OnCreateDevice(pd3dDevice, surface_desc, m_pCascadedShadowMapVarianceTextureArray, i);


	m_genVsmPS.Create(pd3dDevice);
	m_blurHPixelShader.Create(pd3dDevice);
	m_blurVPixelShader.Create(pd3dDevice);

	return S_OK;
}
void CDLODTerrainRender::OnDestroyDevice()
{
	m_gridMesh.Destroy();
	m_VertexShader.Destroy();
	m_PixelShader.Destroy();
	m_genVsmPS.Destroy();
	SAFE_RELEASE(m_pRasterizerState[0]);
	SAFE_RELEASE(m_pRasterizerState[1]);
	SAFE_RELEASE(m_HeightMapTexture);
	SAFE_RELEASE(m_HeightMapShaderResView);
	SAFE_RELEASE(m_pTerrainHMapSS);
	SAFE_RELEASE(m_pGroundTextureSS);
	SAFE_RELEASE(m_TestShaderResView);
	SAFE_RELEASE(m_NormalMapShaderResView);
	SAFE_RELEASE(m_NormalMapTexture);
	SAFE_RELEASE(m_pCascadedShadowMapVarianceTextureArray);
	SAFE_RELEASE(m_pCascadedShadowMapTempBlurTexture);
	SAFE_RELEASE(m_pCascadedShadowMapTempBlurRTV);
	SAFE_RELEASE(m_pCascadedShadowMapTempBlurSRV);
	
	for (int i=0; i<m_groundTextures.nTextureCount;++i)
		SAFE_RELEASE(m_groundTextures.mTexture[i]);


	for (int i = 0; i < CDLODTerrainQuadTree::MAX_LOD_LEVELS; ++i)
		m_terrainShadow[i].OnDestroyDevice();


	m_blurHPixelShader.Destroy();
	m_blurVPixelShader.Destroy();
}

void CDLODTerrainRender::Update( const CBaseCamera& camera )
{
	m_selector.Clear();
	D3DXMatrixMultiply(&m_selector.mViewProj,camera.GetViewMatrix(),camera.GetProjMatrix());
	m_selector.mObservPos = *camera.GetEyePt();
	m_quadTree.Select(m_selector);



	Light& light = m_lightInfo;
	light.mUp = D3DXVECTOR3(0, 0, 1);
	light.mForward = D3DXVECTOR3(-1, -0.5f, 0);
	D3DXVec3Normalize(&light.mForward, &light.mForward);
	D3DXVec3Cross(&light.mRight, &light.mUp, &light.mForward);
	D3DXVec3Normalize(&light.mRight, &light.mRight);
	D3DXVec3Cross(&light.mUp, &light.mForward, &light.mRight);
	D3DXVec3Normalize(&light.mUp, &light.mUp);
	for (int i = m_selector.mMinSelectLevel; i <= m_selector.mMaxSelectLevel; ++i)
	{
		float fRange = m_selector.mVisibilityRanges[i];
		D3DXVECTOR3 halfSize(fRange, fRange, fRange);
		AABB box;
		box.Min = m_selector.mObservPos - halfSize;
		box.Max = m_selector.mObservPos + halfSize;
		m_quadTree.GetAreaMinMaxHeight(box.Min.x, box.Min.y, box.Max.x - box.Min.x, box.Max.z - box.Min.z, box.Min.y, box.Max.y);
		ShadowFitArea fitArea;
		fitArea.mMin = box.Min;
		fitArea.mMax = box.Max;
		//m_terrainShadow[i].Update(light, fitArea);
		shadowfitCamera fitcam;
		fitcam.mCamView = *camera.GetViewMatrix();
		fitcam.mCamProj = *camera.GetProjMatrix();
		fitcam.mCamPos	= *camera.GetEyePt();
		fitcam.mLevelRange = m_selector.mVisibilityRanges[i];
		m_terrainShadow[i].Update(light, fitcam);
	}
}
HRESULT CDLODTerrainRender::Render( const CBaseCamera& camera)
{
	D3D11DebugRender& debugRender = (D3D11DebugRender&)GetDebugRender();
	//
	ID3D11DeviceContext*pImmediateContext		= DXUTGetD3D11DeviceContext();
	ID3D11RenderTargetView* pBackBuffer			= DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pBackDepthStencil	= DXUTGetD3D11DepthStencilView();
	D3D11_VIEWPORT backViewPort;
	backViewPort.TopLeftX = 0;backViewPort.TopLeftY = 0;
	backViewPort.Width = (FLOAT)DXUTGetDXGIBackBufferSurfaceDesc()->Width;
	backViewPort.Height = (FLOAT)DXUTGetDXGIBackBufferSurfaceDesc()->Height;
	backViewPort.MinDepth = 0;backViewPort.MaxDepth = 1;


	static int index = m_selector.mMinSelectLevel;
	static bool prePressed = GetAsyncKeyState('C') < 0;
	if (prePressed==false && GetAsyncKeyState('C') < 0)
	{
		index++;
		if (index > m_selector.mMaxSelectLevel)
			index = m_selector.mMinSelectLevel;
	}
	prePressed = GetAsyncKeyState('C') < 0;


	//for (int i = m_selector.mMinSelectLevel; i <= m_selector.mMaxSelectLevel; ++i)
		//GetDebugRender().DrawBox(m_terrainShadow[index].mMin, m_terrainShadow[index].mMax, 0x11FFFFFF);



	//高度纹理寻址
	pImmediateContext->VSSetSamplers(0, 1, &m_pTerrainHMapSS);
	pImmediateContext->VSSetShaderResources(0, 1, &m_HeightMapShaderResView);
	pImmediateContext->VSSetConstantBuffers(0, m_VertexShader.CONST_BUFFER_COUNT, &m_VertexShader.m_pConstBuffers[0]);

	//设置定点着色器
	pImmediateContext->VSSetShader(m_VertexShader.GetShader(), NULL, 0);
	pImmediateContext->PSSetShader(m_genVsmPS.GetShader(), NULL, 0);	
	RendTerrainDepthMap(pImmediateContext);
	pImmediateContext->RSSetViewports(1, &backViewPort);
	pImmediateContext->OMSetRenderTargets(1, &pBackBuffer, pBackDepthStencil);

	//
	pImmediateContext->PSSetShader( m_PixelShader.GetShader(), NULL, 0 );
	pImmediateContext->PSSetSamplers(1, 1, &m_pGroundTextureSS);

	//设置纹理
	pImmediateContext->PSSetShaderResources(1, 1, &m_NormalMapShaderResView);
	pImmediateContext->PSSetShaderResources(4, m_groundTextures.nTextureCount, &m_groundTextures.mTexture[0]);
	
	pImmediateContext->RSSetState(m_pRasterizerState[m_rendRasterType]);

	// 转换
	CDLODTerrainVShader::cbPerFrame cb1;
	D3DXMATRIX viewProj = (*camera.GetViewMatrix()) * (*camera.GetProjMatrix());
	cb1.g_viewProjection = XMMatrixTranspose(XMLoadFloat4x4((const XMFLOAT4X4*)&viewProj));//
	cb1.g_cameraPos = XMVectorSet(m_selector.mObservPos.x, m_selector.mObservPos.y, m_selector.mObservPos.z, 1.0f);
	const D3DXVECTOR3& lightDir = m_lightInfo.mForward;
	cb1.g_diffuseLightDir = XMVectorSet(lightDir.x, lightDir.y, lightDir.z, 1);
	m_VertexShader.UpdateSubresource(pImmediateContext, cb1);

#if 0
	CDLODTerrainVShader::cbDebugInfo debugInfo;
	debugInfo.g_debugVars.x = 0.f;
	if (::GetAsyncKeyState('C')<0)
	{
		debugInfo.g_debugVars.x = 1.f;
	}
	m_VertexShader.UpdateSubresource(pImmediateContext, debugInfo);
#endif

	//设置定点着色器
	pImmediateContext->VSSetShader(m_VertexShader.GetShader(), NULL, 0);
	pImmediateContext->PSSetShader(m_genVsmPS.GetShader(), NULL, 0);
	//光照信息
	CDLODTerrainPShader::cbLightInfo lightInfo;
	lightInfo.g_lightColorDiffuse = XMVectorSet(0.65f,0.65f,0.65f,1.0f);
	lightInfo.g_lightColorAmbient = XMVectorSet(0.05f, 0.05f, 0.05f, 1.0f);
	lightInfo.g_fogColor = XMVectorSet(0.0f, 0.5f, 0.5f, 1.0f);
	pImmediateContext->UpdateSubresource( m_PixelShader.m_pLightInfo, 0, NULL, &lightInfo, 0, 0 );
	pImmediateContext->PSSetConstantBuffers( lightInfo.Register, 1, &m_PixelShader.m_pLightInfo );
	DrawSelector(pImmediateContext, false, m_selector);

	if(m_bDebugDraw)
	{
		const float w(0.6),h(0.8);
		D3DXVECTOR2 l(0.f,0.f),r(w,h);
		if (::GetAsyncKeyState('N')<0)
		{
			debugRender.DrawTexture(m_terrainShadow[index].GetDepthMapShaderResView(), m_pGroundTextureSS, l, r);
		}
		else
		{
			debugRender.DrawTexture(m_pCascadedShadowMapTempBlurSRV, m_pGroundTextureSS,  l, r);
		}
		//debugRender.DrawTexture(m_terrainShadow[index].GetRendTarShaderResView(), m_pGroundTextureSS, (l.y+=h,l), (r.y+=h,r));
		
		//DebugDrawSelector(m_selector);

		//for (int i = m_selector.mMinSelectLevel; i <= m_selector.mMaxSelectLevel; ++i)
			debugRender.RenderCamera(
			XMLoadFloat4x4((const XMFLOAT4X4*)camera.GetViewMatrix()),
			XMLoadFloat4x4((const XMFLOAT4X4*)camera.GetProjMatrix()),
			m_terrainShadow[index].GetViewProj());

		debugRender.RenderCamera(
			XMLoadFloat4x4((const XMFLOAT4X4*)camera.GetViewMatrix()), 
			XMLoadFloat4x4((const XMFLOAT4X4*)camera.GetProjMatrix()),
			XMLoadFloat4x4((const XMFLOAT4X4*)&m_selector.mViewProj));
	}

	return S_OK;
}


void CDLODTerrainRender::DebugDrawSelector( const CDLODTerrainQuadTree::NodeSlector& selected )
{
	for (int i = selected.mMaxSelectLevel; i >= selected.mMinSelectLevel;--i)
	{
		CDLODTerrainQuadTree::NodeSlector::NodeContinerConstIter it = selected.mSelected[i].begin();
		while (it != selected.mSelected[i].end())
		{
			const CDLODTerrainQuadTree::SelectedNode & nodeSel = (*it);
			int lodLevel = nodeSel.LODLevel;
			bool drawFull = nodeSel.TL && nodeSel.TR && nodeSel.BL && nodeSel.BR;
			unsigned int color = LEVEL_COLOR[lodLevel];

			AABB boundingBox;
			m_quadTree.GetAABB(nodeSel, boundingBox);
			boundingBox.Expand(-0.001f);
			if (drawFull)
				GetDebugRender().DrawBox(boundingBox.Min, boundingBox.Max, color);
			else
			{
				float midX = boundingBox.Center().x;
				float midY = boundingBox.Center().y;

				if (nodeSel.TL)
				{
					AABB bbSub = boundingBox; bbSub.Max.x = midX; bbSub.Max.y = midY;
					bbSub.Expand(-0.002f);
					GetDebugRender().DrawBox(bbSub.Min, bbSub.Max, color);
				}
				if (nodeSel.TR)
				{
					AABB bbSub = boundingBox; bbSub.Min.x = midX; bbSub.Max.y = midY;
					bbSub.Expand(-0.002f);
					GetDebugRender().DrawBox(bbSub.Min, bbSub.Max, color);
				}
				if (nodeSel.BL)
				{
					AABB bbSub = boundingBox; bbSub.Max.x = midX; bbSub.Min.y = midY;
					bbSub.Expand(-0.002f);
					GetDebugRender().DrawBox(bbSub.Min, bbSub.Max, color);
				}
				if (nodeSel.BR)
				{
					AABB bbSub = boundingBox; bbSub.Min.x = midX; bbSub.Min.y = midY;
					bbSub.Expand(-0.002f);
					GetDebugRender().DrawBox(bbSub.Min, bbSub.Max, color);
				}
			}


			++it;
		}
	}
}

void CDLODTerrainRender::DrawSelector( ID3D11DeviceContext* pImmediateContext, bool drawDepth, const CDLODTerrainQuadTree::NodeSlector& selector)
{
	//设置定点输入
	UINT stride = sizeof(XMFLOAT3);	UINT offset = 0;
	ID3D11Buffer*pVertexBuffer = m_gridMesh.GetVertexBuffer();
	pImmediateContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	pImmediateContext->IASetIndexBuffer(m_gridMesh.GetIndexBuffer(), m_gridMesh.GetIndexFormat(), 0);
	pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pImmediateContext->IASetInputLayout(m_VertexShader.GetInputLayout());
	
	ID3D11SamplerState* sampler[1];
	sampler[0]= GetHelpRender()->linearSampler;
	pImmediateContext->PSSetSamplers(0, 1, sampler);
	AABB boundingBox;
	for (int i = selector.mMaxSelectLevel; i >= selector.mMinSelectLevel; --i)
	{
		const int nIndicesCount = m_gridMesh.GetIndicesCount();
		CDLODTerrainVShader::cbLodLevelInfo levelInfo;
		float fMorphStart, fMorphEnd;
		selector.GetLodLevlMorphDist(i, fMorphStart, fMorphEnd);
		levelInfo.SetLodLevlMorph(fMorphStart, fMorphEnd);
		levelInfo.g_shadowViewProj = XMMatrixTranspose(m_terrainShadow[i].GetViewProj());
		if(i != selector.mMinSelectLevel)
			levelInfo.g_shadowViewProj1 = XMMatrixTranspose(m_terrainShadow[i-1].GetViewProj());
		m_VertexShader.UpdateSubresource(pImmediateContext, levelInfo);

		ID3D11ShaderResourceView* ShadowMapRSV = NULL;
		pImmediateContext->PSSetShaderResources(2, 1, &(ShadowMapRSV=m_terrainShadow[i  ].GetDepthMapShaderResView()));
		if(i != selector.mMinSelectLevel)
			pImmediateContext->PSSetShaderResources(3, 1, &(ShadowMapRSV=m_terrainShadow[i-1].GetDepthMapShaderResView()));

		CDLODTerrainQuadTree::NodeSlector::NodeContinerConstIter it = selector.mSelected[i].begin();
		for (; it != selector.mSelected[i].end(); ++it)
		{
			const CDLODTerrainQuadTree::SelectedNode& node = (*it);

			m_quadTree.GetAABB(node, boundingBox);
			CDLODTerrainVShader::cbNodeInfo nodeInfo;
			XMFLOAT3 boundingBoxSize = boundingBox.Max - boundingBox.Min;
			XMFLOAT3 boundingBoxOffset = boundingBox.Min;
			nodeInfo.g_quadScale = XMLoadFloat3(&boundingBoxSize);
			nodeInfo.g_quadOffset = XMLoadFloat3(&boundingBoxOffset);
			m_VertexShader.UpdateSubresource(pImmediateContext, nodeInfo);

			bool drawFull = node.TL && node.TR && node.BL && node.BR;
			if (drawFull)
			{
				pImmediateContext->DrawIndexed(nIndicesCount, 0, 0);
			}
			else
			{
				const int childIndiceCount = nIndicesCount / 4;
				if (node.TL)
					pImmediateContext->DrawIndexed(childIndiceCount, m_gridMesh.GetIndexStartTL(), 0);
				if (node.TR)
					pImmediateContext->DrawIndexed(childIndiceCount, m_gridMesh.GetIndexStartTR(), 0);
				if (node.BL)
					pImmediateContext->DrawIndexed(childIndiceCount, m_gridMesh.GetIndexStartBL(), 0);
				if (node.BR)
					pImmediateContext->DrawIndexed(childIndiceCount, m_gridMesh.GetIndexStartBR(), 0);
			}

			if (m_bDebugDraw)
			{
				const int& lodLevel = i;
				unsigned int color = LEVEL_COLOR[lodLevel];
				const CDLODTerrainQuadTree::SelectedNode& nodeSel = node;

				AABB boundingBox;
				m_quadTree.GetAABB(nodeSel, boundingBox);
				boundingBox.Expand(-0.001f);
				if (drawFull)
					GetDebugRender().DrawBox(boundingBox.Min, boundingBox.Max, color);
				else
				{
					float midX = boundingBox.Center().x;
					float midY = boundingBox.Center().y;

					if (nodeSel.TL)
					{
						AABB bbSub = boundingBox; bbSub.Max.x = midX; bbSub.Max.y = midY;
						bbSub.Expand(-0.002f);
						GetDebugRender().DrawBox(bbSub.Min, bbSub.Max, color);
					}
					if (nodeSel.TR)
					{
						AABB bbSub = boundingBox; bbSub.Min.x = midX; bbSub.Max.y = midY;
						bbSub.Expand(-0.002f);
						GetDebugRender().DrawBox(bbSub.Min, bbSub.Max, color);
					}
					if (nodeSel.BL)
					{
						AABB bbSub = boundingBox; bbSub.Max.x = midX; bbSub.Min.y = midY;
						bbSub.Expand(-0.002f);
						GetDebugRender().DrawBox(bbSub.Min, bbSub.Max, color);
					}
					if (nodeSel.BR)
					{
						AABB bbSub = boundingBox; bbSub.Min.x = midX; bbSub.Min.y = midY;
						bbSub.Expand(-0.002f);
						GetDebugRender().DrawBox(bbSub.Min, bbSub.Max, color);
					}
				}
			}
		}

	}
}


void CDLODTerrainRender::RendTerrainDepthMap(ID3D11DeviceContext* pImmediateContext)
{
	//设置定点输入
	UINT stride = sizeof(XMFLOAT3);	UINT offset = 0;
	ID3D11Buffer*pVertexBuffer = m_gridMesh.GetVertexBuffer();
	pImmediateContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	pImmediateContext->IASetIndexBuffer(m_gridMesh.GetIndexBuffer(), m_gridMesh.GetIndexFormat(), 0);
	pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pImmediateContext->IASetInputLayout(m_VertexShader.GetInputLayout());

	for (int d = m_selector.mMaxSelectLevel; d >= m_selector.mMinSelectLevel; --d)
	{
		CDLODTerrainShadow& shadowMap = m_terrainShadow[d];
		shadowMap.BeginRender(pImmediateContext);
		CDLODTerrainVShader::cbPerFrame cb1;
		cb1.g_viewProjection = XMMatrixTranspose(shadowMap.GetViewProj());//
		cb1.g_cameraPos = XMVectorSet(m_selector.mObservPos.x, m_selector.mObservPos.y, m_selector.mObservPos.z, 1.0f);
		const D3DXVECTOR3& lightDir = m_lightInfo.mForward;
		cb1.g_diffuseLightDir = XMVectorSet(lightDir.x, lightDir.y, lightDir.z, 1);
		m_VertexShader.UpdateSubresource(pImmediateContext, cb1);

		HRESULT hr;
		//pImmediateContext->PSSetShader(NULL, NULL, 0);
		pImmediateContext->RSSetState(m_pRasterizerState[TerrainRendFillMode::Solid]);

		CDLODTerrainQuadTree::NodeSlector	&selector = m_selector;
		AABB boundingBox;
		for (int i = d; i<=m_selector.mMaxSelectLevel&&i<=d+1; ++i)
		{
			CDLODTerrainQuadTree::NodeSlector::NodeContinerConstIter it = selector.mSelected[i].begin();
			const int nIndicesCount = m_gridMesh.GetIndicesCount();
			CDLODTerrainVShader::cbLodLevelInfo levelInfo;
			float fMorphStart, fMorphEnd;
			selector.GetLodLevlMorphDist(i, fMorphStart, fMorphEnd);
			levelInfo.SetLodLevlMorph(fMorphStart, fMorphEnd);
			m_VertexShader.UpdateSubresource(pImmediateContext, levelInfo);
			for (; it != selector.mSelected[i].end(); ++it)
			{
				const CDLODTerrainQuadTree::SelectedNode& node = (*it);

				m_quadTree.GetAABB(node, boundingBox);
				CDLODTerrainVShader::cbNodeInfo nodeInfo;
				XMFLOAT3 boundingBoxSize = boundingBox.Max - boundingBox.Min;
				XMFLOAT3 boundingBoxOffset = boundingBox.Min;
				nodeInfo.g_quadScale = XMLoadFloat3(&boundingBoxSize);
				nodeInfo.g_quadOffset = XMLoadFloat3(&boundingBoxOffset);
				m_VertexShader.UpdateSubresource(pImmediateContext, nodeInfo);

				bool drawFull = node.TL && node.TR && node.BL && node.BR;
				if (drawFull)
				{
					pImmediateContext->DrawIndexed(nIndicesCount, 0, 0);
				}
				else
				{
					const int childIndiceCount = nIndicesCount / 4;
					if (node.TL)
						pImmediateContext->DrawIndexed(childIndiceCount, m_gridMesh.GetIndexStartTL(), 0);
					if (node.TR)
						pImmediateContext->DrawIndexed(childIndiceCount, m_gridMesh.GetIndexStartTR(), 0);
					if (node.BL)
						pImmediateContext->DrawIndexed(childIndiceCount, m_gridMesh.GetIndexStartBL(), 0);
					if (node.BR)
						pImmediateContext->DrawIndexed(childIndiceCount, m_gridMesh.GetIndexStartBR(), 0);
				}
			}

		}

		shadowMap.EndRender(pImmediateContext);
	}
#if 1
	for (int d = m_selector.mMaxSelectLevel; d >= m_selector.mMinSelectLevel; --d)
	{
		CDLODTerrainShadow& shadowMap = m_terrainShadow[d];
		ID3D11RenderTargetView* pView[1] = {m_pCascadedShadowMapTempBlurRTV};
		ID3D11ShaderResourceView* shaderResView[1] = {shadowMap.GetRendTarShaderResView()};
		D3D11_VIEWPORT viewport;
		viewport.Width = (float)m_shadowmapWidth;
		viewport.Height = (float)m_shadowmapHeight;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		pImmediateContext->PSSetSamplers(0,1, &GetHelpRender()->linearBorderSampler);
		pImmediateContext->RSSetViewports(1, &viewport);

		pImmediateContext->OMSetRenderTargets( 1, &pView[0] , NULL );
		pImmediateContext->PSSetShaderResources( 0, 1, &shaderResView[0]);
		GetHelpRender()->DrawFullScreenQuad(pImmediateContext, m_blurHPixelShader.GetShader());

		pView[0] = shadowMap.GetRenderTargetView();
		shaderResView[0] = m_pCascadedShadowMapTempBlurSRV;

		pImmediateContext->OMSetRenderTargets( 1, &pView[0] , NULL );
		pImmediateContext->PSSetShaderResources( 0, 1, &shaderResView[0]);
		GetHelpRender()->DrawFullScreenQuad(pImmediateContext, m_blurVPixelShader.GetShader());
	}
#endif

}
inline D3DXVECTOR3 AvgNormalFromQuad( float ha, float hb, float hc, float hd, float sizex, float sizey, float scaleH )
{
	/*
	a******c
	*      *
	*      *
	b******d
	*/
	D3DXVECTOR3 n0, n1;

	n0.x = - (hb - ha) * scaleH * sizey;
	n0.y = - sizex * (hc - ha) * scaleH;
	n0.z = sizex * sizey;

	//D3DXVec3Normalize( &n0, &n0 );

	n1.x = -sizey * (hd-hc) * scaleH;
	n1.y = ((hb-hc) * sizex - sizex * (hd-hc)) * scaleH;
	n1.z = sizey * sizex;


	//D3DXVec3Normalize( &n1, &n1 );

	n0 += n1;
	//D3DXVec3Normalize( &n0, &n0 );

	return n0;
}
//
static inline int CoordClamp( int val, int limit )
{
	if( val < 0 )        return 0;
	if( val > limit - 1 )  return limit - 1;
	return val;
}
//
static inline int CoordWrap( int val, int limit )
{
	if( val < 0 )        return limit + val;
	if( val > limit - 1 )  return val - limit;
	return val;
}
//
static inline int CoordFix( int val, int limit, bool wrap )
{
	if( wrap )
		return CoordWrap( val, limit );
	else
		return CoordClamp( val, limit );
}
static void CreateNormalMap( int sizeX, int sizeY, const MapDimensions& mapDims, float* heightmapData, int heightmapDataPitch, XMFLOAT3* normalmapData, int normalmapDataPitch, bool wrapEdges )
{
   const float stepx = 1.0f / (sizeX-1) * mapDims.SizeX;
   const float stepy = 1.0f / (sizeY-1) * mapDims.SizeY;

   const int smoothSteps = 0; // can be 0, 1, 2, ... more steps == slower algorithm
   for( int dist = 1; dist < 2+smoothSteps; dist++ )
   {
      for( int y = 0; y < sizeY; y++ )
      {
         float* hmScanLine0 = &heightmapData[ CoordFix(y-dist, sizeY, wrapEdges) * (heightmapDataPitch/sizeof(float)) ];
         float* hmScanLine1 = &heightmapData[ CoordFix(y+0,	sizeY, wrapEdges) * (heightmapDataPitch/sizeof(float)) ];
         float* hmScanLine2 = &heightmapData[ CoordFix(y+dist, sizeY, wrapEdges) * (heightmapDataPitch/sizeof(float)) ];

         XMFLOAT3* nmScanLine = &normalmapData[ y * (normalmapDataPitch/sizeof(XMFLOAT3)) ];

         for( int x = 0; x < sizeX; x++ )
         {
            int xcoordm   = CoordFix( x-dist, sizeX, wrapEdges );
            int xcoord    = CoordFix( x,	  sizeX, wrapEdges );
            int xcoordp   = CoordFix( x+dist, sizeX, wrapEdges );

			/*
			a*******d******g
			*		*	   *
			*		*	   *
			b*******e******h
			*		*	   *
			*		*	   *
			c*******f******i
			*/
            float ha = hmScanLine0[xcoordm];
            float hb = hmScanLine1[xcoordm];
            float hc = hmScanLine2[xcoordm];
            float hd = hmScanLine0[xcoord];
            float he = hmScanLine1[xcoord];
            float hf = hmScanLine2[xcoord];
            float hg = hmScanLine0[xcoordp];
            float hh = hmScanLine1[xcoordp];
            float hi = hmScanLine2[xcoordp];

            D3DXVECTOR3 norm( 0, 0, 0 );
            norm += AvgNormalFromQuad( ha, hb, hd, he, stepx, stepy, mapDims.SizeH );
            norm += AvgNormalFromQuad( hb, hc, he, hf, stepx, stepy, mapDims.SizeH );
            norm += AvgNormalFromQuad( hd, he, hg, hh, stepx, stepy, mapDims.SizeH );
            norm += AvgNormalFromQuad( he, hf, hh, hi, stepx, stepy, mapDims.SizeH );

            D3DXVec3Normalize( &norm, &norm );

#if 0
            if( dist > 1 )
            {
               //D3DXVECTOR3 oldNorm( vaHalfFloatUnpack( (unsigned short)(nmScanLine[x] & 0xFFFF) ), vaHalfFloatUnpack( (unsigned short)(nmScanLine[x] >> 16) ), 0 );
               D3DXVECTOR3 oldNorm( ((nmScanLine[x] >> 16) / 65535.0f - 0.5f) / 0.5f, ((nmScanLine[x] & 0xFFFF ) / 65535.0f - 0.5f) / 0.5f, 0 );
               oldNorm.z = sqrtf( 1 - oldNorm.x*oldNorm.x - oldNorm.y*oldNorm.y );

               norm += oldNorm * 1.0f; // use bigger const to add more weight to normals calculated from smaller quads
               D3DXVec3Normalize( &norm, &norm );
            }
            unsigned short a = (unsigned short)clamp( 65535.0f * ( norm.x * 0.5f + 0.5f ), 0.0f, 65535.0f );
            unsigned short b = (unsigned short)clamp( 65535.0f * ( norm.y * 0.5f + 0.5f ), 0.0f, 65535.0f );

            nmScanLine[x] = (a << 16) | b;
#else
			nmScanLine[x] = XMFLOAT3(norm.x,norm.y,norm.z);
#endif
         }
      }
   }
}

