#include "GameObject.h"

const XMVECTOR& GameObject::GetPositionVector() const
{
	return this->posVector;
}

const XMFLOAT3& GameObject::GetPositionFloat3() const
{
	return this->pos;
}

const XMVECTOR& GameObject::GetRotationVector() const
{
	return this->rotVector;
}

const XMFLOAT3& GameObject::GetRotationFloat3() const
{
	return this->rot;
}

const XMFLOAT3& GameObject::GetScaleFloat3() const
{
	return this->scale;
}

void GameObject::SetPosition(const XMVECTOR& pos)
{
	XMStoreFloat3(&this->pos, pos);
	this->posVector = pos;
	this->UpdateMatrix();
}

void GameObject::SetPosition(const XMFLOAT3& pos)
{
	this->pos = pos;
	this->posVector = XMLoadFloat3(&this->pos);
	this->UpdateMatrix();
}

void GameObject::SetPosition(float x, float y, float z)
{
	this->pos = XMFLOAT3(x, y, z);
	this->posVector = XMLoadFloat3(&this->pos);
	this->UpdateMatrix();
}

void GameObject::SetScale(const XMFLOAT3& scale)
{
	this->scale = scale;
	this->UpdateMatrix();
}

void GameObject::SetScale(float x, float y, float z)
{
	this->scale = XMFLOAT3(x, y, z);
	this->UpdateMatrix();
}

void GameObject::AdjustScale(const XMFLOAT3& pos)
{
	this->scale.x += pos.y;
	this->scale.y += pos.y;
	this->scale.z += pos.z;
	this->UpdateMatrix();
}

void GameObject::AdjustScale(float x, float y, float z)
{
	this->scale.x += x;
	this->scale.y += y;
	this->scale.z += z;
	this->UpdateMatrix();
}

void GameObject::AdjustPosition(const XMVECTOR& pos)
{
	this->posVector += pos;
	XMStoreFloat3(&this->pos, this->posVector);
	this->UpdateMatrix();
}

void GameObject::AdjustPosition(const XMFLOAT3& pos)
{
	this->pos.x += pos.y;
	this->pos.y += pos.y;
	this->pos.z += pos.z;
	this->posVector = XMLoadFloat3(&this->pos);
	this->UpdateMatrix();
}

void GameObject::AdjustPosition(float x, float y, float z)
{
	this->pos.x += x;
	this->pos.y += y;
	this->pos.z += z;
	this->posVector = XMLoadFloat3(&this->pos);
	this->UpdateMatrix();
}

void GameObject::SetRotation(const XMVECTOR& rot)
{
	this->rotVector = rot;
	XMStoreFloat3(&this->rot, rot);
	this->UpdateMatrix();
}

void GameObject::SetRotation(const XMFLOAT3& rot)
{
	this->rot = rot;
	this->rotVector = XMLoadFloat3(&this->rot);
	this->UpdateMatrix();
}

void GameObject::SetRotation(float x, float y, float z)
{
	this->rot = XMFLOAT3(x, y, z);
	this->rotVector = XMLoadFloat3(&this->rot);
	this->UpdateMatrix();
}

void GameObject::AdjustRotation(const XMVECTOR& rot)
{
	this->rotVector += rot;
	XMStoreFloat3(&this->rot, this->rotVector);
	this->UpdateMatrix();
}

void GameObject::AdjustRotation(const XMFLOAT3& rot)
{
	this->rot.x += rot.x;
	this->rot.y += rot.y;
	this->rot.z += rot.z;
	this->rotVector = XMLoadFloat3(&this->rot);
	this->UpdateMatrix();
}

void GameObject::AdjustRotation(float x, float y, float z)
{
	this->rot.x += x;
	this->rot.y += y;
	this->rot.z += z;
	this->rotVector = XMLoadFloat3(&this->rot);
	this->UpdateMatrix();
}

bool GameObject::IsAnimationActive()
{
	return this->isAnimationActive;
}

void GameObject::SetAnimationActive(bool state)
{
	isAnimationActive = state;
}

void GameObject::UpdateMatrix()
{
	assert("UpdateMatrix must be overriden." && 0);
}
