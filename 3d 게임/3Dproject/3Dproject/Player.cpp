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


/* �÷��̾��� ��ġ�� �����ϴ� �Լ��̴�. �÷��̾��� ��ġ�� �⺻������ ����ڰ� �÷��̾ �̵��ϱ� ���� Ű���带 ������ ����ȴ�. �÷��̾��� �̵����⿡ ���� �÷��̾ fDistance��ŭ �̵��Ѵ�.*/
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

	//bUpdateVelocity�� ���̸� �÷��̾ �̵������ʰ� �ӵ� ���͸� �����Ѵ�.
	if (bUpdateVelocity) {

		//�÷��̾��� �ӵ� ���͸� xmf3Shift ���͸�ŭ �����Ѵ�.
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);

	}
	else {

		//�÷��̾ ���� ��ġ ���Ϳ��� xmf3Shift ���͸�ŭ �̵��Ѵ�.
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);

		//�÷��̾��� ��ġ�� ���� �Ǿ����Ƿ� ī�޶��� ��ġ�� xmf3Shfit��ŭ �̵�
		if (m_pCamera) m_pCamera->Move(xmf3Shift);
	}

}


void CPlayer::Rotate(float x, float y, float z) {

	DWORD nCameraMode = m_pCamera->GetMode();
	//1��Ī ī�޶� �Ǵ� 3��Ī ī�޶��� ��� �÷��̾��� ȸ���� �ణ�� ������ ������.
	if ((nCameraMode == FIRST_PERSON_CAMERA) || (nCameraMode == THIRD_PERSON_CAMERA)) {

		/*���� x���� �߽����� ȸ���ϴ� ���� ���� �յڷ� ���̴� ���ۿ� �ش��Ѵ�. �׷��Ƿ� x���� �߽����� ȸ���ϴ� ������ -89.0~89.0�� ���̷� �����Ѵ�.x�� ������ m_fPitch���� ���� ȸ���ϴ� �����̹Ƿ� x��ŭ ȸ���� ���� pitch�� 89������ ũ�ų� -89�� ���� ������ m_fPitch�� 89�� �Ǵ� -89���� �ǵ��� ȸ�� ������ �����Ѵ�. */

		if (x!= 0.0f) {
			m_fPitch += x;
			if (m_fPitch > 89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}

		if (y != 0.0f) {
			//������ �����°Ϳ��� ������ ����.
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

		//�̵�
		
		if (y != 0.0f) {
			XMMATRIX  xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCameraMode == SPACESHIP_CAMERA) {
	//�����̽�-�� ī�޶󿡼� �÷��̾��� ȸ���� ȸ�� ������ ������ ����. �׸��� ��� ���� �߽����� ȸ���� �� �� �ִ�.
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
	//ȸ������ ���� �÷��̾��� ���� x-��, y-�� , z-���� ���� ������������ �� �����Ƿ� z-��(look����)�� ���������Ͽ� ���� �����ϰ� ���� ���Ͱ� �ǵ��� �Ѵ�.

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

}

// �������� ���� ȣ�� �Ǹ�, �÷��̾��� �ӵ� ���Ϳ� �߷°� ������ ���� �����Ѵ�.
void CPlayer::Update(float fTimeElapsed) {

	//�÷��̾��� �ӵ� ���͸� �߷� ���Ϳ� ���Ѵ�. �߷� ���Ϳ� fTimeElapsed�� ���ϴ� ���� �߷��� �ð��� ����ϵ��� �����Ѵٴ� �ǹ��̴�.
	
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));

	//�÷��̾��� �ӵ� ������ XZ-������ ũ�⸦ ���Ѵ�. �̰��� xz-����� �ִ� �ӷº��� ũ�� �ӵ� ������ x�� z-���� ������ �����Ѵ�.
	
	float fLength = sqrt(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;

	if (fLength > m_fMaxVelocityXZ) {

		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	//�÷��̾��� �ӵ� ������ y������ ũ�⸦ ���Ѵ� �̰��� y�� ������ ���� �ӷº��� ũ�� �ӵ� ������ y ���⤷ ������ �����Ѵ�.

	
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);

	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	//�÷��̾ �ӵ� ���͸�ŭ ������ �̵��Ѵ�(ī�޶� �̵��� ���̴�.)

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	//�÷��̾��� ��ġ�� ����ɶ� �߰��� ������ �۾��� �����Ѵ�. �÷��̾��� ���ο� ��ġ�� ��ȿ�� ��ġ�� �ƴ� ���� �ְ� �Ǵ� �÷��̾��� �浹 �˻���� ������ �ʿ䰡 �ִ�. �̷��� ��Ȳ����
	//�÷��̾��� ��ġ�� ��ȿ�� ��ġ�� �ٽ� �����Ҽ� �ִ�.
	
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCameraMode = m_pCamera->GetMode();
	//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� 3��Ī ī�޶� �����Ѵ�.
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);

	//ī�޶��� ��ġ�� ���� �� �� �߰��� ������ �۾��� �����Ѵ�.
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	//ī�޶� 3��Ī ī�޶��̸� ī�޶� ����� �÷��̾� ��ġ�� �ٶ󺸵��� �Ѵ�.
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	//ī�޶��� ī�޶� ��ȯ ����� �ٽ� �����Ѵ�.
	m_pCamera->RegenerateViewMatrix();
	
	/*�÷��̾��� �ӵ� ���Ͱ� ������ ������ ������ �Ǿ�� �Ѵٸ� ���� ���͸� �����Ѵ�. �ӵ� ������ �ݴ� ���� ���͸� ���ϰ� ���� ���ͷ� �����. ���� ����� �ð��� ����ϵ��� �Ͽ� �������� ���Ѵ�.
	���� ���Ϳ� �������� ���Ͽ� ���� ���͸� ���Ѵ� �ӵ� ���Ϳ� ���� ���͸� ���Ͽ� �ӵ� ���͸� ���δ�. �������� �ӷº��� ũ�� �ӷ��� 0�� �� ���̴�.*/

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));


}

//ī�޶� ������ �� changeCamera() �Լ����� ȣ��Ǵ� �Լ��̴�. nCurrentCameraMode�� ���� ī�޶��� ����̰� nNewCameraMode�� ���� ������ ī�޶� ����̴�.


CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode) {

	//���ο� ī�޶��� ��忡 ���� ī�޶� ���� �����Ѵ�.
	
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
		//Look ���Ϳ� ������ǥ���� z��(0,0,1)�� �̷�� ������ ����Ͽ� �÷��̾��� ȸ�� ���� m_fYaw�� �����Ѵ�.
		temp = XMFLOAT3{ 0.0f, 0.0f, 1.0f };
		m_fYaw = Vector3::Angle( temp , m_xmf3Look);

		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{

		//���ο� ī�޶��� ��尡 �����̽� �� ����� ī�޶��̰� ���� ī�޶� ��尡 1��Ī�Ǵ� 3��Ī ī�޶��̸� �÷��̾��� ���� ���� ���� ī�޶��� ���� ��� ���� �����.
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera) {

		pNewCamera->SetMode(nNewCameraMode);
		//���� ī�޶� ����ϴ� �÷��̾� ��ü�� �����Ѵ�.
		pNewCamera->SetPlayer(this);
	}
	
	if (m_pCamera)	 delete m_pCamera;

	return(pNewCamera);
}



/*�÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ��̴�. �÷��̾��� Right ���Ͱ� ���� ��ȯ ���
�� ù ��° �� ����, Up ���Ͱ� �� ��° �� ����, Look ���Ͱ� �� ��° �� ����, �÷��̾��� ��ġ ���Ͱ� �� ��° ��
���Ͱ� �ȴ�.*/
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

	

	//�÷��̾��� ī�޶� �����̽�-�v ī�޶�� �����Ѵ�.
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	//�÷��̾ ���� ���̴� ������ �����Ѵ�.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);


}

void Creater::OnPrepareRender() {

	CPlayer::OnPrepareRender();
	
	//����� ���� �׸��� ���� x-������ 90�� ȸ���Ѵ�.
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f), 0.0f, 0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);

	
}

void Creater::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {

	OnPrepareRender();
	m_pShader->OnPrepareRender(pd3dCommandList, 0);
	CGameObject::UpdateAllTansform();
	CGameObject::Render(pd3dCommandList, pCamera);

}
/*3��Ī ī�޶��� �� �÷��̾� �޽��� ���� x-���� �߽����� +90�� ȸ���ϰ� �������Ѵ�. �ֳ��ϸ� ����� �� �޽��� ���� �׸��� ���� y-�� ������ ������� ������ �ǵ��� �𵨸� �Ǿ��� �����̴�.
�׸��� �� �޽��� ī�޶��� z-�� �������� ���ϵ��� �׸� ���̱� �����̴�.*/

//ī�޶� ������ �� ȣ��Ǵ� �Լ��̴�. nNewCameraMode�� ���� ������ ī�޶� ����̴�.

CCamera* Creater::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) {

	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return (m_pCamera);


	XMFLOAT3 temp = XMFLOAT3(0.0f, 1.0f, -1.0f);

	switch (nNewCameraMode) {

	case FIRST_PERSON_CAMERA:

		//�÷��̾��� Ư���� 1��Ī ī�޶� ��忡 �°� �����Ѵ�. �߷��� �������� �ʴ´�.
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
		//�÷��̾��� Ư���� �����̽�-�� ī�޶� ��忡 �°� �����Ѵ� �߷��� �������� �ʴ´�.
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
		//3��Ī ī�޶��� ���� ȿ���� �����Ѵ�. ���� 0.25 ��� 0.0f �� 1.0f�� ������ ����� ���ϱ� �ٶ���.
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

	//�÷��̾��� ī�޶� �����̽�-�v ī�޶�� �����Ѵ�.
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	//�÷��̾ ���� ���̴� ������ �����Ѵ�.
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

		//�÷��̾��� Ư���� 1��Ī ī�޶� ��忡 �°� �����Ѵ�. �߷��� �������� �ʴ´�.
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
		//�÷��̾��� Ư���� �����̽�-�� ī�޶� ��忡 �°� �����Ѵ� �߷��� �������� �ʴ´�.
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
		//3��Ī ī�޶��� ���� ȿ���� �����Ѵ�. ���� 0.25 ��� 0.0f �� 1.0f�� ������ ����� ���ϱ� �ٶ���.
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

	//��� ��� ����
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

	
	//�ϴ� ��� ����
	if (flag_move_screen) {
		XMFLOAT3 v1(BottomTransform._31, BottomTransform._32, BottomTransform._33);
		XMFLOAT3 v2(m_xmf3Look);
		v1 = Vector3::Normalize(v1);
		v2 = Vector3::Normalize(v2);
		float dotProduct = Vector3::DotProduct(v1, v2);//��ü�� ����� �̵������� ����..
		float v1Size = VectorSize(v1);//���̰��� ���ϱ� ���� ���͵��� ũ�ⱸ�ϱ�
		float v2Size = VectorSize(v2);
		betweenangle = acosf(dotProduct / (v1Size * v2Size));//���̰� ���ϱ�

		CrossBetween = Vector3::Normalize(Vector3::CrossProduct(v1, v2));//�ִ� ȸ���� ���ϱ� ���� ����
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
	//��� ���� ���ϱ�
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
