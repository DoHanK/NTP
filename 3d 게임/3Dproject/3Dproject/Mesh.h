#pragma once
#include "stdafx.h"
#include "Camera.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct UIRect {
	// -1 ~ 1 nomalize value
	float top;
	float bottom;
	float left;
	float right;
	int purpose;
};



static int ReadIntegerFromFile(FILE* pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile);
	return(nValue);
}

static float ReadFloatFromFile(FILE* pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
	return(fValue);
}


static BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

///

class CMesh
{
public:
	CMesh() { }
	virtual ~CMesh() { }

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual void ReleaseUploadBuffers() { }

protected:
	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nVertices = 0;
	UINT							m_nOffset = 0;
	UINT							m_nType = 0;
	UINT							m_nStride = 0;
	


public:
	UINT GetType() { return(m_nType); }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) { }


	//충돌 처리를 위한 함수와 변수


public:
	XMFLOAT3*						m_pVertices = NULL;
	UINT*							m_pnIndices = NULL;
	int								m_nIndices = 0;
	BoundingOrientedBox				m_xmBoundingBox;


	ID3D12Resource*					m_pd3dPositionBuffer;
	UINT							m_nVertexBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW*		m_pd3dVertexBufferViews;


	virtual int CheckRayIntersection(XMFLOAT3& xmRayPosition, XMFLOAT3& xmRayDirection, float* pfNearHitDistance) { return 0; };
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define VERTEXT_POSITION			0x01
#define VERTEXT_COLOR				0x02
#define VERTEXT_NORMAL				0x04

class CMeshLoadInfo
{
public:
	CMeshLoadInfo() { }
	~CMeshLoadInfo();

public:
	char							m_pstrMeshName[256] = { 0 };

	UINT							m_nType = 0x00;

	XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);

	int								m_nVertices = 0;
	XMFLOAT3*						m_pxmf3Positions = NULL;
	XMFLOAT4*						m_pxmf4Colors = NULL;
	XMFLOAT3*						m_pxmf3Normals = NULL;

	int								m_nIndices = 0;
	UINT*							m_pnIndices = NULL;

	int								m_nUves = 0;
	XMFLOAT2*						 m_pUves = NULL;

	int								m_nSubMeshes = 0;
	int*							m_pnSubSetIndices = NULL;
	UINT**							m_ppnSubSetIndices = NULL;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMeshFromFile : public CMesh
{
public:
	CMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo);
	virtual ~CMeshFromFile();

public:
	virtual void ReleaseUploadBuffers();

protected:
	ID3D12Resource* m_pd3dPositionBuffer = NULL;
	ID3D12Resource* m_pd3dPositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dPositionBufferView;

	int								m_nSubMeshes = 0;
	int* m_pnSubSetIndices = NULL;

	ID3D12Resource** m_ppd3dSubSetIndexBuffers = NULL;
	ID3D12Resource** m_ppd3dSubSetIndexUploadBuffers = NULL;
	D3D12_INDEX_BUFFER_VIEW* m_pd3dSubSetIndexBufferViews = NULL;



public:
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMeshIlluminatedFromFile : public CMeshFromFile
{
public:
	CMeshIlluminatedFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo);
	virtual ~CMeshIlluminatedFromFile();

	virtual void ReleaseUploadBuffers();

protected:
	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;
	ID3D12Resource* m_pd3dUvBuffer = NULL;
	ID3D12Resource* m_pd3dUvUploadBuffer = NULL;

	D3D12_VERTEX_BUFFER_VIEW				m_d3dUvBufferView;
	D3D12_VERTEX_BUFFER_VIEW				m_d3dNormalBufferView;

public:
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
	virtual int CheckRayIntersection(XMFLOAT3& xmRayPosition, XMFLOAT3& xmRayDirection, float* pfNearHitDistance);


};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CBoundingBoxMesh : public CMesh
{
public:
	CBoundingBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CBoundingBoxMesh();

	XMFLOAT3* m_pcbMappedPositions = NULL;

	void UpdateVertexPosition(BoundingOrientedBox* pxmBoundingBox);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
};





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//UI Mesh box 

class CUiRectMesh :public CMesh {
public:
	CUiRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~CUiRectMesh();

	XMFLOAT3* m_pcbMappedPositions = NULL;


	//텍스쳐 매핑을 위한
	ID3D12Resource*					m_pd3dUvBuffer;
	UINT							m_nUvBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW*		m_pd3dUvBufferViews;
	ID3D12Resource*			m_pd3dUvUploadBuffer = NULL;

	//텍스쳐 매핑을 위한
	ID3D12Resource* m_pd3dNormalBuffer;
	UINT							m_nNormalBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW*	m_pd3dNormalBufferViews;
	ID3D12Resource*			m_pd3dNormalUploadBuffer = NULL;
	void UpdataVertexPosition(UIRect Rect);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 
//Billboard Mesh box 

class CBillboardMesh :public CMesh {
public:
	CBillboardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~CBillboardMesh();

	XMFLOAT3* m_pcbMappedPositions = NULL;


	//텍스쳐 매핑을 위한
	ID3D12Resource* m_pd3dUvBuffer;
	UINT							m_nUvBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW* m_pd3dUvBufferViews;
	ID3D12Resource* m_pd3dUvUploadBuffer = NULL;
	XMFLOAT2* m_pcbMappedUvs = NULL;

	//텍스쳐 매핑을 위한
	ID3D12Resource* m_pd3dNormalBuffer;
	UINT							m_nNormalBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW* m_pd3dNormalBufferViews;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;


	void UpdataVertexPosition(UIRect Rect,float z);
	void UpdateUvCoord(UIRect Rect);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

};












////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

class CShader;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

struct MATERIALLOADINFO
{
	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	UINT							m_nType = 0x00;

	//char							m_pstrAlbedoMapName[64] = { '\0' };
	//char							m_pstrSpecularMapName[64] = { '\0' };
	//char							m_pstrMetallicMapName[64] = { '\0' };
	//char							m_pstrNormalMapName[64] = { '\0' };
	//char							m_pstrEmissionMapName[64] = { '\0' };
	//char							m_pstrDetailAlbedoMapName[64] = { '\0' };
	//char							m_pstrDetailNormalMapName[64] = { '\0' };
};

struct MATERIALSLOADINFO
{
	int								m_nMaterials = 0;
	MATERIALLOADINFO* m_pMaterials = NULL;
};

class CMaterialColors
{
public:
	CMaterialColors() { }
	CMaterialColors(MATERIALLOADINFO* pMaterialInfo);
	virtual ~CMaterialColors() { }

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	XMFLOAT4						m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	XMFLOAT4						m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); //(r,g,b,a=power)
	XMFLOAT4						m_xmf4Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	CShader* m_pShader = NULL;

	CMaterialColors* m_pMaterialColors = NULL;

	void SetMaterialColors(CMaterialColors* pMaterialColors);
	void SetShader(CShader* pShader);
	void SetIlluminatedShader() { SetShader(m_pIlluminatedShader); }

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);

protected:
	static CShader* m_pIlluminatedShader;

public:

	static CShader* PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CSumMesh
{
private:
	int								m_nReferences = 0;

public:
	void AddRef();
	void Release();

public:
	CSumMesh();
	virtual ~CSumMesh();
	
public:
	char							m_pstrFrameName[64];

	CMesh* m_pMesh = NULL;

	int								m_nMaterials = 0;
	CMaterial** m_ppMaterials = NULL;


	XMFLOAT4X4						m_xmf4x4Transform;
	XMFLOAT4X4						m_xmf4x4World;

	CSumMesh* m_pParent = NULL;
	CSumMesh* m_pChild = NULL;
	CSumMesh* m_pSibling = NULL;

	CBoundingBoxMesh* m_pBoundingBoxMesh = NULL;


	void SetMesh(CMesh* pMesh);
	void SetShader(CShader* pShader);
	void SetShader(int nMaterial, CShader* pShader);
	void SetMaterial(int nMaterial, CMaterial* pMaterial);

	void SetChild(CSumMesh* pChild, bool bReferenceUpdate = false);

	virtual void BuildMaterials(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }

	virtual void OnInitialize() { }
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);

	virtual void OnPrepareRender() { }


	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial);

	virtual void ReleaseUploadBuffers();

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	CSumMesh* GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent  , vector<pair<string, XMFLOAT4X4>> T,int &num);
	CSumMesh* FindFrame(char* pstrFrameName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0); }

public:
	static MATERIALSLOADINFO* LoadMaterialsInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);
	static CMeshLoadInfo* LoadMeshInfoFromFile(FILE* pInFile);

	static CSumMesh* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, FILE* pInFile);
	static CSumMesh* LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName);

	static void PrintFrameInfo(CSumMesh* pGameObject, CSumMesh* pParent);

public:
	void CopyTransform(vector<pair<string, XMFLOAT4X4>> &T);
	void GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3* pxmf3PickRayOrigin, XMFLOAT3* pxmf3PickRayDirection);
	int CheckRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3& xmRayPosition, XMFLOAT3& xmRayDirection, float* pfNearHitDistance );
};




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//