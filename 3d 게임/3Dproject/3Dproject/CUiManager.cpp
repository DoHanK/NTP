#include "CUiManager.h"
#include "Mesh.h"
#include "Shader.h"	

CUiManager::CUiManager(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pShader = new CUIShader();
	m_pMesh.reserve(300);
	//CUiRectMesh* pCUIRectMesh;
	for (int i = 0; i < 300; i++) {

		m_pMesh.push_back(new CUiRectMesh(pd3dDevice, pd3dCommandList));
	}


}

//nomalize data
bool CUiManager::CreateNormalUIRect(float top, float bottom, float left, float right, CD3DX12_GPU_DESCRIPTOR_HANDLE* texaddr)
{
	// is not invalid 
	if (!(top > bottom && right > left))
		return false;

	UIRect* PRect = new UIRect{ top, bottom, left, right };
	RectList.push_back({ PRect,texaddr });
	
	//success
	return true;

}

//auto pixel coord
bool CUiManager::CreateUIRect(float top , float bottom , float left , float right , CD3DX12_GPU_DESCRIPTOR_HANDLE* texaddr)
{
	// is not invalid 
	if (!(top < bottom && right > left))
		return false;


	UIRect* PRect = new UIRect{ CreateNormalizePixel(top, bottom, left, right)};
	RectList.push_back({ PRect,texaddr });

	return false;

}


// Binding UI and Func
bool CUiManager::CreateUIRect_Func(float top, float bottom, float left, float right, int state, CD3DX12_GPU_DESCRIPTOR_HANDLE* texaddr, bool(*f)(void* argu))
{
	// is not invalid 
	if (!(top < bottom && right > left))
		return false;


	UIRect* PRect = new UIRect{ CreateNormalizePixel(top, bottom, left, right) };

	PRect->purpose = state;

	RectList.push_back({ PRect,texaddr });
	PRect = new UIRect{top, bottom, left, right,state};
	FunctionList.push_back({ PRect,f });

	return false;
}

UIRect CUiManager::CreateNormalizePixel(float top, float bottom, float left, float right)
{
	//y coord
	float NormalTop = 1 - ((top * 2) / FRAME_BUFFER_HEIGHT);
	float NormalBottom = 1 - ((bottom * 2) / FRAME_BUFFER_HEIGHT);
	//x coord
	float NormalLeft = -1+((left * 2) / FRAME_BUFFER_WIDTH);
	float NormalRight = -1+ ((right * 2) / FRAME_BUFFER_WIDTH);

	return { NormalTop, NormalBottom, NormalLeft, NormalRight };
}

void CUiManager::DeleteAllRect()
{

	//Delete Rect With Texture
	for (auto& p : RectList) {
		delete p.first;
	}
	RectList.clear();

	//Delete Rect With Function
	for (auto& p : FunctionList) {
		delete p.first;

	}
	FunctionList.clear();

}

void CUiManager::AlDrawRect(ID3D12GraphicsCommandList* pd3d12CommandList)
{
	//업로드힙메쉬의 갯수는 20개로 한정해놓음.
	int count = 0;

	if (m_pMesh.size()!=0) {
		for (pair<UIRect*, CD3DX12_GPU_DESCRIPTOR_HANDLE* >e : RectList) {
			
			m_pShader->OnPrepareRender(pd3d12CommandList, 1);
			pd3d12CommandList->SetGraphicsRootDescriptorTable(3,*e.second);

			m_pMesh[count]->UpdataVertexPosition((*e.first));
			

			m_pMesh[count++]->Render(pd3d12CommandList);
		
		}
	}


}



