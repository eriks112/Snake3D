#include "CharacterMiddle.h"

bool CharacterMiddle::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	// Initializing snake body
	if (!model.Initialize(modelPath, device, deviceContext, cb_vs_vertexshader))
		return false;

	SetPosition(0.0f, 30.0f, 0.0f);
	SetRotation(0.0f, 0.0f, 0.0f);
	return true;
}
