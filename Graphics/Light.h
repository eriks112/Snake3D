#pragma once
#include "RenderableGameObject.h"

class Light : public RenderableGameObject
{
public:
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader);
	void UpdateMatrix() override;

	void SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ);

	const XMMATRIX& GetViewMatrix() const;
	const XMMATRIX& GetProjectionMatrix() const;

	DirectX::XMFLOAT3 lightColor = DirectX::XMFLOAT3(0.89f, 0.790f, 0.950f);
	float lightStrength = 1.98f;
	float attenuation_a = 1.0f;
	float attenuation_b = 0.0f;
	float attenuation_c = 0.0f;
	float dynamicSpecularPower = 160.0f;
	DirectX::XMFLOAT3 dynamicSpecularColor = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);

private:
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
};