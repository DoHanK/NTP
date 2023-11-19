#include "Player.h"
#include "Shader.h"


CPlayer::CPlayer() {
	m_pCamera = NULL;
		
	m_xmf3Position = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();
	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {

	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	if (m_pCamera) m_pCamera->CreateShaderVariavles(pd3dDevice, pd3dCommandList);

}

void CPlayer::ReleaseShaderVariables() {

	CGameObject::ReleaseShaderVariables();

	if (m_pCamera) m_pCamera->ReleaseShaderVariavles();
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {


	CGameObject::UpdateShaderVariables(pd3dCommandList);

}


/* 플레이어의 위치를 변경하는 함수이다. 플레이어의 위치는 기본적으로 사용자가 플레이어를 이동하기 위한 키보드를 누를때 변경된다. 플레이어의 이동방향에 따라 플레이어를 fDistance만큼 이동한다.*/
void CPlayer::Move(ULONG dwDirection, float fDistance, bool bVelocity) {

	if (dwDirection) {
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
	

		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, Vector3::ScalarProduct(m_xmf3Look, fDistance));
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, Vector3::ScalarProduct(m_xmf3Look, -fDistance));


		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, Vector3::ScalarProduct(m_xmf3Right, fDistance));
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, Vector3::ScalarProduct(m_xmf3Right, -fDistance));


		if (dwDirection & DIR_UP)	xmf3Shift = Vector3::Add(xmf3Shift, Vector3::ScalarProduct(m_xmf3Up, fDistance));
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, Vector3::ScalarProduct(m_xmf3Up, -fDistance));

		Move(xmf3Shift, bVelocity);
	}


}


void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity) {

	//bUpdateVelocity가 참이면 플레이어를 이동하지않고 속도 벡터를 변경한다.
	if (bUpdateVelocity) {

		//플레이어의 속도 벡터를 xmf3Shift 벡터만큼 변경한다.
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);

	}
	else {

		//플레이어를 현재 위치 벡터에서 xmf3Shift 벡터만큼 이동한다.
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);

		//플레이어의 위치가 변경 되었으므로 카메라의 위치도 xmf3Shfit만큼 이동
		if (m_pCamera) m_pCamera->Move(xmf3Shift);
	}

}


void CPlayer::Rotate(float x, float y, float z) {

	DWORD nCameraMode = m_pCamera->GetMode();
	//1인칭 카메라 또는 3인칭 카메라의 경우 플레이어의 회전은 약간의 제약이 따른다.
	if ((nCameraMode == FIRST_PERSON_CAMERA) || (nCameraMode == THIRD_PERSON_CAMERA)) {

		/*로컬 x축을 중심으로 회전하는 것은 고개를 앞뒤로 숙이는 동작에 해당한다. 그러므로 x축을 중심으로 회전하는 각도는 -89.0~89.0도 사이로 제한한다.x는 현재의 m_fPitch에서 실제 회전하는 각도이므로 x만큼 회전한 다음 pitch가 89도보다 크거나 -89도 보다 작으면 m_fPitch가 89도 또는 -89도가 되도록 회전 각도를 수정한다. */

		if (x!= 0.0f) {
			m_fPitch += x;
			if (m_fPitch > 89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}

		if (y != 0.0f) {
			//몸통을 돌리는것에는 제약이 없다.
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}

		if (z != 0.0f) {
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);

		//이동
		
		if (y != 0.0f) {
			XMMATRIX  xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCameraMode == SPACESHIP_CAMERA) {
	//스페이스-쉽 카메라에서 플레이어의 회전은 회전 각도의 제한이 없다. 그리고 모든 축을 중심으로 회전을 할 수 있다.
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f) {
			
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));

			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}

		if (y != 0.0f) {
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}

		if (z != 0.0f) {
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	//회전으로 인해 플레이어의 로컬 x-축, y-축 , z-축이 서로 직교하지않을 수 있으므로 z-축(look벡터)을 기준으로하여 서로 직교하고 단위 벡터가 되도록 한다.

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

}

// 매프레임 마다 호출 되며, 플레이어의 속도 벡터에 중력과 마찰력 등을 적용한다.
void CPlayer::Update(float fTimeElapsed) {

	//플레이어의 속도 벡터를 중력 벡터와 더한다. 중력 벡터에 fTimeElapsed를 곱하는 것은 중력을 시간에 비례하도록 적용한다는 의미이다.
	
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));

	//플레이어의 속도 벡터의 XZ-성분의 크기를 구한다. 이것이 xz-평면의 최대 속력보다 크면 속도 벡터의 x와 z-방향 성분을 조정한다.
	
	float fLength = sqrt(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;

	if (fLength > m_fMaxVelocityXZ) {

		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	//플레이어의 속도 벡터의 y성분의 크기를 구한다 이것이 y축 바향의 쵀대 속력보다 크면 속도 벡터의 y 방향ㅇ 성분을 조정한다.

	
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);

	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	//플레이어를 속도 벡터만큼 실제로 이동한다(카메라도 이동될 것이다.)

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	//플레이어의 위치가 변경될때 추가로 수행할 작업을 수행한다. 플레이어의 새로운 위치가 유효한 윝치가 아닐 수도 있고 또는 플레이어의 충돌 검사등을 수행할 필요가 있다. 이러한 상황에서
	//플레이어의 위치를 유효한 위치로 다시 변경할수 있다.
	
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCameraMode = m_pCamera->GetMode();
	//플레이어의 위치가 변경되었으므로 3인칭 카메라를 갱신한다.
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);

	//카메라의 위치가 변경 될 때 추가로 수행할 작업을 수행한다.
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	//카메라가 3임칭 카메라이면 카메라가 변경된 플레이어 위치를 바라보도록 한다.
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	//카메라의 카메라 변환 행렬을 다시 생성한다.
	m_pCamera->RegenerateViewMatrix();
	
	/*플레이어의 속도 벡터가 마찰력 때문에 감속이 되어야 한다면 감속 벡터를 생성한다. 속도 벡터의 반대 방향 벡터를 구하고 단위 벡터로 만든다. 마찰 계수를 시간에 비례하도록 하여 마찰력을 구한다.
	단위 벡터에 마찰력을 곱하여 감속 벡터를 구한다 속도 벡터에 감속 벡터를 더하여 속도 벡터를 줄인다. 마찰력이 속력보다 크면 속력은 0이 될 것이다.*/

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));


}

//카메라를 변경할 때 changeCamera() 함수에서 호출되는 함수이다. nCurrentCameraMode는 현재 카메라의 모드이고 nNewCameraMode는 새로 설정할 카메라 모드이다.


CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode) {

	//새로운 카메라의 모드에 따라 카메라를 새로 생성한다.
	
	CCamera* pNewCamera = NULL;
	switch (nNewCameraMode) {

	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = new CSpaceShipCamera(m_pCamera);
		break;
	}

	if (nCurrentCameraMode == SPACESHIP_CAMERA) {
		
		XMFLOAT3 temp{ m_xmf3Right.x, 0.0f, m_xmf3Right.z };
		
		m_xmf3Right = Vector3::Normalize(temp);
		temp = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_xmf3Up = Vector3::Normalize(temp);
		temp = XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z);
		m_xmf3Look = Vector3::Normalize(temp);
	
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		//Look 벡터와 월드좌표계의 z축(0,0,1)이 이루는 각도를 계산하여 플레이어의 회전 각도 m_fYaw로 설정한다.
		temp = XMFLOAT3{ 0.0f, 0.0f, 1.0f };
		m_fYaw = Vector3::Angle( temp , m_xmf3Look);

		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{

		//새로운 카메라의 모드가 스페이스 쉽 모드의 카메라이고 현재 카메라 모드가 1인칭또는 3인칭 카메라이면 플레이어의 로컬 축을 현재 카메라의 로컬 축과 같게 만든다.
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera) {

		pNewCamera->SetMode(nNewCameraMode);
		//현재 카메라를 사용하는 플레이어 객체를 설정한다.
		pNewCamera->SetPlayer(this);
	}
	
	if (m_pCamera)	 delete m_pCamera;

	return(pNewCamera);
}



/*플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수이다. 플레이어의 Right 벡터가 월드 변환 행렬
의 첫 번째 행 벡터, Up 벡터가 두 번째 행 벡터, Look 벡터가 세 번째 행 벡터, 플레이어의 위치 벡터가 네 번째 행
벡터가 된다.*/
void CPlayer::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x;
	m_xmf4x4World._12 = m_xmf3Right.y;
	m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x;
	m_xmf4x4World._22 = m_xmf3Up.y;
	m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x;
	m_xmf4x4World._42 = m_xmf3Position.y;
	m_xmf4x4World._43 = m_xmf3Position.z;
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {

	DWORD nCameraMode = (pCamera) ? (pCamera)->GetMode() : 0x00;

	CGameObject::Render(pd3dCommandList, pCamera);
	

}

Creater::Creater(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) {

	

	//플레이어의 카메라를 스페이스-쉡 카메라로 변경한다.
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	//플레이어를 위한 세이더 변수를 생성한다.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);


}

void Creater::OnPrepareRender() {

	CPlayer::OnPrepareRender();
	
	//비행기 모델을 그릭기 전에 x-축으로 90도 회전한다.
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f), 0.0f, 0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);

	
}

void Creater::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {

	OnPrepareRender();
	m_pShader->OnPrepareRender(pd3dCommandList, 0);
	CGameObject::UpdateAllTansform();
	CGameObject::Render(pd3dCommandList, pCamera);

}
/*3인칭 카메라일 때 플레이어 메쉬를 로컬 x-축을 중심으로 +90도 회전하고 렌더링한다. 왜냐하면 비행기 모델 메쉬는 다음 그림과 같이 y-축 방향이 비행기의 앞쪽이 되도록 모델링 되었기 때문이다.
그리고 이 메쉬를 카메라의 z-축 방향으로 향하도록 그릴 것이기 때문이다.*/

//카메라를 변경할 때 호출되는 함수이다. nNewCameraMode는 새로 설정할 카메라 모드이다.

CCamera* Creater::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) {

	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return (m_pCamera);


	XMFLOAT3 temp = XMFLOAT3(0.0f, 1.0f, -1.0f);

	switch (nNewCameraMode) {

	case FIRST_PERSON_CAMERA:

		//플레이어의 특성을 1인칭 카메라 모드에 맞게 변경한다. 중력은 적용하지 않는다.
		SetFriction(200.0f);
		SetGravity(temp);
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		temp = XMFLOAT3(0.0f, 20.0f, 0.0f);
		m_pCamera->SetOffset(temp);
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewPort(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;

	case SPACESHIP_CAMERA:
		//플레이어의 특성을 스페이스-싑 카메라 모드에 맞게 변경한다 중력은 적용하지 않는다.
		SetFriction(20.0f);
		temp = XMFLOAT3(0.0f, 0.0f, 0.0f);
		SetGravity(temp);
		SetMaxVelocityXZ(400.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		temp = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_pCamera->SetOffset(temp);
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewPort(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT,0.0f , 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(40.0f);
		temp = XMFLOAT3(0.0f, 0.0f, 0.0f);
		SetGravity(temp);
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		//3인칭 카메라의 지연 효과를 설정한다. 값을 0.25 대신 0.0f 와 1.0f로 설정한 결과를 비교하기 바란다.
		m_pCamera->SetTimeLag(0.25f);
		temp = XMFLOAT3(0.0f, 1.0f, -6.0f);
		m_pCamera->SetOffset(temp);
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewPort(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT,0.0f , 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));

	Update(fTimeElapsed);

	return (m_pCamera);
}

CTanker::CTanker(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,CSumMesh* bulletMesh, CD3DX12_GPU_DESCRIPTOR_HANDLE* TextureAddr, CSumMesh* MineMesh)
{

	//플레이어의 카메라를 스페이스-쉡 카메라로 변경한다.
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	//플레이어를 위한 세이더 변수를 생성한다.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	XMStoreFloat4x4(&TopTransform, XMMatrixIdentity());
	XMStoreFloat4x4(&BottomTransform, XMMatrixIdentity());

	for (int i = 0; i < BULLETS; i++)
	{
		m_ppBullets[i] = new CMissile(1500000.f);
		m_ppBullets[i]->SetMesh(bulletMesh);
		m_ppBullets[i]->m_TextureAddr =TextureAddr;
		m_ppBullets[i]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppBullets[i]->SetRotationSpeed(360.0f);
		m_ppBullets[i]->SetMovingSpeed(20.0f);
		m_ppBullets[i]->SetActive(false);
	}

	for (int i = 0; i < MINES; ++i) {
		m_pMine[i] = new CMine();
		m_pMine[i]->SetMesh(MineMesh);
		m_pMine[i]->m_TextureAddr = TextureAddr;
		m_pMine[i]->SetActive(false);	
	}


}

CCamera* CTanker::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return (m_pCamera);


	XMFLOAT3 temp = XMFLOAT3(0.0f, 1.0f, -1.0f);

	switch (nNewCameraMode) {

	case FIRST_PERSON_CAMERA:

		//플레이어의 특성을 1인칭 카메라 모드에 맞게 변경한다. 중력은 적용하지 않는다.
		SetFriction(200.0f);
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		temp = XMFLOAT3(0.0f, 20.0f, 0.0f);
		m_pCamera->SetOffset(temp);
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewPort(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;

	case SPACESHIP_CAMERA:
		//플레이어의 특성을 스페이스-싑 카메라 모드에 맞게 변경한다 중력은 적용하지 않는다.
		SetFriction(20.0f);
		SetGravity(temp);
		SetMaxVelocityXZ(400.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(temp);
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewPort(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(49.0f);
		temp = XMFLOAT3(0.0f, 0.0f, 0.0f);
		SetGravity(temp);
		SetMaxVelocityXZ(65.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		//3인칭 카메라의 지연 효과를 설정한다. 값을 0.25 대신 0.0f 와 1.0f로 설정한 결과를 비교하기 바란다.
		m_pCamera->SetTimeLag(0.25f);
		temp = XMFLOAT3(0.0f, 4.0f, -10.0f);
		m_pCamera->SetOffset(temp);
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewPort(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));

	Update(fTimeElapsed);

	return (m_pCamera);

}

void CTanker::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	m_pShader->OnPrepareRender(pd3dCommandList, 0);

	CGameObject::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive) {
			m_ppBullets[i]->UpdateAllTansform();
			m_ppBullets[i]->Render(pd3dCommandList, pCamera);
		}
	}
	for (int i = 0; i < MINES; ++i) {
		if (m_pMine[i]->m_bActive) {
			m_pMine[i]->UpdateAllTansform();
			m_pMine[i]->Render(pd3dCommandList, pCamera);
		}
	}
}

void CTanker::UpdateWheel()
{
	XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(1.2f));
	for (int i = 0; i < 4; ++i) {
		*wheel[i] = Matrix4x4::Multiply(xmmtxRotate, *wheel[i]);
	}

}

void CTanker::OnPrepareRender()
{
	if (m_BCameraRotate) {
		m_xmf4x4World._41 = m_xmf3Position.x;
		m_xmf4x4World._42 = m_xmf3Position.y;
		m_xmf4x4World._43 = m_xmf3Position.z;
	}
	else {

	CPlayer::OnPrepareRender();
	}

	//상단 행렬 정보
	TopTransform._11 = m_xmf3Right.x;
	TopTransform._12 = m_xmf3Right.y;
	TopTransform._13 = m_xmf3Right.z;
	TopTransform._21 = m_xmf3Up.x;
	TopTransform._22 = m_xmf3Up.y;
	TopTransform._23 = m_xmf3Up.z;
	TopTransform._31 = m_xmf3Look.x;
	TopTransform._32 = m_xmf3Look.y;
	TopTransform._33 = m_xmf3Look.z;
	TopTransform._41 = m_xmf3Position.x;
	TopTransform._42 = m_xmf3Position.y;
	TopTransform._43 = m_xmf3Position.z;

	
	//하단 행렬 정보
	if (flag_move_screen) {
		XMFLOAT3 v1(BottomTransform._31, BottomTransform._32, BottomTransform._33);
		XMFLOAT3 v2(m_xmf3Look);
		v1 = Vector3::Normalize(v1);
		v2 = Vector3::Normalize(v2);
		float dotProduct = Vector3::DotProduct(v1, v2);//물체의 방향과 이동방향을 내적..
		float v1Size = VectorSize(v1);//사이각을 구하기 위한 벡터들의 크기구하기
		float v2Size = VectorSize(v2);
		betweenangle = acosf(dotProduct / (v1Size * v2Size));//사이각 구하기

		CrossBetween = Vector3::Normalize(Vector3::CrossProduct(v1, v2));//최단 회전을 구하기 위한 외적
		BottomAnimationFrame = bottom_frame;
		flag_move_screen = FALSE;
	}
	BottomTransform._41 = m_xmf3Position.x;
	BottomTransform._42 = m_xmf3Position.y;
	BottomTransform._43 = m_xmf3Position.z;


	UpdateAllTansform();
}

void CTanker::UpdateAllTansform()
{
	//행렬 정보 곱하기
	BottomMesh->m_xmf4x4World = Matrix4x4::Multiply(*Bottom, BottomTransform);
	WheelMesh[0]->m_xmf4x4World = Matrix4x4::Multiply(*wheel[0],BottomMesh->m_xmf4x4World);
	WheelMesh[1]->m_xmf4x4World = Matrix4x4::Multiply(*wheel[1],BottomMesh->m_xmf4x4World);
	WheelMesh[2]->m_xmf4x4World = Matrix4x4::Multiply(*wheel[2],BottomMesh->m_xmf4x4World);
	WheelMesh[3]->m_xmf4x4World = Matrix4x4::Multiply(*wheel[3],BottomMesh->m_xmf4x4World);


	TopMesh->m_xmf4x4World = Matrix4x4::Multiply(*Top, TopTransform);
	GunMesh->m_xmf4x4World = Matrix4x4::Multiply(*Gun, TopMesh->m_xmf4x4World);

}

void CTanker::FindFrameSet() {

	auto p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Wheel_b_right"; });
	wheel[0] = &(p->second);
	p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Wheel_b_left"; });
	wheel[1] = &(p->second);
	p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Wheel_f_right"; });
	wheel[2] = &(p->second); 
	p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Wheel_f_left"; });
	wheel[3] = &(p->second); 
	p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Tower"; });
	Top = &(p->second);
	p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Canon"; });
	Gun = &(p->second);
	p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Green"; });
	Bottom = &(p->second);

	TopMesh = m_pMesh->FindFrame("TankFree_Tower");
	GunMesh = m_pMesh->FindFrame("TankFree_Canon");
	BottomMesh = m_pMesh->FindFrame("TankFree_Green");
	WheelMesh[0] = m_pMesh->FindFrame("TankFree_Wheel_b_right");
	WheelMesh[1] = m_pMesh->FindFrame("TankFree_Wheel_b_left");
	WheelMesh[2] = m_pMesh->FindFrame("TankFree_Wheel_f_right");
	WheelMesh[3] = m_pMesh->FindFrame("TankFree_Wheel_f_left");

}

float CTanker::VectorSize(XMFLOAT3 V) {

	return (float)sqrt(V.x * V.x + V.y * V.y + V.z * V.z);
}

void CTanker::Animate(float fTimeEelapsed)
{
	if (BottomAnimationFrame > 0) {

		if (betweenangle > 0.01f) {

			if (Vector3::DotProduct(XMFLOAT3(0.0f, 1.0f, 0.0f), CrossBetween) > 0)
				BottomTransform = Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f), betweenangle / bottom_frame, 0.0f), BottomTransform);
			else
				BottomTransform = Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f), -betweenangle / bottom_frame, 0.0f), BottomTransform);
		}

		UpdateWheel();
		BottomAnimationFrame--;
	}

	for (int i = 0; i < BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive)
			m_ppBullets[i]->Animate(fTimeEelapsed);
	}

	if (m_CameraAngle > 0)
	{
		m_CameraAngle -= 1;
		m_pCamera->Rotate(0, -1, 0);
	}
	else if(m_CameraAngle < 0){
		m_CameraAngle += 1;
		m_pCamera->Rotate(0, 1, 0);
	}
	m_pCamera->RegenerateViewMatrix();


	if (m_BCameraMove) {

		m_pCamera->Move(Vector3::ScalarProduct(m_xmf3Look,-0.1));


		m_CameraDistance -= fTimeEelapsed;
		if (m_CameraDistance <= 0) {
			m_BCameraMove = false;
		}
	}

	
}

void CTanker::FireMissile()
{
	CMissile* pBulletObject = NULL;

	for (int i = 0; i < BULLETS; i++)
	{
		if (!m_ppBullets[i]->m_bActive)
		{
			pBulletObject = m_ppBullets[i];
			break;
		}
	}

	if (pBulletObject)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmf3Direction = GetLook();
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 2.0f, false));
		xmf3FirePosition.y += 0.5;
		pBulletObject->m_xmf4x4World = m_xmf4x4World;
		pBulletObject->SetFirePosition(xmf3FirePosition);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetActive(true);

	}




	m_BCameraMove = true;
	m_CameraDistance = 0.05;
	int strength = 0.01f;
	m_pCamera->Move(Vector3::ScalarProduct(m_xmf3Right, strength* RANDOM_SIGN));
	m_pCamera->Move(Vector3::ScalarProduct(m_xmf3Up, strength* RANDOM_SIGN));
	
}

void CTanker::SetMine()
{
	if (UseMine < MINES) {
		m_pMine[UseMine]->m_bActive = true;
		m_pMine[UseMine]->m_xmf4x4World = m_xmf4x4World;
		m_pMine[UseMine++]->m_xmf4x4World._42 -= 1;
	}
}

void CTanker::UpdateBoundingBox()
{
		m_pMesh->m_pMesh->m_xmBoundingBox.Transform(m_BoundingBox, XMLoadFloat4x4(&m_xmf4x4World));
		XMStoreFloat4(&m_BoundingBox.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_BoundingBox.Orientation)));

		XMFLOAT4X4 temp = Matrix4x4::Multiply(*Top,m_xmf4x4World);


		TopMesh->m_pMesh->m_xmBoundingBox.Transform(TopBoundingBox, XMLoadFloat4x4(&temp));
		XMStoreFloat4(&TopBoundingBox.Orientation, XMQuaternionNormalize(XMLoadFloat4(&TopBoundingBox.Orientation)));
}

void CTanker::ChangeColor(CD3DX12_GPU_DESCRIPTOR_HANDLE* TankTexAddr, CD3DX12_GPU_DESCRIPTOR_HANDLE* ObjectTexAddr)
{

	m_TextureAddr = TankTexAddr;


	for (int i = 0; i < BULLETS; i++){
		m_ppBullets[i]->m_TextureAddr = ObjectTexAddr;
	}

	for (int i = 0; i < MINES; ++i) {
		m_pMine[i]->m_TextureAddr = ObjectTexAddr;
	}

}

void CTanker::initGame(void* packet)
{
	SC_ADD_PLAYER_PACKET* p = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(packet);
	//XMFLOAT4X4 TempMatrix;
	//TempMatrix._12 = p->top_dir.y;
	//TempMatrix._13 = p->top_dir.z;
	//TempMatrix._11 = p->top_dir.x;
	//TempMatrix._14 = 1.0f;
	//TempMatrix._21 = 0;
	//TempMatrix._22 = 1.0f;
	//TempMatrix._23 = 0;
	//TempMatrix._24 = 1.0f;
	//XMFLOAT3 LookVector = Vector3::CrossProduct(p->top_dir, XMFLOAT3(0, 1, 0));
	//TempMatrix._31 = LookVector.x;
	//TempMatrix._32 = LookVector.y;
	//TempMatrix._33 = LookVector.z;
	//TempMatrix._34 = 1.0f;
	//TempMatrix._41 = p->pos.x;
	//TempMatrix._42 = p->pos.y;
	//TempMatrix._43 = p->pos.z;
	//TempMatrix._44 = 1.0f;

	////top Info
	//TopTransform = TempMatrix;

	////top Info
	//TempMatrix._11 = p->bottom_dir.x;
	//TempMatrix._12 = p->bottom_dir.y;
	//TempMatrix._13 = p->bottom_dir.z;
	//LookVector = Vector3::CrossProduct(p->bottom_dir, XMFLOAT3(0, 1, 0));
	//TempMatrix._31 = LookVector.x;
	//TempMatrix._32 = LookVector.y;
	//TempMatrix._33 = LookVector.z;
	//BottomTransform = TempMatrix;
	 
	 
	 
	 

	//XMFLOAT3 LookVector = Vector3::CrossProduct(p->top_dir, XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_xmf3Position= p->pos;
	//m_xmf3Right =p->top_dir; 
	//m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	//m_xmf3Look= LookVector;


}
