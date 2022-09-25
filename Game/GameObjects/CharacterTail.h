#pragma once
#include "..\\Graphics\RenderableGameObject.h"

class CharacterTail : public RenderableGameObject
{
public:
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader);

private:
	std::string modelPath = "Data\\Objects\\Snake2\\Snake_Tail.fbx";
};