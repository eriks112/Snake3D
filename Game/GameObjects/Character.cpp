#include "Character.h"

bool Character::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	// Initializing snake body
	if (!model.Initialize(modelPath, device, deviceContext, cb_vs_vertexshader))
		return false;

	previousPosition = GetPositionVector();
	previousRotation = GetRotationVector();

	SetPosition(0.0f, 30.0f, 0.0f);
	SetRotation(0.0f, 0.0f, 0.0f);
	UpdateMatrix();

	return true;
}

void Character::MoveForward(float dt)
{
	AdjustPosition(((GetForwardVector() / 100) * speed) * dt);
}

void Character::RotateLeft(float dt)
{
	AdjustRotation(-(GetUpVector() * (speed / 8000)) * dt);
}

void Character::RotateRight(float dt)
{
	AdjustRotation((GetUpVector() * (speed / 8000)) * dt);
}

float Character::GetSpeed()
{
	return speed;
}

void Character::SetSpeed(float speed)
{
	this->speed = speed;
}
