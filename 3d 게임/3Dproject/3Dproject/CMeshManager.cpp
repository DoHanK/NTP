#include "stdafx.h"
#include "Mesh.h"
#include "CMeshManager.h"
#include "DDSTextureLoader.h"

CMeshManager::CMeshManager(ID3D12Device* pd3dDevice) {

	//MaxTecture
	CrateSrvDescriptorHeap(pd3dDevice, 100);
	CreateSampleDescriptorHeap(pd3dDevice);

	hGDescriptor = pSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	hCDescriptor = pSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	size = pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void CMeshManager::CreateSampleDescriptorHeap(ID3D12Device* pd3dDevice) {

	if (pSamplerDescriptorHeap) {
		pSamplerDescriptorHeap->Release();
		pSamplerDescriptorHeap = nullptr;
	}

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type =D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	pd3dDevice->CreateDescriptorHeap(&srvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&pSamplerDescriptorHeap);


	D3D12_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	SamplerDesc.MipLODBias = 0.0f;
	SamplerDesc.MaxAnisotropy = 1; 
	SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;


	pd3dDevice->CreateSampler(&SamplerDesc, pSamplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}

void CMeshManager::CrateSrvDescriptorHeap(ID3D12Device* pd3dDevice, int numDesc) {

	if (pSrvDescriptorHeap) {
		pSrvDescriptorHeap->Release();
		pSrvDescriptorHeap = nullptr;
	}

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = numDesc;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	pd3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&pSrvDescriptorHeap));

}

 CSumMesh* CMeshManager::BringMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,ID3D12RootSignature* pd3dRootSignature, const char* filename) {

	
	 string CheckTag = ObjectNameFromFile(filename);

		auto pCheck = find_if(MeshList.begin(), MeshList.end(), [&CheckTag](pair<string, CSumMesh*> element) {return element.first == CheckTag; });
		
		//�޽������� �߰� ������
		if (pCheck != MeshList.end()) {
			//�޽� ������ ��ȯ�Ѵ�.
			return	(*pCheck).second;

		}
		//�߰� ����.
		else {

			//������ �о� ���̰� ������ �޾ƿ�.
			CSumMesh* LoadObjectMesh = CSumMesh::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dRootSignature, filename);

			MeshList.push_back({ CheckTag,LoadObjectMesh });

			return LoadObjectMesh;

		}
	

	return nullptr;
}

 CSumMesh* CMeshManager::BringMesh(const char* filename) {


	 string CheckTag = ObjectNameFromFile(filename);

	 auto pCheck = find_if(MeshList.begin(), MeshList.end(), [&CheckTag](pair<string, CSumMesh*> element) {return element.first == CheckTag; });

	 //�޽������� �߰� ������
	 if (pCheck != MeshList.end()) {
		 //�޽� ������ ��ȯ�Ѵ�.
		 return	(*pCheck).second;

	 }
	 //�߰� ����.
	 else {

		 auto firstmesh = MeshList.begin();
		 return (*firstmesh).second;
	 }


	 return nullptr;
 }

string CMeshManager::ObjectNameFromFile(const char* filename) {
	string MeshName = filename;
	string MeshTag = "";


	//�ڿ��� ���� �ܾ fatch�Ұű� ������ ���ݺ��ڸ� �̿���.
	auto pName = std::find(MeshName.rbegin(), MeshName.rend(), '/').base();

	for (; pName != MeshName.end(); pName++)
	{
		if (*pName == '.')
			break;

		MeshTag += *pName;
	}
	
	return MeshTag;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE* CMeshManager::BringTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* filename) {

	string TargetName = ObjectNameFromFile(filename);

	auto  pPosition = find_if(TextureList.begin(), TextureList.end(), [TargetName](pair<string, CD3DX12_GPU_DESCRIPTOR_HANDLE*> element) {return element.first == TargetName; });

	string temp = filename;
	wstring ttemp(temp.begin(), temp.end());
	
	//OutputDebugStringW(ttemp.c_str());

	if (pPosition == TextureList.end()) {

		Texture* TempTexture = new Texture;
		//������ ������
		DirectX::CreateDDSTextureFromFile12(pd3dDevice, pd3dCommandList, ttemp.c_str(), TempTexture->Resource, TempTexture->UploadHeap);
		TempTexture->Name = temp;
		TempTexture->Filename = ttemp;

		srvDesc.Format = TempTexture->Resource->GetDesc().Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = TempTexture->Resource->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		pd3dDevice->CreateShaderResourceView(TempTexture->Resource.Get(), &srvDesc, hCDescriptor);
		CD3DX12_GPU_DESCRIPTOR_HANDLE* TextureAddr= new CD3DX12_GPU_DESCRIPTOR_HANDLE(pSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		TextureAddr->Offset(counting, size);

		TextureList.push_back({ TargetName, TextureAddr });

		//�ش� ���� ����
		hCDescriptor.Offset(1, size);
		counting++;

		return TextureAddr;
	}
	else {
	
		return pPosition->second;
	}
	
}

CD3DX12_GPU_DESCRIPTOR_HANDLE* CMeshManager::BringTexture(const char* filename) {

	string TargetName = ObjectNameFromFile(filename);

	auto  pPosition = find_if(TextureList.begin(), TextureList.end(), [TargetName](pair<string, CD3DX12_GPU_DESCRIPTOR_HANDLE*> element) {return element.first == TargetName; });

	if (TextureList.end() == pPosition) {
		auto error = TextureList.begin();
		//ù��° ���ҽ��� ���� ��ȯ����
		return error->second;

	}
	else {

		//�ش� Ÿ�� ���ҽ� ��ȯ
		return pPosition->second;
	}
}



//Project GameObject CrushBox into XZ Plane
void CMeshManager::MakeNaviationMesh(const std::vector<CGameObject>& GameObjectList)
{
	
	for (CGameObject object : GameObjectList) {

		object.m_BoundingBox;




	}
	



}

