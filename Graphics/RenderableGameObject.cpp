#include "RenderableGameObject.h"

bool RenderableGameObject::Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	if (!model.Initialize(filePath, device, deviceContext, cb_vs_vertexshader))
		return false;

	UpdateMatrix();
	return true;
}

RenderableGameObject::RenderableGameObject()
{
	SetPosition(0.0f, 0.0f, 0.0f);
	SetRotation(0.0f, 0.0f, 0.0f);
	SetScale(1.0f, 1.0f, 1.0f);
	SetParentOffset(0.0f, 0.0f, 0.0f);
	SetParentRotationOffset(0.0f, 0.0f, 0.0f);
}

void RenderableGameObject::Draw(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* shaderResource, ID3D11ShaderResourceView* shaderResource2)
{
	model.Draw(worldMatrix, viewMatrix, projectionMatrix, shaderResource, shaderResource2);
}

void RenderableGameObject::SetModel(const Model& model)
{
	this->model = model;
}

void RenderableGameObject::SetWorldMatrix(const XMMATRIX& worldMatrix)
{
	this->worldMatrix = worldMatrix;
	UpdateMatrix();
}

Model RenderableGameObject::GetModel()
{
	return model;
}

XMMATRIX RenderableGameObject::GetWorldMatrix()
{
	return worldMatrix;
}

bool RenderableGameObject::IsVisible()
{
	return isVisible;
}

void RenderableGameObject::SetVisible(const bool& state)
{
	isVisible = state;
}

void RenderableGameObject::UpdateMatrix()
{
	worldMatrix = XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) * XMMatrixTranslation(pos.x, pos.y, pos.z);
	UpdateDirectionVectors();
}