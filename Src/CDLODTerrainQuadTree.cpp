#include "DXUT.h"
#include <Windows.h>
#include <assert.h>
#include "CDLODTerrainQuadTree.h"
#include "DebugRender.h"
#include "common.h"


unsigned short	CDLODTerrainQuadTree::Node::GetLevel() const { return (unsigned short)(Level & 0x7FFF); }
bool			CDLODTerrainQuadTree::Node::IsLeaf()	const { return ((Level&0x8000)!=0); }

CDLODTerrainQuadTree::CDLODTerrainQuadTree(void):m_allNodesBuff(0),m_allNodesCount(0)
{
}

CDLODTerrainQuadTree::~CDLODTerrainQuadTree(void)
{
	if(m_allNodesBuff)
		delete [] m_allNodesBuff;
	if(m_topLevelNode)
		delete [] m_topLevelNode;
}

void CDLODTerrainQuadTree::GetAABB(const Node* node, AABB & aabb) const
{
	GetWorldMinMaxX(node, aabb.Min.x, aabb.Max.x);
	GetWorldMinMaxH(node, aabb.Min.y, aabb.Max.y);
	GetWorldMinMaxY(node, aabb.Min.z, aabb.Max.z);
}
void CDLODTerrainQuadTree::DrawAllNode()
{
	for( int i = 0; i < m_allNodesCount; i++ )
		if( m_allNodesBuff[i].GetLevel() != 0 )
		{
			AABB aabb;
			GetAABB(m_allNodesBuff+i,aabb);
			GetDebugRender().DrawBox(aabb.Min, aabb.Max, 0xff00ffff);
		}

		for( int i = 0; i < m_allNodesCount; i++ )
			if( m_allNodesBuff[i].GetLevel() == 0 ) 
			{
				AABB aabb;
				GetAABB(m_allNodesBuff+i,aabb);
				GetDebugRender().DrawBox(aabb.Min, aabb.Max, 0xff00ff00);
			}
}
bool CDLODTerrainQuadTree::Create( const CreateDesc& desc )
{
	m_desc = desc;
	m_rasterSizeX  = desc.pHeightmap->GetSizeX();
	m_rasterSizeY  = desc.pHeightmap->GetSizeY();


	int totalNodeCount = 0;
	m_topNodeSize = desc.LeafRenderNodeSize;

	///逐层计算结点数
	for (int i=0; i<m_desc.LODLevelCount; ++i)
	{
		if( i != 0 ) m_topNodeSize *= 2;
		int nodeCountX = (m_rasterSizeX-1) / m_topNodeSize + 1;
		int nodeCountY = (m_rasterSizeY-1) / m_topNodeSize + 1;
		totalNodeCount += nodeCountX * nodeCountY;
	}

	///
	m_allNodesBuff = new Node[totalNodeCount];
	m_topNodeCountX = (m_rasterSizeX-1) / m_topNodeSize + 1;
	m_topNodeCountY = (m_rasterSizeY-1) / m_topNodeSize + 1;
	int nodeCounter = 0;

	m_topLevelNode = new Node*[m_topNodeCountX*m_topNodeCountY];
	memset(m_topLevelNode, 0,sizeof(Node*)*m_topNodeCountX*m_topNodeCountY);
	for( int y = 0; y < m_topNodeCountY; y++ )
	{
		for( int x = 0; x < m_topNodeCountX; x++ )
		{
			m_topLevelNode[y*m_topNodeCountX+x] = &m_allNodesBuff[nodeCounter++];
			Create(m_topLevelNode[y*m_topNodeCountX+x], x*m_topNodeSize,y*m_topNodeSize,m_topNodeSize,0,m_allNodesBuff,nodeCounter);
		}
	}

	m_allNodesCount = nodeCounter;


	return true;
}


bool CDLODTerrainQuadTree::Create( Node*node, int x, int y, int size, int level, Node*nodeBuffer, int& lastIdx )
{
	const CreateDesc& desc = m_desc;

	node->X     = (unsigned short)x;       assert( x >= 0 && x < 65535 );
	node->Y     = (unsigned short)y;       assert( y >= 0 && y < 65535 );
	node->Level = (unsigned short)level;   assert( level >= 0 && level < 16 );
	node->Size  = (unsigned short)size;    assert( size >= 0 && level < 32768 );

	int rasterSizeX = desc.pHeightmap->GetSizeX();
	int rasterSizeY = desc.pHeightmap->GetSizeY();

	node->SubTL = NULL;
	node->SubTR = NULL;
	node->SubBL = NULL;
	node->SubBR = NULL;

	if( size == (desc.LeafRenderNodeSize) )	//如果是叶子结点
	{
		assert( level == (desc.LODLevelCount-1) );
		node->Level |= 0x8000;

		// Find min/max heights at this patch of terrain
		int limitX = min( rasterSizeX, x + size + 1 );
		int limitY = min( rasterSizeY, y + size + 1 );
		desc.pHeightmap->GetAreaMinMaxH( x, y, limitX - x, limitY - y, node->MinH, node->MaxH );
	}
	else
	{
		//top left
		int subSize = size / 2;
		node->SubTL = &nodeBuffer[lastIdx++];
		Create(node->SubTL, x,y,subSize, level+1, nodeBuffer, lastIdx);
		node->MinH = node->SubTL->MinH;
		node->MaxH = node->SubTL->MaxH;

		//top right
		if( (x + subSize) < rasterSizeX )
		{
			node->SubTR = &nodeBuffer[lastIdx++];
			Create(node->SubTR, x + subSize, y, subSize, level+1, nodeBuffer, lastIdx );
			node->MinH = min( node->MinH, node->SubTR->MinH );
			node->MaxH = max( node->MaxH, node->SubTR->MaxH );
		}

		//bottom left
		if( (y + subSize) < rasterSizeY )
		{
			node->SubBL = &nodeBuffer[lastIdx++];
			Create(node->SubBL, x, y + subSize, subSize, level+1, nodeBuffer, lastIdx );
			node->MinH = min( node->MinH, node->SubBL->MinH );
			node->MaxH = max( node->MaxH, node->SubBL->MaxH );
		}

		//bottom right
		if( ((x + subSize) < rasterSizeX) && ((y + subSize) < rasterSizeY) )
		{
			node->SubBR = &nodeBuffer[lastIdx++];
			Create(node->SubBR, x + subSize, y + subSize, subSize, level+1, nodeBuffer, lastIdx );
			node->MinH = min( node->MinH, node->SubBR->MinH );
			node->MaxH = max( node->MaxH, node->SubBR->MaxH );
		}
	}

	return true;
}

void CDLODTerrainQuadTree::Select( NodeSlector& selector )
{
	selector.GetFrustumPlanes(selector.mFrustumPlanes,selector.mViewProj);


	const int   layerCount           = m_desc.LODLevelCount;

	float total                = 0;
	float currentDetailBalance = 1.0f;
	const float detailBalance  =2;
	float LODNear           = 0;
	float LODFar            = 4750;
	selector.mStopAtLevel     = layerCount-1;

	for (int i=0; i<layerCount; i++) 
	{
		total += currentDetailBalance;
		currentDetailBalance *= detailBalance;
	}

	float sect = (LODFar - LODNear) / total;

	float prevPos = LODNear;
	currentDetailBalance = 1.0f;
	for( int i = 0; i < layerCount; i++ ) 
	{
		const int LODLevel = layerCount-1-i;
		selector.mVisibilityRanges[LODLevel] = prevPos + sect * currentDetailBalance;
		prevPos = selector.mVisibilityRanges[LODLevel];
		currentDetailBalance *= detailBalance;
	}

	prevPos = LODNear;
	for (int i=0; i<layerCount; i++)
	{
		int LODLevel = layerCount - i - 1;
		selector.mMorphEnd[LODLevel] = selector.mVisibilityRanges[LODLevel];
		selector.mMorphStart[LODLevel] = prevPos + (selector.mMorphEnd[LODLevel] - prevPos) * selector.mMorphStartRatio;

		prevPos = selector.mMorphStart[LODLevel];
	}



	for( int y = 0; y < m_topNodeCountY; y++ ){
		for( int x = 0; x < m_topNodeCountX; x++ )
		{
			Node* node = m_topLevelNode[y*m_topNodeCountX+x];
			SelectNode(node, selector, false);
		}
	}
}
CDLODTerrainQuadTree::LODSelectRes::Type CDLODTerrainQuadTree::SelectNode(Node*node, NodeSlector& selector, bool bParentCompletelyInFrustum)
{
	AABB boundingBox;
	GetAABB( node, boundingBox);


	const D3DXPLANE* frustumPlanes  = selector.mFrustumPlanes;
	const D3DXVECTOR3& observerPos  = selector.mObservPos;
	const float* lodRanges			= selector.mVisibilityRanges;


	//视椎体检测（结点在视椎体外）
	IntersectRes::Type frustumIt = (bParentCompletelyInFrustum) ? (IntersectRes::eInside) : (boundingBox.TestInBoundingPlanes(frustumPlanes));
	if (frustumIt == IntersectRes::eOutside)
		return LODSelectRes::eOutOfFrustum;

	///细节范围检测
	float distanceLimit = lodRanges[node->GetLevel()];
	if( !boundingBox.IntersectSphereSq(observerPos, distanceLimit*distanceLimit) )
		return LODSelectRes::eOutOfRange;

	LODSelectRes::Type SubTLSelRes = LODSelectRes::eUndefined;
	LODSelectRes::Type SubTRSelRes = LODSelectRes::eUndefined;
	LODSelectRes::Type SubBLSelRes = LODSelectRes::eUndefined;
	LODSelectRes::Type SubBRSelRes = LODSelectRes::eUndefined;

	///如果不是叶子结点,则进行子结点处理
	if( node->GetLevel() != selector.mStopAtLevel )
	{
		//如果该结点的所有子结点都在他们的细节层次范围内
		float nextDistanceLimit = lodRanges[node->GetLevel()+1];
		if( boundingBox.IntersectSphereSq( observerPos, nextDistanceLimit*nextDistanceLimit ) )
		{
			bool bCompletelyInFrustum = (frustumIt == IntersectRes::eInside);
			if( node->SubTL != NULL ) SubTLSelRes = SelectNode(node->SubTL,selector, bCompletelyInFrustum );
			if( node->SubTR != NULL ) SubTRSelRes = SelectNode(node->SubTR,selector, bCompletelyInFrustum );
			if( node->SubBL != NULL ) SubBLSelRes = SelectNode(node->SubBL,selector, bCompletelyInFrustum );
			if( node->SubBR != NULL ) SubBRSelRes = SelectNode(node->SubBR,selector, bCompletelyInFrustum );
		}
	}

	bool bRemoveSubTL = (SubTLSelRes == LODSelectRes::eOutOfFrustum) || (SubTLSelRes == LODSelectRes::eSelected);
	bool bRemoveSubTR = (SubTRSelRes == LODSelectRes::eOutOfFrustum) || (SubTRSelRes == LODSelectRes::eSelected);
	bool bRemoveSubBL = (SubBLSelRes == LODSelectRes::eOutOfFrustum) || (SubBLSelRes == LODSelectRes::eSelected);
	bool bRemoveSubBR = (SubBRSelRes == LODSelectRes::eOutOfFrustum) || (SubBRSelRes == LODSelectRes::eSelected);

	//1.如果并不是所有的子结点都被选择，则选择当前结点
	//2.如果并不是所有的子结点都在视椎体外，则选择当前结点
	if( !(bRemoveSubTL && bRemoveSubTR && bRemoveSubBL && bRemoveSubBR) )
	{
		int LODLevel = node->GetLevel();
		selector.mMinSelectLevel = min(selector.mMinSelectLevel, LODLevel);
		selector.mMaxSelectLevel = max(selector.mMaxSelectLevel, LODLevel);
		selector.mSelected[LODLevel].push_back(SelectedNode( node, LODLevel, !bRemoveSubTL, !bRemoveSubTR, !bRemoveSubBL, !bRemoveSubBR ));
		return LODSelectRes::eSelected;
	}

	//如果有任何子结点被选择，则返回eSelected，否则返回eOutOfFrustum
	if( (SubTLSelRes == LODSelectRes::eSelected) || (SubTRSelRes == LODSelectRes::eSelected) || (SubBLSelRes == LODSelectRes::eSelected) || (SubBRSelRes == LODSelectRes::eSelected) )
		return LODSelectRes::eSelected;
	else
		return LODSelectRes::eOutOfFrustum;

}


void CDLODTerrainQuadTree::GetAreaMinMaxHeight(Node* node, int fromX, int fromY, int toX, int toY, float & minH, float & maxH) const
{
	if (((toX < node->X) || (toY < node->Y)) || ((fromX >(node->X + node->Size)) || (fromY >(node->Y + node->Size))))
	{
		return;
	}

	bool hasNoLeafs = node->IsLeaf();

	if (hasNoLeafs || (((fromX <= node->X) && (fromY <= node->Y)) && ((toX >= (node->X + node->Size)) && (toY >= (node->Y + node->Size)))))
	{
		float worldMinZ, worldMaxZ;
		GetWorldMinMaxH(node, worldMinZ, worldMaxZ);
		minH = min(minH, worldMinZ);
		maxH = max(maxH, worldMaxZ);
		return;
	}

	// Partially inside, partially outside
	if (node->SubTL != NULL)
		GetAreaMinMaxHeight(node->SubTL, fromX, fromY, toX, toY, minH, maxH);
	if (node->SubTR != NULL)
		GetAreaMinMaxHeight(node->SubTR, fromX, fromY, toX, toY, minH, maxH);
	if (node->SubBL != NULL)
		GetAreaMinMaxHeight(node->SubBL, fromX, fromY, toX, toY, minH, maxH);
	if (node->SubBR != NULL)
		GetAreaMinMaxHeight(node->SubBR, fromX, fromY, toX, toY, minH, maxH);
}

void CDLODTerrainQuadTree::GetAreaMinMaxHeight(float fromX, float fromY, float sizeX, float sizeY, float & minH, float & maxH) const
{
	float bfx = (fromX - m_desc.MapDims.MinX) / m_desc.MapDims.SizeX;
	float bfy = (fromY - m_desc.MapDims.MinY) / m_desc.MapDims.SizeY;
	float btx = (fromX + sizeX - m_desc.MapDims.MinX) / m_desc.MapDims.SizeX;
	float bty = (fromY + sizeY - m_desc.MapDims.MinY) / m_desc.MapDims.SizeY;

	int rasterFromX = clamp((int)(bfx * m_rasterSizeX), 0, m_rasterSizeX - 1);
	int rasterFromY = clamp((int)(bfy * m_rasterSizeY), 0, m_rasterSizeY - 1);
	int rasterToX = clamp((int)(btx * m_rasterSizeX), 0, m_rasterSizeX - 1);
	int rasterToY = clamp((int)(bty * m_rasterSizeY), 0, m_rasterSizeY - 1);

	int baseFromX = rasterFromX / m_topNodeSize;
	int baseFromY = rasterFromY / m_topNodeSize;
	int baseToX = rasterToX / m_topNodeSize;
	int baseToY = rasterToY / m_topNodeSize;

	assert(baseFromX < m_topNodeCountX);
	assert(baseFromY < m_topNodeCountY);
	assert(baseToX < m_topNodeCountX);
	assert(baseToY < m_topNodeCountY);

	minH = FLT_MAX;
	maxH = -FLT_MAX;

	for (int y = baseFromY; y <= baseToY; y++)
		for (int x = baseFromX; x <= baseToX; x++)
		{
			GetAreaMinMaxHeight(m_topLevelNode[y*m_topNodeCountX + x], rasterFromX, rasterFromY, rasterToX, rasterToY, minH, maxH);
		}

	//GetCanvas3D()->DrawBox( D3DXVECTOR3(fromX, fromY, minZ), D3DXVECTOR3(fromX + sizeX, fromY + sizeY, maxZ), 0xFFFFFF00, 0x10FFFF00 );
}

void CDLODTerrainQuadTree::NodeSlector::GetFrustumPlanes( D3DXPLANE * pPlanes, const D3DXMATRIX & mCameraViewProj )
{
	// Left clipping plane
	pPlanes[0].a = mCameraViewProj(0,3) + mCameraViewProj(0,0);
	pPlanes[0].b = mCameraViewProj(1,3) + mCameraViewProj(1,0);
	pPlanes[0].c = mCameraViewProj(2,3) + mCameraViewProj(2,0);
	pPlanes[0].d = mCameraViewProj(3,3) + mCameraViewProj(3,0);

	// Right clipping plane
	pPlanes[1].a = mCameraViewProj(0,3) - mCameraViewProj(0,0);
	pPlanes[1].b = mCameraViewProj(1,3) - mCameraViewProj(1,0);
	pPlanes[1].c = mCameraViewProj(2,3) - mCameraViewProj(2,0);
	pPlanes[1].d = mCameraViewProj(3,3) - mCameraViewProj(3,0);

	// Top clipping plane
	pPlanes[2].a = mCameraViewProj(0,3) - mCameraViewProj(0,1);
	pPlanes[2].b = mCameraViewProj(1,3) - mCameraViewProj(1,1);
	pPlanes[2].c = mCameraViewProj(2,3) - mCameraViewProj(2,1);
	pPlanes[2].d = mCameraViewProj(3,3) - mCameraViewProj(3,1);

	// Bottom clipping plane
	pPlanes[3].a = mCameraViewProj(0,3) + mCameraViewProj(0,1);
	pPlanes[3].b = mCameraViewProj(1,3) + mCameraViewProj(1,1);
	pPlanes[3].c = mCameraViewProj(2,3) + mCameraViewProj(2,1);
	pPlanes[3].d = mCameraViewProj(3,3) + mCameraViewProj(3,1);

	// Near clipping plane
	pPlanes[4].a = mCameraViewProj(0,2);
	pPlanes[4].b = mCameraViewProj(1,2);
	pPlanes[4].c = mCameraViewProj(2,2);
	pPlanes[4].d = mCameraViewProj(3,2);

	// Far clipping plane
	pPlanes[5].a = mCameraViewProj(0,3) - mCameraViewProj(0,2);
	pPlanes[5].b = mCameraViewProj(1,3) - mCameraViewProj(1,2);
	pPlanes[5].c = mCameraViewProj(2,3) - mCameraViewProj(2,2);
	pPlanes[5].d = mCameraViewProj(3,3) - mCameraViewProj(3,2);

	// Normalize the plane equations, if requested
	for (int i = 0; i < 6; i++) 
		D3DXPlaneNormalize( &pPlanes[i], &pPlanes[i] );
}
