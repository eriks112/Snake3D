#pragma once
#include "GameObject3D.h"

class RenderableGameObject : public GameObject3D
{
public:
	bool Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader);
	RenderableGameObject();

	void Draw(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* shaderResource, ID3D11ShaderResourceView* shaderResource2);
	void SetModel(const Model& model);
	void SetWorldMatrix(const XMMATRIX& worldMatrix);
	Model GetModel();
	XMMATRIX GetWorldMatrix();

	bool IsVisible();
	void SetVisible(const bool& state);
	
protected:
	Model model;
	void UpdateMatrix() override;

	XMMATRIX worldMatrix = XMMatrixIdentity();

	bool isVisible = true;
};