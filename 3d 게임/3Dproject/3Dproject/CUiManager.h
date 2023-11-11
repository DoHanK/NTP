#pragma once
#include "stdafx.h"
#include "Mesh.h"
#include "Shader.h"
#include "CMeshManager.h"




// this hararchy is equal at Framwork
class CUiManager
{
public:

	vector<pair<UIRect*, CD3DX12_GPU_DESCRIPTOR_HANDLE*>> RectList;
	list<pair<UIRect*, bool(*)(void*)>> FunctionList;
	vector<CUiRectMesh*> m_pMesh;

	CUIShader* m_pShader = NULL;


	//Function
	CUiManager(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~CUiManager() {};
	bool CreateNormalUIRect(float top, float bottom ,float left, float right, CD3DX12_GPU_DESCRIPTOR_HANDLE* texaddr);
	bool CreateUIRect(float top, float bottom, float left, float right, CD3DX12_GPU_DESCRIPTOR_HANDLE* texaddr);
	bool CreateUIRect_Func(float top, float bottom, float left, float right,int state ,CD3DX12_GPU_DESCRIPTOR_HANDLE* texaddr, bool(*f)(void* argu));
	void AlDrawRect(ID3D12GraphicsCommandList* pd3d12CommandList);
	UIRect CreateNormalizePixel(float top, float bottom, float left, float right);
	void DeleteAllRect();

};

