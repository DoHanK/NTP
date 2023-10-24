//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Mesh.h"
#include "Camera.h"
#include "Shader.h"
/////////////////////////////////////////////////////////////////////////////////////////////////
//



CMeshLoadInfo::~CMeshLoadInfo()
{
	if (m_pxmf3Positions) delete[] m_pxmf3Positions;
	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;

	if (m_pnIndices) delete[] m_pnIndices;

	if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;

	for (int i = 0; i < m_nSubMeshes; i++) if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
	if (m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CMeshFromFile::CMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo)
{
	m_nVertices = pMeshInfo->m_nVertices;
	m_nType = pMeshInfo->m_nType;

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pMeshInfo->m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_nSubMeshes = pMeshInfo->m_nSubMeshes;
	if (m_nSubMeshes > 0)
	{
		m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
		m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];
		m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

		m_pnSubSetIndices = new int[m_nSubMeshes];

		for (int i = 0; i < m_nSubMeshes; i++)
		{
			m_pnSubSetIndices[i] = pMeshInfo->m_pnSubSetIndices[i];
			m_ppd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pMeshInfo->m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[i]);

			m_pd3dSubSetIndexBufferViews[i].BufferLocation = m_ppd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
			m_pd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
			m_pd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * pMeshInfo->m_pnSubSetIndices[i];
		}
	}
}

CMeshFromFile::~CMeshFromFile()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();

	if (m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexBuffers[i]) m_ppd3dSubSetIndexBuffers[i]->Release();
		}
		if (m_ppd3dSubSetIndexBuffers) delete[] m_ppd3dSubSetIndexBuffers;
		if (m_pd3dSubSetIndexBufferViews) delete[] m_pd3dSubSetIndexBufferViews;

		if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
	}
}

void CMeshFromFile::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;

	if ((m_nSubMeshes > 0) && m_ppd3dSubSetIndexUploadBuffers)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexUploadBuffers[i]) m_ppd3dSubSetIndexUploadBuffers[i]->Release();
		}
		if (m_ppd3dSubSetIndexUploadBuffers) delete[] m_ppd3dSubSetIndexUploadBuffers;
		m_ppd3dSubSetIndexUploadBuffers = NULL;
	}
}

void CMeshFromFile::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//

CMeshIlluminatedFromFile::CMeshIlluminatedFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo) : CMeshFromFile::CMeshFromFile(pd3dDevice, pd3dCommandList, pMeshInfo)
{
	m_xmBoundingBox = BoundingOrientedBox(pMeshInfo->m_xmf3AABBCenter, pMeshInfo->m_xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_pVertices = pMeshInfo->m_pxmf3Positions;
	m_pnIndices = pMeshInfo->m_pnIndices;
	m_nIndices = pMeshInfo->m_nIndices;

	m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pMeshInfo->m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

	m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
	
	m_pd3dUvBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pMeshInfo->m_pUves, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dUvUploadBuffer);

	m_d3dUvBufferView.BufferLocation = m_pd3dUvBuffer->GetGPUVirtualAddress();
	m_d3dUvBufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dUvBufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

}

CMeshIlluminatedFromFile::~CMeshIlluminatedFromFile()
{
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
}

void CMeshIlluminatedFromFile::ReleaseUploadBuffers()
{
	CMeshFromFile::ReleaseUploadBuffers();

	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;
}

void CMeshIlluminatedFromFile::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[3] = { m_d3dPositionBufferView, m_d3dNormalBufferView , m_d3dUvBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, pVertexBufferViews);

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[0]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[0], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}


int CMeshIlluminatedFromFile::CheckRayIntersection(XMFLOAT3& xmf3RayOrigin, XMFLOAT3& xmf3RayDirection, float* pfNearHitDistance)
{
	int m_nStride = sizeof(XMFLOAT3);

	//하나의 메쉬에서 광선은 여러 개의 삼각형과 교차할 수 있다. 교차하는 삼각형들 중 가장 가까운 삼각형을 찾는다. 

	int nIntersections = 0;
	XMFLOAT3* pbPositions = m_pVertices;
	int nOffset = (m_d3dPrimitiveTopology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? 3 : 1;
	/*메쉬의 프리미티브(삼각형)들의 개수이다. 삼각형 리스트인 경우 (정점의 개수 / 3) 또는 (인덱스의 개수 / 3), 삼각
	형 스트립의 경우 (정점의 개수 - 2) 또는 (인덱스의 개수 – 2)이다.*/
	int nPrimitives = (m_d3dPrimitiveTopology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? (m_nVertices / 3) : (m_nVertices - 2);
	if (m_nIndices > 0) nPrimitives = (m_d3dPrimitiveTopology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? (m_nIndices / 3) : (m_nIndices - 2);

	//광선은 모델 좌표계로 표현된다.

	XMVECTOR xmRayOrigin = XMLoadFloat3(&xmf3RayOrigin);
	XMVECTOR xmRayDirection = XMLoadFloat3(&xmf3RayDirection);

	//모델 좌표계의 광선과 메쉬의 바운딩 박스(모델 좌표계)와의 교차를 검사한다.
	float check = 0;
	bool bIntersected = false;
	if (this)
		bIntersected = this->m_xmBoundingBox.Intersects(xmRayOrigin, xmRayDirection, check);

	//모델 좌표계의 광선이 메쉬의 바운딩 박스와 교차하면 메쉬와의 교차를 검사한다. 

	if (bIntersected)
	{
		float fNearHitDistance = FLT_MAX;

		/*메쉬의 모든 프리미티브(삼각형)들에 대하여 픽킹 광선과의 충돌을 검사한다. 충돌하는 모든 삼각형을 찾아 광선의
		시작점(실제로는 카메라 좌표계의 원점)에 가장 가까운 삼각형을 찾는다.*/

		for (int i = 0; i < nPrimitives; i++)
		{

			XMVECTOR v0 = XMLoadFloat3(&pbPositions[3 * i]);
			XMVECTOR v1 = XMLoadFloat3(&pbPositions[3 * i + 1]);
			XMVECTOR v2 = XMLoadFloat3(&pbPositions[3 * i + 2]);

			float fHitDistance;

			BOOL bIntersected = TriangleTests::Intersects(xmRayOrigin, xmRayDirection, v0, v1, v2, fHitDistance);


			if (bIntersected)
			{
				if (fHitDistance < *pfNearHitDistance)
				{
					*pfNearHitDistance = fNearHitDistance = fHitDistance;
				}
				nIntersections++;
			}
		}
	}

	return(nIntersections);


}
////////////////////////////////////////////////////////////////////////////////////////////////
//

////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterialColors::CMaterialColors(MATERIALLOADINFO* pMaterialInfo)
{
	m_xmf4Diffuse = pMaterialInfo->m_xmf4AlbedoColor;
	m_xmf4Specular = pMaterialInfo->m_xmf4SpecularColor; //(r,g,b,a=power)
	m_xmf4Specular.w = (pMaterialInfo->m_fGlossiness * 255.0f);
	m_xmf4Emissive = pMaterialInfo->m_xmf4EmissiveColor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CShader* CMaterial::m_pIlluminatedShader = NULL;

CMaterial::CMaterial()
{
}

CMaterial::~CMaterial()
{
	if (m_pShader) m_pShader->Release();
	if (m_pMaterialColors) m_pMaterialColors->Release();
}

void CMaterial::SetShader(CShader* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CMaterial::SetMaterialColors(CMaterialColors* pMaterialColors)
{
	if (m_pMaterialColors) m_pMaterialColors->Release();
	m_pMaterialColors = pMaterialColors;
	if (m_pMaterialColors) m_pMaterialColors->AddRef();
}

void CMaterial::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &(m_pMaterialColors->m_xmf4Ambient), 16);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &(m_pMaterialColors->m_xmf4Diffuse), 20);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &(m_pMaterialColors->m_xmf4Specular), 24);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &(m_pMaterialColors->m_xmf4Emissive), 28);
}

CShader* CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_pIlluminatedShader = new CIlluminatedShader();
	m_pIlluminatedShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pIlluminatedShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	return m_pIlluminatedShader;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSumMesh::CSumMesh()
{
	m_xmf4x4Transform = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
}

CSumMesh::~CSumMesh()
{
	if (m_pMesh) m_pMesh->Release();

	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		}
	}
	if (m_ppMaterials) delete[] m_ppMaterials;

}

void CSumMesh::AddRef()
{
	m_nReferences++;

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}

void CSumMesh::Release()
{
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();

	if (--m_nReferences <= 0) delete this;
}

void CSumMesh::SetChild(CSumMesh* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}

void CSumMesh::SetMesh(CMesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CSumMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			//if (m_ppMaterials[i])
			//{
			//	//if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
			//	//m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);
			//}

		}
	}
			if (m_pMesh) m_pMesh->Render(pd3dCommandList, 0);

	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera);

}

void CSumMesh::RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{

	if (m_pMesh) {
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
		m_pBoundingBoxMesh->UpdateVertexPosition(&m_pMesh->m_xmBoundingBox);
		m_pBoundingBoxMesh->Render(pd3dCommandList);
	}
	
	if (m_pSibling)
		m_pSibling->RenderBoundingBox(pd3dCommandList, pCamera);
	if (m_pChild)
		m_pChild->RenderBoundingBox(pd3dCommandList, pCamera);

}

void CSumMesh::SetShader(CShader* pShader)
{
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial * [m_nMaterials];
	m_ppMaterials[0] = new CMaterial();
	m_ppMaterials[0]->SetShader(pShader);
}

void CSumMesh::SetShader(int nMaterial, CShader* pShader)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->SetShader(pShader);
}

void CSumMesh::SetMaterial(int nMaterial, CMaterial* pMaterial)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->Release();
	m_ppMaterials[nMaterial] = pMaterial;
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->AddRef();
}

void CSumMesh::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	if (m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
	if (m_pChild) m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);
}

CSumMesh* CSumMesh::FindFrame(char* pstrFrameName)
{
	CSumMesh* pFrameObject = NULL;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

void CSumMesh::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CSumMesh::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CSumMesh::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
}

void CSumMesh::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial)
{
}

void CSumMesh::ReleaseShaderVariables()
{
}

void CSumMesh::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();

	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void CSumMesh::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Transform, *pxmf4x4Parent) : m_xmf4x4Transform;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void CSumMesh::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent, vector< pair<string, XMFLOAT4X4>> T,int &num)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(T[num].second, *pxmf4x4Parent) :T[num].second;
	num++;
	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent, T , num);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World,T,num);
}

void CSumMesh::SetPosition(float x, float y, float z)
{
	m_xmf4x4Transform._41 = x;
	m_xmf4x4Transform._42 = y;
	m_xmf4x4Transform._43 = z;

	UpdateTransform(NULL);
}

void CSumMesh::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CSumMesh::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxScale, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

XMFLOAT3 CSumMesh::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CSumMesh::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CSumMesh::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CSumMesh::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CSumMesh::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CSumMesh::SetPosition(xmf3Position);
}

void CSumMesh::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CSumMesh::SetPosition(xmf3Position);
}

void CSumMesh::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CSumMesh::SetPosition(xmf3Position);
}

void CSumMesh::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void CSumMesh::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void CSumMesh::Rotate(XMFLOAT4* pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


#define _WITH_DEBUG_FRAME_HIERARCHY

CMeshLoadInfo* CSumMesh::LoadMeshInfoFromFile(FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nPositions = 0, nColors = 0, nNormals = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0 , nUvs = 0;

	CMeshLoadInfo* pMeshInfo = new CMeshLoadInfo;

	pMeshInfo->m_nVertices = ::ReadIntegerFromFile(pInFile);
	::ReadStringFromFile(pInFile, pMeshInfo->m_pstrMeshName);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&(pMeshInfo->m_xmf3AABBCenter), sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&(pMeshInfo->m_xmf3AABBExtents), sizeof(XMFLOAT3), 1, pInFile);

		}
		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			nPositions = ::ReadIntegerFromFile(pInFile);
			if (nPositions > 0)
			{
				pMeshInfo->m_nType |= VERTEXT_POSITION;
				pMeshInfo->m_pxmf3Positions = new XMFLOAT3[nPositions];
				nReads = (UINT)::fread(pMeshInfo->m_pxmf3Positions, sizeof(XMFLOAT3), nPositions, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<Colors>:"))
		{
			nColors = ::ReadIntegerFromFile(pInFile);
			if (nColors > 0)
			{
				pMeshInfo->m_nType |= VERTEXT_COLOR;
				pMeshInfo->m_pxmf4Colors = new XMFLOAT4[nColors];
				nReads = (UINT)::fread(pMeshInfo->m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nNormals = ::ReadIntegerFromFile(pInFile);
			if (nNormals > 0)
			{
				pMeshInfo->m_nType |= VERTEXT_NORMAL;
				pMeshInfo->m_pxmf3Normals = new XMFLOAT3[nNormals];
				nReads = (UINT)::fread(pMeshInfo->m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<UVs>:"))
		{
			nUvs = ::ReadIntegerFromFile(pInFile);
			if (nUvs > 0)
			{
				pMeshInfo->m_nUves = nUvs;
				pMeshInfo->m_pUves = new XMFLOAT2[nUvs];
				nReads = (UINT)::fread(pMeshInfo->m_pUves, sizeof(XMFLOAT2), nUvs, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<Indices>:"))
		{
			nIndices = ::ReadIntegerFromFile(pInFile);
			if (nIndices > 0)
			{
				pMeshInfo->m_pnIndices = new UINT[nIndices];
				nReads = (UINT)::fread(pMeshInfo->m_pnIndices, sizeof(int), nIndices, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			pMeshInfo->m_nSubMeshes = ::ReadIntegerFromFile(pInFile);
			if (pMeshInfo->m_nSubMeshes > 0)
			{
				pMeshInfo->m_pnSubSetIndices = new int[pMeshInfo->m_nSubMeshes];
				pMeshInfo->m_ppnSubSetIndices = new UINT * [pMeshInfo->m_nSubMeshes];
				for (int i = 0; i < pMeshInfo->m_nSubMeshes; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						int nIndex = ::ReadIntegerFromFile(pInFile);
						pMeshInfo->m_pnSubSetIndices[i] = ::ReadIntegerFromFile(pInFile);
						if (pMeshInfo->m_pnSubSetIndices[i] > 0)
						{
							pMeshInfo->m_ppnSubSetIndices[i] = new UINT[pMeshInfo->m_pnSubSetIndices[i]];
							nReads = (UINT)::fread(pMeshInfo->m_ppnSubSetIndices[i], sizeof(int), pMeshInfo->m_pnSubSetIndices[i], pInFile);
						}

					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
	return(pMeshInfo);
}

MATERIALSLOADINFO* CSumMesh::LoadMaterialsInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nMaterial = 0;

	MATERIALSLOADINFO* pMaterialsInfo = new MATERIALSLOADINFO;

	pMaterialsInfo->m_nMaterials = ::ReadIntegerFromFile(pInFile);
	pMaterialsInfo->m_pMaterials = new MATERIALLOADINFO[pMaterialsInfo->m_nMaterials];

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ::ReadIntegerFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4SpecularColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<Glossiness>:"))
		{
			nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_fGlossiness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Smoothness>:"))
		{
			nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_fSmoothness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Metallic>:"))
		{
			nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_fSpecularHighlight), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
		{
			nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_fMetallic), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<GlossyReflection>:"))
		{
			nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_fGlossyReflection), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			break;
		}
	}
	return(pMaterialsInfo);
}

CSumMesh* CSumMesh::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0;

	CSumMesh* pSumMesh = NULL;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			pSumMesh = new CSumMesh();

			nFrame = ::ReadIntegerFromFile(pInFile);
			::ReadStringFromFile(pInFile, pSumMesh->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pSumMesh->m_xmf4x4Transform, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			CMeshLoadInfo* pMeshInfo = pSumMesh->LoadMeshInfoFromFile(pInFile);
			if (pMeshInfo)
			{
				CMesh* pMesh = NULL;
				if (pMeshInfo->m_nType & VERTEXT_NORMAL)
				{
					pMesh = new CMeshIlluminatedFromFile(pd3dDevice, pd3dCommandList, pMeshInfo);

				}
				if (pMesh) pSumMesh->SetMesh(pMesh);
				//피킹을 위해서 해당 정보는 유지.
				//delete pMeshInfo;
			}
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			MATERIALSLOADINFO* pMaterialsInfo = pSumMesh->LoadMaterialsInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
			if (pMaterialsInfo && (pMaterialsInfo->m_nMaterials > 0))
			{
				pSumMesh->m_nMaterials = pMaterialsInfo->m_nMaterials;
				pSumMesh->m_ppMaterials = new CMaterial * [pMaterialsInfo->m_nMaterials];

				for (int i = 0; i < pMaterialsInfo->m_nMaterials; i++)
				{
					pSumMesh->m_ppMaterials[i] = NULL;

					CMaterial* pMaterial = new CMaterial();

					CMaterialColors* pMaterialColors = new CMaterialColors(&pMaterialsInfo->m_pMaterials[i]);
					pMaterial->SetMaterialColors(pMaterialColors);

					if (pSumMesh->GetMeshType() & VERTEXT_NORMAL) pMaterial->SetIlluminatedShader();

					pSumMesh->SetMaterial(i, pMaterial);
				}
			}
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					CSumMesh* pChild = CSumMesh::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pInFile);
					if (pChild) pSumMesh->SetChild(pChild);

#ifdef _WITH_DEBUG_RUNTIME_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, _T("(Child Frame: %p) (Parent Frame: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}

	if (pSumMesh) {
		CBoundingBoxMesh* pBoundingBoxMesh = new CBoundingBoxMesh(pd3dDevice, pd3dCommandList);
		pSumMesh->m_pBoundingBoxMesh = pBoundingBoxMesh;
	}
	return(pSumMesh);
}

void CSumMesh::PrintFrameInfo(CSumMesh* pGameObject, CSumMesh* pParent)
{
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) CSumMesh::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) CSumMesh::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}

CSumMesh* CSumMesh::LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CSumMesh* pSumMesh = NULL;
	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Hierarchy>:"))
		{
			pSumMesh = CSumMesh::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pInFile);
		}
		else if (!strcmp(pstrToken, "</Hierarchy>"))
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CSumMesh::PrintFrameInfo(pSumMesh, NULL);
#endif

	return(pSumMesh);

}

void CSumMesh::CopyTransform(vector<pair<string, XMFLOAT4X4>> &T)
{
	T.push_back({ string(m_pstrFrameName), m_xmf4x4Transform});

	if (m_pSibling) m_pSibling->CopyTransform(T);
	if (m_pChild) m_pChild->CopyTransform(T);

}

void CSumMesh::GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3* pxmf3PickRayOrigin, XMFLOAT3* pxmf3PickRayDirection)
{
	XMFLOAT4X4 xmf4x4WorldView = Matrix4x4::Multiply(m_xmf4x4World, xmf4x4View);
	XMFLOAT4X4 xmf4x4Inverse = Matrix4x4::Inverse(xmf4x4WorldView);
	XMFLOAT3 xmf3CameraOrigin(0.0f, 0.0f, 0.0f);
	//카메라 좌표계의 원점을 모델 좌표계로 변환한다. 
	*pxmf3PickRayOrigin = Vector3::TransformCoord(xmf3CameraOrigin, xmf4x4Inverse);
	//카메라 좌표계의 점(마우스 좌표를 역변환하여 구한 점)을 모델 좌표계로 변환한다
	*pxmf3PickRayDirection = Vector3::TransformCoord(xmf3PickPosition, xmf4x4Inverse);
	//광선의 방향 벡터를 구한다
	*pxmf3PickRayDirection = Vector3::Normalize(Vector3::Subtract(*pxmf3PickRayDirection, *pxmf3PickRayOrigin));
}

int CSumMesh::CheckRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3& xmRayPosition, XMFLOAT3& xmRayDirection, float* pfNearHitDistance) {

	int nIntersections = 0;

	if (m_pMesh) {

		XMFLOAT3 xmf3PickRayOrigin, xmf3PickRayDirection;
		GenerateRayForPicking(xmf3PickPosition, xmf4x4View, &xmf3PickRayOrigin, &xmf3PickRayDirection);
		nIntersections += ((CMeshIlluminatedFromFile*)m_pMesh)->CheckRayIntersection(xmf3PickRayOrigin, xmf3PickRayDirection, pfNearHitDistance);

	}

	if (m_pSibling)
		m_pSibling->CheckRayIntersection(xmf3PickPosition, xmf4x4View, xmRayPosition, xmRayDirection, pfNearHitDistance);
	if (m_pChild)
		m_pChild->CheckRayIntersection(xmf3PickPosition, xmf4x4View, xmRayPosition, xmRayDirection, pfNearHitDistance);

	return nIntersections;

}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CBoundingBoxMesh::CBoundingBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh()
{
	m_nVertices = 12 * 2;
	m_nStride = sizeof(XMFLOAT3);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, m_nStride * m_nVertices, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dPositionBuffer->Map(0, NULL, (void**)&m_pcbMappedPositions);

	m_nVertexBufferViews = 1;
	m_pd3dVertexBufferViews = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferViews];

	m_pd3dVertexBufferViews[0].BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[0].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

CBoundingBoxMesh::~CBoundingBoxMesh()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Unmap(0, NULL);
}

void CBoundingBoxMesh::UpdateVertexPosition(BoundingOrientedBox* pxmBoundingBox)
{
	XMFLOAT3 xmf3Corners[8];
	pxmBoundingBox->GetCorners(xmf3Corners);

	int i = 0;

	m_pcbMappedPositions[i++] = xmf3Corners[0];
	m_pcbMappedPositions[i++] = xmf3Corners[1];

	m_pcbMappedPositions[i++] = xmf3Corners[1];
	m_pcbMappedPositions[i++] = xmf3Corners[2];

	m_pcbMappedPositions[i++] = xmf3Corners[2];
	m_pcbMappedPositions[i++] = xmf3Corners[3];

	m_pcbMappedPositions[i++] = xmf3Corners[3];
	m_pcbMappedPositions[i++] = xmf3Corners[0];

	m_pcbMappedPositions[i++] = xmf3Corners[4];
	m_pcbMappedPositions[i++] = xmf3Corners[5];

	m_pcbMappedPositions[i++] = xmf3Corners[5];
	m_pcbMappedPositions[i++] = xmf3Corners[6];

	m_pcbMappedPositions[i++] = xmf3Corners[6];
	m_pcbMappedPositions[i++] = xmf3Corners[7];

	m_pcbMappedPositions[i++] = xmf3Corners[7];
	m_pcbMappedPositions[i++] = xmf3Corners[4];

	m_pcbMappedPositions[i++] = xmf3Corners[0];
	m_pcbMappedPositions[i++] = xmf3Corners[4];

	m_pcbMappedPositions[i++] = xmf3Corners[1];
	m_pcbMappedPositions[i++] = xmf3Corners[5];

	m_pcbMappedPositions[i++] = xmf3Corners[2];
	m_pcbMappedPositions[i++] = xmf3Corners[6];

	m_pcbMappedPositions[i++] = xmf3Corners[3];
	m_pcbMappedPositions[i++] = xmf3Corners[7];
}

void CBoundingBoxMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, m_nVertexBufferViews, m_pd3dVertexBufferViews);

	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
CUiRectMesh::CUiRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_nVertices = 6;
	m_nStride = sizeof(XMFLOAT3);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, m_nStride * m_nVertices, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dPositionBuffer->Map(0, NULL, (void**)&m_pcbMappedPositions);

	m_nVertexBufferViews = 1;
	m_pd3dVertexBufferViews = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferViews];

	m_pd3dVertexBufferViews[0].BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[0].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	//XMFLOAT2* temp = new XMFLOAT2[4];
	//// left top
	//temp[0].x = 0; temp[0].y = 0;
	////right top
	//temp[1].x = 1; temp[1].y = 0;
	////left bottom
	//temp[2].x = 0;	temp[2].y = 1;
	////right bottom 
	//temp[4].x = 1;	temp[4].y = 1;



	//after code
	XMFLOAT2* temp = new XMFLOAT2[6];
	// left top
	temp[0].x = 0.0f; temp[0].y = 0.0f;
	//right top
	temp[1].x = 1.0f; temp[1].y = 0.0f;
	//left bottom
	temp[2].x = 0.0f;	temp[2].y = 1.0f;


	//left bottom
	temp[3].x = 0.0f;	temp[3].y = 1.0f;
	//right top
	temp[4].x = 1.0f;	temp[4].y = 0.0f;
	//right bottom 
	temp[5].x = 1.0f;	temp[5].y = 1.0f;


	m_pd3dUvBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, temp, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dUvUploadBuffer);
	m_pd3dUvBufferViews = new D3D12_VERTEX_BUFFER_VIEW[1];

	m_pd3dUvBufferViews[0].BufferLocation = m_pd3dUvBuffer->GetGPUVirtualAddress();
	m_pd3dUvBufferViews[0].StrideInBytes = sizeof(XMFLOAT2);
	m_pd3dUvBufferViews[0].SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
	

	XMFLOAT3*  Normal = new XMFLOAT3[m_nVertices];
	Normal[0] = XMFLOAT3(0.0f, 0.0f, 1.0f);
	Normal[1] = XMFLOAT3(0.0f, 0.0f, 1.0f);
	Normal[2] = XMFLOAT3(0.0f, 0.0f, 1.0f);


	Normal[3] = XMFLOAT3(0.0f, 0.0f, 1.0f);
	Normal[4] = XMFLOAT3(0.0f, 0.0f, 1.0f);
	Normal[5] = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_pd3dNormalBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, Normal, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

	m_pd3dNormalBufferViews = new D3D12_VERTEX_BUFFER_VIEW[1];

	m_pd3dNormalBufferViews[0].BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_pd3dNormalBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dNormalBufferViews[0].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;


}

CUiRectMesh::~CUiRectMesh()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Unmap(0, NULL);
}

void CUiRectMesh::UpdataVertexPosition(UIRect Rect)
{
	int i = 0;


	//LEFT TOP 
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.left,Rect.top,0.5f);
	// RIGHT TOP
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.right, Rect.top, 0.5f);
	// LEFT BOTTOM
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.left, Rect.bottom, 0.5f);


	// LEFT BOTTOM
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.left, Rect.bottom, 0.5f);
	// RIGHT TOP
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.right, Rect.top, 0.5f);
	// RIGHT BOTTOM
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.right, Rect.bottom, 0.5f);



}


void CUiRectMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{


	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[3] = { *m_pd3dVertexBufferViews,*m_pd3dNormalBufferViews,*m_pd3dUvBufferViews  };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, pVertexBufferViews);

	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);

}

CBillboardMesh::CBillboardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_nVertices = 6;
	m_nStride = sizeof(XMFLOAT3);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, m_nStride * m_nVertices, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dPositionBuffer->Map(0, NULL, (void**)&m_pcbMappedPositions);

	m_nVertexBufferViews = 1;
	m_pd3dVertexBufferViews = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferViews];

	m_pd3dVertexBufferViews[0].BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[0].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;




	m_pd3dUvBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dUvBuffer->Map(0, NULL, (void**)&m_pcbMappedUvs);
	m_pd3dUvBufferViews = new D3D12_VERTEX_BUFFER_VIEW[1];

	m_pd3dUvBufferViews[0].BufferLocation = m_pd3dUvBuffer->GetGPUVirtualAddress();
	m_pd3dUvBufferViews[0].StrideInBytes = sizeof(XMFLOAT2);
	m_pd3dUvBufferViews[0].SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;


	// left top
	m_pcbMappedUvs[0].x = 0.0f; m_pcbMappedUvs[0].y = 0.0f;
	//right top
	m_pcbMappedUvs[1].x = 1.0f; m_pcbMappedUvs[1].y = 0.0f;
	//left bottom
	m_pcbMappedUvs[2].x = 0.0f;	m_pcbMappedUvs[2].y = 1.0f;


	//left bottom
	m_pcbMappedUvs[3].x = 0.0f;	m_pcbMappedUvs[3].y = 1.0f;
	//right top
	m_pcbMappedUvs[4].x = 1.0f;	m_pcbMappedUvs[4].y = 0.0f;
	//right bottom 
	m_pcbMappedUvs[5].x = 1.0f;	m_pcbMappedUvs[5].y = 1.0f;



	XMFLOAT3* Normal = new XMFLOAT3[m_nVertices];
	Normal[0] = XMFLOAT3(0.0f, 0.0f, -1.0f);
	Normal[1] = XMFLOAT3(0.0f, 0.0f, -1.0f);
	Normal[2] = XMFLOAT3(0.0f, 0.0f, -1.0f);


	Normal[3] = XMFLOAT3(0.0f, 0.0f, -1.0f);
	Normal[4] = XMFLOAT3(0.0f, 0.0f, -1.0f);
	Normal[5] = XMFLOAT3(0.0f, 0.0f, -1.0f);

	m_pd3dNormalBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, Normal, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

	m_pd3dNormalBufferViews = new D3D12_VERTEX_BUFFER_VIEW[1];

	m_pd3dNormalBufferViews[0].BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_pd3dNormalBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dNormalBufferViews[0].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

CBillboardMesh::~CBillboardMesh()
{

}

void CBillboardMesh::UpdataVertexPosition(UIRect Rect,float z)
{
	int i = 0;


	//LEFT TOP 
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.left, Rect.top, z);
	// RIGHT TOP
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.right, Rect.top, z);
	// LEFT BOTTOM
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.left, Rect.bottom, z);

	// LEFT BOTTOM
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.left, Rect.bottom, z);
	// RIGHT TOP
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.right, Rect.top, z);
	// RIGHT BOTTOM
	m_pcbMappedPositions[i++] = XMFLOAT3(Rect.right, Rect.bottom, z);


}

void CBillboardMesh::UpdateUvCoord(UIRect Rect)
{
	// left top
	m_pcbMappedUvs[0].x = Rect.left; m_pcbMappedUvs[0].y = Rect.top;
	//right top
	m_pcbMappedUvs[1].x = Rect.right; m_pcbMappedUvs[1].y = Rect.top;
	//left bottom
	m_pcbMappedUvs[2].x = Rect.left; m_pcbMappedUvs[2].y = Rect.bottom;


	////left bottom
	m_pcbMappedUvs[3].x = Rect.left;  m_pcbMappedUvs[3].y = Rect.bottom;
	////right top
	m_pcbMappedUvs[4].x = Rect.right;	m_pcbMappedUvs[4].y = Rect.top;
	////right bottom 
	m_pcbMappedUvs[5].x = Rect.right;	m_pcbMappedUvs[5].y = Rect.bottom;


}

void CBillboardMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{


	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[3] = { *m_pd3dVertexBufferViews,*m_pd3dNormalBufferViews,*m_pd3dUvBufferViews };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, pVertexBufferViews);

	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}
