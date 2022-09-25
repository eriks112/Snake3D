#pragma once
#include "..\\Graphics\RenderableGameObject.h"

class Character : public RenderableGameObject
{
public:
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader);

	void MoveForward(float dt);
	void RotateLeft(float dt);
	void RotateRight(float dt);

	float GetSpeed();
	void SetSpeed(float speed);

private:
	std::string modelPath = "Data\\Objects\\Snake2\\Snake_Head.fbx";
	
	float speed = 20;
};