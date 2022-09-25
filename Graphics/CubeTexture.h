#pragma once
#include "Texture.h"

class CubeTexture
{

public:
	CubeTexture() {};

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string& path);
	ID3D11Texture2D* Get();
	ID3D11ShaderResourceView* GetResourceView();
	ID3D11ShaderResourceView** GetResourceViewAddress();

private:
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	std::string path;

	ID3D11Texture2D* cubeTexture = NULL;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
};