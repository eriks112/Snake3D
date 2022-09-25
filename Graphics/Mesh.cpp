#include "Mesh.h"

Mesh::Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex>& vertices, std::vector<DWORD>& indices, std::vector<Texture> & textures, const DirectX::XMMATRIX& transformMatrix)
{
	this->deviceContext = deviceContext;
	this->textures = textures;
	this->transformMatrix = transformMatrix;

	HRESULT hr = vertexbuffer.Initialize(device, vertices.data(), vertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");

	hr = indexbuffer.Initialize(device, indices.data(), indices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");
}

Mesh::Mesh(const Mesh& mesh)
{
	deviceContext = mesh.deviceContext;
	indexbuffer = mesh.indexbuffer;
	vertexbuffer = mesh.vertexbuffer;
	textures = mesh.textures;
	transformMatrix = mesh.transformMatrix;
}

void Mesh::Draw(ID3D11ShaderResourceView* shaderResource, ID3D11ShaderResourceView* shaderResource2, ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader)
{
	UINT offset = 0;

	ID3D11ShaderResourceView* textureBuf[4] = { nullptr, nullptr, nullptr, nullptr };

	cb_vs_vertexshader->data.isNormalEnabled = 0;
	cb_vs_vertexshader->data.isSpecularMapped = 0;
	cb_vs_vertexshader->data.isGlossMapped = 0;

	for (int i = 0; i < textures.size(); i++)
	{
		if (textures[i].GetType() == aiTextureType::aiTextureType_DIFFUSE)
		{
			textureBuf[0] = textures[i].GetTextureResourceView();
			deviceContext->PSSetShaderResources(1, 1, &shaderResource);
			break;
		}
	}

	for (int i = 0; i < textures.size(); i++)
	{
		if (textures[i].GetType() == aiTextureType::aiTextureType_NORMALS)
		{
			textureBuf[1] = textures[i].GetTextureResourceView();
			cb_vs_vertexshader->data.isNormalEnabled = 1;
			break;
		}
	}

	for (int i = 0; i < textures.size(); i++)
	{
		if (textures[i].GetType() == aiTextureType::aiTextureType_SHININESS)
		{
			textureBuf[2] = textures[i].GetTextureResourceView();
			cb_vs_vertexshader->data.isGlossMapped = 1;
			break;
		}
	}

	cb_vs_vertexshader->ApplyChanges();

	deviceContext->PSSetShaderResources(3, 3, textureBuf);

	// Sets vertex and index buffers then draws the mesh
	deviceContext->IASetVertexBuffers(0, 1, vertexbuffer.GetAddressOf(), vertexbuffer.StridePtr(), &offset);
	deviceContext->IASetIndexBuffer(indexbuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexed(indexbuffer.IndexCount(), 0, 0);
}

const DirectX::XMMATRIX& Mesh::GetTransformMatrix()
{
	return transformMatrix;
}
