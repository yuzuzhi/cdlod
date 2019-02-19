#pragma once


struct Light
{
	D3DXVECTOR3 mForward;
	D3DXVECTOR3 mRight;
	D3DXVECTOR3 mUp;
};

struct ShadowFitType
{
	class Id
	{
		Id(const Id&);
		ShadowFitType& operator = (const Id&);
	public:
		Id(){}
		inline bool operator ==(const Id& rhs)const{ return this == &rhs; }
	};
	virtual Id& GetId() const = 0;
};

#define __DeclShadowFitType static Id id;	\
	virtual Id& GetId() const { return id; }

#define __ImplShadowFitType(Type) ShadowFitType::Id Type::id;

struct ShadowFitArea : public ShadowFitType
{
	__DeclShadowFitType;
	D3DXVECTOR3 mMin;
	D3DXVECTOR3 mMax;
};
struct shadowfitCamera : public ShadowFitType
{
	__DeclShadowFitType;

	D3DXMATRIX	mCamView;
	D3DXMATRIX	mCamProj;
	float		mLevelRange;
	D3DXVECTOR3 mCamPos;
};

class LayerShadowMap
{
public:
	LayerShadowMap();
	~LayerShadowMap();

	void Update(const Light&, const ShadowFitType& type);
	D3DXVECTOR3 mObServrPos;
	float		mProjWidth;
	float		mProjHeight;
	float		mProjDepth;

	D3DXMATRIX  mView;
	D3DXMATRIX  mProj;


protected:
	template<class T>
	void CalculateViewProj(const Light& light, const T&args, D3DXMATRIX* view, D3DXMATRIX* proj);

};

