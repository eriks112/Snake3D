#include "CubeTexture.h"
#include "..\\StringHelper.h"

bool CubeTexture::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string& path)
{
	this->device = device;
	this->deviceContext = deviceContext;

	// Load textures for cube faces
	ID3D11Texture2D* tex[6] = { nullptr, nullptr, nullptr,nullptr, nullptr, nullptr };
	for (int i = 0; i < 6; i++)
	{
		std::string fullPath = path + "\\" + std::to_string(i) + ".png";
		HRESULT hr = DirectX::CreateWICTextureFromFileEx(device, StringHelper::StringToWide(fullPath).c_str(), 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE, 0,
			DirectX::WIC_LOADER_FLAGS::WIC_LOADER_DEFAULT,
			(ID3D11Resource**)&tex[i], 0);
		if (FAILED(hr))
			return false;
	}

	D3D11_TEXTURE2D_DESC texDesc1 = {};
	tex[0]->GetDesc(&texDesc1);

	// Texture descriptor
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = texDesc1.Width;
	textureDesc.Height = texDesc1.Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	//The Shader Resource view description
	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc = {};
	SMViewDesc.Format = textureDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = textureDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	device->CreateTexture2D(&textureDesc, NULL, &cubeTexture);
	for (int i = 0; i < 6; i++)
	{
		for (UINT mipLevel = 0; mipLevel < textureDesc.MipLevels; ++mipLevel)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			HRESULT hr = (deviceContext->Map(tex[i], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));
			if (FAILED(hr))
				return false;
			deviceContext->UpdateSubresource(cubeTexture,
				D3D11CalcSubresource(mipLevel, i, textureDesc.MipLevels),
				0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

			deviceContext->Unmap(tex[i], mipLevel);
		}
	}

	// Release buffer
	for (int i = 0; i < 6; i++)
	{
		tex[i]->Release();
	}

	device->CreateShaderResourceView(cubeTexture, &SMViewDesc, textureView.GetAddressOf());

	return true;
}

ID3D11Texture2D* CubeTexture::Get()
{
	return cubeTexture;
}

ID3D11ShaderResourceView* CubeTexture::GetResourceView()
{
	return textureView.Get();
}

ID3D11ShaderResourceView** CubeTexture::GetResourceViewAddress()
{
	return textureView.GetAddressOf();
}
