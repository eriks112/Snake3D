#include "GameObject3D.h"

void GameObject3D::SetLookAtPos(XMFLOAT3 lookAtPos)
{
	//Verify that look at pos is not the same as cam pos. They cannot be the same as that wouldn't make sense and would result in undefined behavior.
	if (lookAtPos.x == pos.x && lookAtPos.y == pos.y && lookAtPos.z == pos.z)
		return;

	lookAtPos.x = pos.x - lookAtPos.x;
	lookAtPos.y = pos.y - lookAtPos.y;
	lookAtPos.z = pos.z - lookAtPos.z;

	float pitch = 0.0f;
	if (lookAtPos.y != 0.0f)
	{
		const float distance = sqrt(lookAtPos.x * lookAtPos.x + lookAtPos.z * lookAtPos.z);
		pitch = atan(lookAtPos.y / distance);
	}

	float yaw = 0.0f;
	if (lookAtPos.x != 0.0f)
	{
		yaw = atan(lookAtPos.x / lookAtPos.z);
	}
	if (lookAtPos.z > 0)
		yaw += XM_PI;

	SetRotation(pitch, yaw, 0.0f);
}

const XMVECTOR& GameObject3D::GetForwardVector(bool omitY)
{
	if (omitY)
		return vec_forward_noY;
	else
		return vec_forward;
}

const XMVECTOR& GameObject3D::GetRightVector(bool omitY)
{
	if (omitY)
		return vec_right_noY;
	else
		return vec_right;
}

const XMVECTOR& GameObject3D::GetBackwardVector(bool omitY)
{
	if (omitY)
		return vec_backward_noY;
	else
		return vec_backward;
}

const XMVECTOR& GameObject3D::GetLeftVector(bool omitY)
{
	if (omitY)
		return vec_left_noY;
	else
		return vec_left;
}

const XMVECTOR& GameObject3D::GetUpVector()
{
	return vec_up;
}

const XMVECTOR& GameObject3D::GetDownVector()
{
	return vec_down;
}

void GameObject3D::SetParent(GameObject3D* GameObject3D)
{
	// Removes parent from parent list if exists when nullptr i passed
	if (GameObject3D == nullptr && parent != nullptr)
	{
		GetParentedObjects().erase(std::remove(GetParentedObjects().begin(), GetParentedObjects().end(), this), GetParentedObjects().end());
		return;
	}

	if (GameObject3D == nullptr)
		return;

	// Sets up parent for this object
	this->parent = GameObject3D;
	GetParentedObjects().push_back(this);
	parentPositionOffset.x = 0.0f;
	parentPositionOffset.y = 0.0f;
	parentPositionOffset.z = 0.0f;
	if (trackParentPosition)
	{
		SetPosition(GameObject3D->GetPositionVector());
	}
}

void GameObject3D::SetParentOffset(const XMFLOAT3& offset)
{
	parentPositionOffset = offset;
	parentPositionOffsetVector = XMLoadFloat3(&parentPositionOffset);
	UpdateMatrix();
}

void GameObject3D::SetParentOffset(float x, float y, float z)
{
	parentPositionOffset = XMFLOAT3(x, y, z);
	parentPositionOffsetVector = XMLoadFloat3(&parentPositionOffset);
	UpdateMatrix();
}

void GameObject3D::SetParentOffset(const XMVECTOR& offset)
{
	parentPositionOffsetVector += offset;
	XMStoreFloat3(&this->parentPositionOffset, parentPositionOffsetVector);
	UpdateMatrix();
}

void GameObject3D::SetParentRotationOffset(const XMFLOAT3& offset)
{
	parentRotationOffsetFloat3 = offset;
	parentRotationOffsetVector = XMLoadFloat3(&parentRotationOffsetFloat3);
	UpdateMatrix();
}

void GameObject3D::SetParentRotationOffset(float x, float y, float z)
{
	parentRotationOffsetFloat3 = XMFLOAT3(x, y, z);
	parentRotationOffsetVector = XMLoadFloat3(&parentRotationOffsetFloat3);
	UpdateMatrix();
}

void GameObject3D::SetParentRotationOffset(const XMVECTOR& offset)
{
	parentRotationOffsetVector += offset;
	XMStoreFloat3(&parentRotationOffsetFloat3, parentRotationOffsetVector);
	UpdateMatrix();
}

void GameObject3D::AdjustParentRotationOffset(float x, float y, float z)
{
	parentRotationOffsetFloat3.x += x;
	parentRotationOffsetFloat3.y += y;
	parentRotationOffsetFloat3.z += z;
	parentRotationOffsetVector = XMLoadFloat3(&parentRotationOffsetFloat3);
	UpdateMatrix();
}

void GameObject3D::SetParentTracking(bool state)
{
	trackParentPosition = state;
}

const XMFLOAT3& GameObject3D::GetParentOffset()
{
	return parentPositionOffset;
}

const XMVECTOR& GameObject3D::GetParentOffsetVector()
{
	return parentPositionOffsetVector;
}

const XMFLOAT3& GameObject3D::GetParentRotationOffset()
{
	return parentRotationOffsetFloat3;
}

const XMVECTOR& GameObject3D::GetParentRotationOffsetVector()
{
	return parentRotationOffsetVector;
}

const bool GameObject3D::IsParentTracking()
{
	return trackParentPosition;
}

GameObject3D* GameObject3D::GetParent()
{
	return parent;
}

void GameObject3D::UpdateMatrix()
{
	assert("UpdateMatrix must be overriden." && 0);
}

void GameObject3D::UpdateDirectionVectors()
{
	XMMATRIX vecRotationMatrix = XMMatrixRotationRollPitchYaw(rot.x, rot.y, 0.0f);
	vec_forward = XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, vecRotationMatrix);
	vec_backward = XMVector3TransformCoord(DEFAULT_BACKWARD_VECTOR, vecRotationMatrix);
	vec_left = XMVector3TransformCoord(DEFAULT_LEFT_VECTOR, vecRotationMatrix);
	vec_right = XMVector3TransformCoord(DEFAULT_RIGHT_VECTOR, vecRotationMatrix);
	vec_up = XMVector3TransformCoord(DEFAULT_UP_VECTOR, vecRotationMatrix);
	vec_down = XMVector3TransformCoord(DEFAULT_DOWN_VECTOR, vecRotationMatrix);

	XMMATRIX vecRotationMatrixNoY = XMMatrixRotationRollPitchYaw(0.0f, rot.y, 0.0f);
	vec_forward_noY = XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, vecRotationMatrixNoY);
	vec_backward_noY = XMVector3TransformCoord(DEFAULT_BACKWARD_VECTOR, vecRotationMatrixNoY);
	vec_left_noY = XMVector3TransformCoord(DEFAULT_LEFT_VECTOR, vecRotationMatrixNoY);
	vec_right_noY = XMVector3TransformCoord(DEFAULT_RIGHT_VECTOR, vecRotationMatrixNoY);
}