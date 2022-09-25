#pragma once
#include <DirectXMath.h>

// float - 4 bytes
// float2 - 8 byte
// float3 - 12 byte
//float4 - 16 byte

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projectionMatrix;
	DirectX::XMMATRIX worldMatrix;
	int isNormalEnabled;
	int isSpecularMapped;
	int isGlossMapped;
};

struct CB_VS_light
{
	DirectX::XMMATRIX lightViewMatrix;
	DirectX::XMMATRIX lightProjectionMatrix;
	DirectX::XMFLOAT3 lightPos;
};

struct CB_VS_cameraBuffer
{
	DirectX::XMFLOAT3 cameraPosition;
	float padding;
	DirectX::XMFLOAT3 inCameraDir;
};

struct CB_VS_fog
{
	float fogStart;
	float fogEnd;
};

struct CB_PS_light
{
	DirectX::XMFLOAT3 ambientLightColor; // 12 byte
	float ambientLightStrenght;          // 4 byte

	DirectX::XMFLOAT3 dynamicLightColor; // 12 byte
	float dynamicLightStrenght;          // 4 byte
	DirectX::XMFLOAT3 dynamicLightPosition; // 12 byte
	float dynamicLightAttenuation_a;        // 4 byte
	DirectX::XMFLOAT3 cameraPosition;       // 12 byte
	float dynamicLightAttenuation_b;        // 4 byte
	float dynamicLightAttenuation_c;        // 4 byte	
};

struct CB_PS_specBuffer
{
	float dynamicSpecularPower;
	DirectX::XMFLOAT3 dynamicSpecularColor;
	DirectX::XMFLOAT3 lightPos;
};

struct CB_VS_depth
{
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projectionMatrix;
	DirectX::XMMATRIX worldMatrix;
};

struct CB_VS_skybox
{
	DirectX::XMMATRIX viewProjectionMatrix;
};