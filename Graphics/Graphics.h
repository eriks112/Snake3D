#pragma once
#include "AdapterReader.h"
#include "Shaders.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "Camera3D.h"
#include "..\\Timer.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "RenderableGameObject.h"
#include "Light.h"
#include "..\\Game\Snake3D.h"
#include "RenderTextureClass.h"
#include "CubeTexture.h"

class Graphics
{
public:
	bool Initialize(HWND hwnd, int width, int height);
	void RenderFrame();
	RenderableGameObject* CreateGameObject(RenderableGameObject* source = nullptr, std::string filePath = "");

	Camera3D camera;
	RenderableGameObject cameraRig;
	Light light;
	Light light2;
	Snake3D snake3D;

	bool debugCameraEnabled = false;

	RenderableGameObject character;
	RenderableGameObject windmillBlades;
	std::vector<RenderableGameObject*> snakeBodyList;
	std::vector<RenderableGameObject*> gameObjectList;
	std::vector<RenderableGameObject*> pickupOrbList;
	float characterSpeedModifier = 20.0f;
	int score = 0;
	bool CreateSnakeChild();
	void SpawnPickup();
	void EnableThirdPersonCamera(const bool& state);
	bool IsThirdPersonCameraEnabled();

private:
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool InitializeScene();
	bool CreatePickupMatrix();
	void Render(RenderableGameObject* gameObject, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix,
		        ID3D11ShaderResourceView* shaderResource, ID3D11ShaderResourceView* shaderResource2);

	void RenderDepthBuffer();
	void RenderSkybox();

	void RenderScene();

	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

	VertexShader vertexshader;
	PixelShader pixelshader;
	PixelShader pixelshader_nolight;
	DepthVertexShader depthVertexShader;
	DepthPixelShader depthPixelShader;
	SkyboxPixelShader skyboxPixelShader;
	SkyboxVertexShader skyboxVertexShader;
	
	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	ConstantBuffer<CB_VS_light> cb_vs_light;
	ConstantBuffer<CB_PS_specBuffer> cb_ps_specBuffer;
	ConstantBuffer<CB_VS_fog> cb_vs_fog;
	ConstantBuffer<CB_VS_cameraBuffer> cb_vs_camera;

	ConstantBuffer<CB_PS_light> cb_ps_light;
	ConstantBuffer<CB_VS_depth> cb_vs_depth;
	ConstantBuffer<CB_VS_skybox> cb_vs_skybox;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStateSky;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_CullFront;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_CullNone;

	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStateClamp;

	int windowWidth = 0;
	int windowHeight = 0;
	Timer fpsTimer;

	RenderableGameObject snakeBody;
	RenderableGameObject pickupOrb;
	RenderableGameObject snakeTail;
	RenderableGameObject skybox;

	bool thirdPersonCameraEnabled = false;

	RenderTextureClass renderTexture;

	CubeTexture skyboxTexture;
};