#include <Windows.h>
#include <d3dx9math.h>
#include <xnamath.h>
#include <list>
#include "AABB.h"
#pragma once

struct MapDimensions
{
	float                MinX;
	float                MinY;
	float                MinH;
	float                SizeX;
	float                SizeY;
	float                SizeH;

	float                MaxX() const   { return MinX + SizeX; }
	float                MaxY() const   { return MinY + SizeY; }
	float                MaxH() const   { return MinH + SizeH; }
};

class IHeightmapSource
{
public:
	virtual int             GetSizeX( )                   = 0;
	virtual int             GetSizeY( )                   = 0;

	// returned value is converted to height using following formula:
	// 'WorldHeight = WorldMinZ + GetHeightAt(,) * WorldSizeZ / 65535.0f'
	virtual unsigned short  GetHeightAt( int x, int y )   = 0;

	virtual void            GetAreaMinMaxH( int x, int y, int sizeX, int sizeY, unsigned short & minZ, unsigned short & maxZ ) = 0;
};

template<class VertexType>
class CDLODTerrainGridMesh
{
public:
	typedef VertexType Vertex;
	CDLODTerrainGridMesh(const int dimention);
	~CDLODTerrainGridMesh();

	int				GetDimentions() const { return m_dimentions; }

	VertexType*		GetVertexData() const { return m_vertexsBuffer; }
	int				GetVertexCount() const { return m_vertexsCount; }

	unsigned short*	GetIndicesData() const { return m_indicesBuffer; }
	int				GetIndicesCount() const { return m_indicesCount; }

	int				GetIndexStartTL() const { return m_indexStartTL; }
	int				GetIndexStartTR() const { return m_indexStartTR; }
	int				GetIndexStartBL() const { return m_indexStartBL; }
	int				GetIndexStartBR() const { return m_indexStartBR; }

private:
	VertexType*		m_vertexsBuffer;
	int				m_vertexsCount;
	unsigned short*	m_indicesBuffer;
	int				m_indicesCount;
	int				m_dimentions;

	int				m_indexStartTL;
	int				m_indexStartTR;
	int				m_indexStartBL;
	int				m_indexStartBR;

};


class CDLODTerrainQuadTree
{
public:
	static const int     MAX_LOD_LEVELS    = 15;

	struct LODSelectRes
	{
		enum Type {
			eUndefined,
			eOutOfFrustum,
			eOutOfRange,
			eSelected,
		};
	};
	

	struct Node;
	struct SelectedNode
	{
		unsigned int   X;
		unsigned int   Y;
		unsigned short Size;
		unsigned short MinH;
		unsigned short MaxH;

		bool           TL;
		bool           TR;
		bool           BL;
		bool           BR;
		float          MinDistToCamera;
		int            LODLevel;

		SelectedNode() {}
		SelectedNode( const Node * node, int LODLevel, bool tl, bool tr, bool bl, bool br );
	};

	class NodeSlector
	{
	public:
		typedef std::list<SelectedNode> NodeContiner;
		typedef std::list<SelectedNode>::iterator NodeContinerIter;
		typedef std::list<SelectedNode>::const_iterator NodeContinerConstIter;
		typedef std::list<SelectedNode>::const_iterator NodeContinerConstIter;

		NodeSlector()
		{
			memset(mVisibilityRanges, 0, sizeof(mVisibilityRanges));
			mMorphStartRatio = 0.6666f;
			mMinSelectLevel = MAX_LOD_LEVELS;
			mMaxSelectLevel = 0;
		}
		void Clear()
		{
			for (int i = 0; i < MAX_LOD_LEVELS; ++i)
				mSelected[i].clear();
			mMinSelectLevel = MAX_LOD_LEVELS;
			mMaxSelectLevel = 0;
		}
		D3DXMATRIX	mViewProj;
		D3DXPLANE	mFrustumPlanes[6];
		D3DXVECTOR3 mObservPos;
		int			mStopAtLevel;
		int			mMinSelectLevel;
		int			mMaxSelectLevel;
		float		mMorphStartRatio;
		float		mVisibilityRanges[MAX_LOD_LEVELS];
		float		mMorphEnd[MAX_LOD_LEVELS];
		float		mMorphStart[MAX_LOD_LEVELS];

		NodeContiner mSelected[MAX_LOD_LEVELS];


		void	GetFrustumPlanes( D3DXPLANE * pPlanes, const D3DXMATRIX & mCameraViewProj );
		void	GetLodLevlMorphDist(int lodLevl, float& startDist, float& endDist) const
		{
			startDist	= mMorphStart[lodLevl];
			endDist		= mMorphEnd[lodLevl];
		}

	};

	struct CreateDesc
	{
		IHeightmapSource* pHeightmap;
		int               LeafRenderNodeSize;
		int               LODLevelCount;
		MapDimensions     MapDims;

	};

	struct Node
	{
		unsigned short	X;
		unsigned short	Y;
		unsigned short	Size;

		unsigned short	Level;

		unsigned short	MinH;
		unsigned short	MaxH;

		unsigned short	GetLevel() const;
		bool			IsLeaf() const;


		Node *			SubTL;
		Node *			SubTR;
		Node *			SubBL;
		Node *			SubBR;
	};

			CDLODTerrainQuadTree(void);
			~CDLODTerrainQuadTree(void);

	bool	Create(const CreateDesc& desc);
	void	DrawAllNode();
	void	DrawSelector(const NodeSlector& selected);

	void	Select(NodeSlector& selector);
	void	GetAABB(const Node*node, AABB & aabb) const;
	void	GetAABB( const SelectedNode& node,AABB & aabb) const;
	void	GetWorldMinMaxX(const Node* node, float & minX, float & maxX) const;
	void	GetWorldMinMaxY(const Node* node, float & minY, float & maxY) const;
	void	GetWorldMinMaxH(const Node* node, float & minZ, float & maxZ) const;
	int		GetRasterSizeX() const { return m_rasterSizeX; }
	int		GetRasterSizeY() const { return m_rasterSizeY; }
	bool	SelectNode(Node*node, NodeSlector& selector);
	void	GetAreaMinMaxHeight(float fromX, float fromY, float sizeX, float sizeY, float & minH, float & maxH) const;
	void	GetAreaMinMaxHeight(Node* node, int fromX, int fromY, int toX, int toY, float & minH, float & maxH) const;
	int		GetLODLevelCount() const { return m_desc.LODLevelCount; }
	const MapDimensions &   GetWorldMapDims() const	{ return m_desc.MapDims; }
private:
	bool Create( Node*node, int x, int y, int size, int level, Node*nodeBuffer, int& lastIdx );
	LODSelectRes::Type	SelectNode(Node*node, NodeSlector& selector, bool bParentCompletelyInFrustum);
	Node*	m_allNodesBuff;
	Node**	m_topLevelNode;
	int		m_allNodesCount;

	int		m_rasterSizeX;
	int		m_rasterSizeY;


	int		m_topNodeSize;
	int		m_topNodeCountX;
	int		m_topNodeCountY;

	CreateDesc	m_desc;
};


template<class VertexType>
CDLODTerrainGridMesh<VertexType>::CDLODTerrainGridMesh( const int dimention ):m_dimentions(dimention)
{
	const int vertDim = dimention + 1;
	m_vertexsCount		= vertDim*vertDim;
	m_vertexsBuffer	= new VertexType[m_vertexsCount];
	m_indicesCount	= dimention * dimention * 2 * 3;	//
	m_indicesBuffer	= new unsigned short[m_indicesCount];


	for( int y = 0; y < vertDim; y++ ){
		for( int x = 0; x < vertDim; x++ )
		{
			VertexType& vert = m_vertexsBuffer[x + vertDim * y];
			vert.x = x/(float)(dimention);
			vert.y = 0;
			vert.z = y/(float)(dimention);
		}
	}


	int index = 0;
	int halfd = (vertDim/2);
	int fulld = dimention;
	unsigned short*const indices = m_indicesBuffer;

	m_indexStartTL = index;
	// Top left part
	for( int y = 0; y < halfd; y++ ) {
		for( int x = 0; x < halfd; x++ ) {
			indices[index++] = (unsigned short)( x		+ vertDim *  y	 );
			indices[index++] = (unsigned short)((x+1)	+ vertDim *  y	 );
			indices[index++] = (unsigned short)( x		+ vertDim * (y+1));

			indices[index++] = (unsigned short)((x+1)	+ vertDim *  y	 );
			indices[index++] = (unsigned short)((x+1)	+ vertDim * (y+1)); 
			indices[index++] = (unsigned short)( x		+ vertDim * (y+1));
		}
	}

	// Top right part
	m_indexStartTR = index;
	for( int y = 0; y < halfd; y++ ) {
		for( int x = halfd; x < fulld; x++ ) {
			indices[index++] = (unsigned short)( x		+ vertDim *  y	 );
			indices[index++] = (unsigned short)((x+1)	+ vertDim *  y	 );
			indices[index++] = (unsigned short)( x		+ vertDim * (y+1));

			indices[index++] = (unsigned short)((x+1)	+ vertDim *  y	 );
			indices[index++] = (unsigned short)((x+1)	+ vertDim * (y+1)); 
			indices[index++] = (unsigned short)( x		+ vertDim * (y+1));
		}
	}

	// Bottom left part
	m_indexStartBL = index;
	for( int y = halfd; y < fulld; y++ ){
		for( int x = 0; x < halfd; x++ ){
			indices[index++] = (unsigned short)( x		+ vertDim *  y	 );
			indices[index++] = (unsigned short)((x+1)	+ vertDim *  y	 );
			indices[index++] = (unsigned short)( x		+ vertDim * (y+1));

			indices[index++] = (unsigned short)((x+1)	+ vertDim *  y	 );
			indices[index++] = (unsigned short)((x+1)	+ vertDim * (y+1));
			indices[index++] = (unsigned short)( x		+ vertDim * (y+1));
		}
	}

	// Bottom right part
	m_indexStartBR = index;
	for( int y = halfd; y < fulld; y++ ) {
		for( int x = halfd; x < fulld; x++ ) {
			indices[index++] = (unsigned short)( x		+ vertDim * y	 );
			indices[index++] = (unsigned short)((x+1)	+ vertDim * y	 );
			indices[index++] = (unsigned short)( x		+ vertDim * (y+1));

			indices[index++] = (unsigned short)((x+1)	+ vertDim * y	 );
			indices[index++] = (unsigned short)((x+1)	+ vertDim * (y+1));
			indices[index++] = (unsigned short)( x		+ vertDim * (y+1));
		}
	}

}


template<class VertexType>
CDLODTerrainGridMesh<VertexType>::~CDLODTerrainGridMesh()
{
	if (m_vertexsBuffer)
		delete []m_vertexsBuffer;

	if(m_indicesBuffer)
		delete []m_indicesBuffer;
}

//
void inline CDLODTerrainQuadTree::GetWorldMinMaxX(const Node* node, float & minX, float & maxX) const
{
	minX   = m_desc.MapDims.MinX + node->X * m_desc.MapDims.SizeX / (float)(m_rasterSizeX-1);
	maxX   = m_desc.MapDims.MinX + (node->X+node->Size) * m_desc.MapDims.SizeX / (float)(m_rasterSizeX-1);
}
//
void inline CDLODTerrainQuadTree::GetWorldMinMaxY(const Node* node, float & minY, float & maxY) const
{
	minY   = m_desc.MapDims.MinY + node->Y * m_desc.MapDims.SizeY / (float)(m_rasterSizeY-1);
	maxY   = m_desc.MapDims.MinY + (node->Y+node->Size) * m_desc.MapDims.SizeY / (float)(m_rasterSizeY-1);
}
//
void inline CDLODTerrainQuadTree::GetWorldMinMaxH(const Node* node, float & minH, float & maxH) const
{
	minH   = m_desc.MapDims.MinH + node->MinH * m_desc.MapDims.SizeH / 65535.0f;
	maxH   = m_desc.MapDims.MinH + node->MaxH * m_desc.MapDims.SizeH / 65535.0f;
}

void inline CDLODTerrainQuadTree::GetAABB( const SelectedNode& node,AABB & aabb) const
{
	const MapDimensions& mapDims = m_desc.MapDims;
	int rasterSizeX = m_rasterSizeX;
	int rasterSizeY = m_rasterSizeY;
	aabb.Min.x = mapDims.MinX + node.X * mapDims.SizeX / (float)(rasterSizeX-1);
	aabb.Max.x = mapDims.MinX + (node.X+node.Size) * mapDims.SizeX / (float)(rasterSizeX-1);
	aabb.Min.z = mapDims.MinY + node.Y * mapDims.SizeY / (float)(rasterSizeY-1);
	aabb.Max.z = mapDims.MinY + (node.Y+node.Size) * mapDims.SizeY / (float)(rasterSizeY-1);
	aabb.Min.y = mapDims.MinH + node.MinH * mapDims.SizeH / 65535.0f;
	aabb.Max.y = mapDims.MinH + node.MaxH * mapDims.SizeH / 65535.0f;
}

inline CDLODTerrainQuadTree::SelectedNode::SelectedNode( const Node * node, int LODLevel, bool tl, bool tr, bool bl, bool br )
: LODLevel(LODLevel), TL(tl), TR(tr), BL(bl), BR(br)
{
	this->X = node->X;
	this->Y = node->Y;
	this->Size = node->Size;
	this->MinH = node->MinH;
	this->MaxH = node->MaxH;
}
