#include "Graphics.h"

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	this->windowWidth = width;
	this->windowHeight = height;
	this->fpsTimer.Start();

	if (!InitializeDirectX(hwnd))
		return false;

	if (!InitializeShaders())
		return false;

	if (!InitializeScene())
		return false;

	//Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(this->device.Get(), this->deviceContext.Get());
	ImGui::StyleColorsDark();

	return true;
}

void Graphics::RenderFrame()
{
	// Setting constant buffers for fog
	cb_vs_fog.data.fogStart = 1000.0f;
	cb_vs_fog.data.fogEnd = 10000.0f;
	cb_vs_fog.ApplyChanges();

	// Setting constant buffers with light position and matrixes
	cb_vs_light.data.lightProjectionMatrix = light.GetProjectionMatrix();
	cb_vs_light.data.lightViewMatrix = light.GetViewMatrix();
	cb_vs_light.data.lightPos = light.GetPositionFloat3();
	cb_vs_light.ApplyChanges();

	// Setting camera buffer
	cb_vs_camera.data.cameraPosition = camera.GetPositionFloat3();
	XMFLOAT3 cameraDir;
	XMStoreFloat3(&cameraDir, camera.GetForwardVector());
	cb_vs_camera.data.inCameraDir = cameraDir;
	cb_vs_camera.ApplyChanges();

	// Setting constant buffers with dynamic light data
	cb_ps_light.data.dynamicLightColor = light.lightColor;
	cb_ps_light.data.dynamicLightStrenght = light.lightStrength;
	cb_ps_light.data.dynamicLightPosition = light.GetPositionFloat3();
	cb_ps_light.data.dynamicLightAttenuation_a = light.attenuation_a;
	cb_ps_light.data.dynamicLightAttenuation_b = light.attenuation_b;
	cb_ps_light.data.dynamicLightAttenuation_c = light.attenuation_c;
	cb_ps_light.data.cameraPosition = camera.GetPositionFloat3();
	cb_ps_light.ApplyChanges();

	cb_ps_specBuffer.data.dynamicSpecularColor = light.dynamicSpecularColor;
	cb_ps_specBuffer.data.dynamicSpecularPower = light.dynamicSpecularPower;
	cb_ps_specBuffer.data.lightPos = light.GetPositionFloat3();
	cb_ps_specBuffer.ApplyChanges();

	// Reset frame
	float bgcolor[] = { 0.7f, 0.80f, 1.0f, 1.0f };
	deviceContext->ClearRenderTargetView(renderTargetView.Get(), bgcolor);
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	deviceContext->IASetInputLayout(depthVertexShader.GetInputLayout());
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->RSSetState(rasterizerState.Get());
	deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);
	deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);

	deviceContext->PSSetConstantBuffers(0, 1, cb_ps_light.GetAddressOf()); // Setting buffers in shader
	deviceContext->PSSetConstantBuffers(1, 1, cb_ps_specBuffer.GetAddressOf()); // Setting buffers in shader
	deviceContext->VSSetConstantBuffers(1, 1, cb_vs_light.GetAddressOf());
	deviceContext->VSSetConstantBuffers(2, 1, cb_vs_fog.GetAddressOf());
	deviceContext->VSSetConstantBuffers(3, 1, cb_vs_camera.GetAddressOf());

	// Render shadow map
	RenderDepthBuffer();

	// Render skybox
	RenderSkybox();

	// Render scene
	RenderScene();

	//Draw Text
	static int fpsCounter = 0;
	static std::string fpsString = "FPS: 0";
	fpsCounter += 1;
	if (fpsTimer.GetMilisecondsElapsed() > 1000.0)
	{
		fpsString = "FPS: " + std::to_string(fpsCounter);
		fpsCounter = 0;
		fpsTimer.Restart();
	}
	std::string scoreString = "Score: ";
	scoreString += std::to_string(score);
	spriteBatch->Begin();
	spriteFont->DrawString(spriteBatch.get(), StringHelper::StringToWide(scoreString).c_str(), DirectX::XMFLOAT2(windowWidth/2 - 100, 0), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(2.0f, 2.0f));
	spriteBatch->End();

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Debug");
	ImGui::DragFloat3("Ambient Light Color", &cb_ps_light.data.ambientLightColor.x, 0.01, 0.0f, 1.0f);
	ImGui::DragFloat3("Ambient Light Strenght", &cb_ps_light.data.ambientLightStrenght, 0.01, 0.0f, 1.0f);
	ImGui::NewLine();
	ImGui::DragFloat3("Dynamic Light Position", &light.pos.x, 0.1f, -5000.0f, 5000.0f);
	ImGui::DragFloat3("Dynamic Light Color", &light.lightColor.x, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat3("Dynamic Specular Color", &light.dynamicSpecularColor.x, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("Dynamic Specular Strength", &light.dynamicSpecularPower, 1.0f, 0.0f, 800.0f);
	ImGui::DragFloat("Dynamic Light Strength", &light.lightStrength, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("Dynamic Light Attenuation A", &light.attenuation_a, 0.01f, 0.1f, 10.0f);
	ImGui::DragFloat("Dynamic Light Attenuation B", &light.attenuation_b, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("Dynamic Light Attenuation C", &light.attenuation_c, 0.01f, 0.0f, 10.0f);
	ImGui::NewLine();
	if (ImGui::Button("Free Camera"))
		debugCameraEnabled = !debugCameraEnabled;
	ImGui::SameLine(200);
	if (ImGui::Button("Follow Camera"))
		EnableThirdPersonCamera(!IsThirdPersonCameraEnabled());
    ImGui::Text("Cam X: %f", camera.GetPositionFloat3().x);
	ImGui::SameLine(200);
	ImGui::Text("Cam Y: %f", camera.GetPositionFloat3().y);
	ImGui::SameLine(400);
	ImGui::Text("Cam Z: %f", camera.GetPositionFloat3().z);
	ImGui::Text("Cam Rot X: %f", camera.GetRotationFloat3().x);
	ImGui::SameLine(200);
	ImGui::Text("Cam Rot Y: %f", camera.GetRotationFloat3().y);
	ImGui::SameLine(400);
	ImGui::Text("Cam Rot Z: %f", camera.GetRotationFloat3().z);
	ImGui::Text("Character Rotation: %f", character.GetRotationFloat3().y);
	ImGui::Text("Character Pos X: %f", character.GetPositionFloat3().x);
	ImGui::Text("Character Pos Y: %f", character.GetPositionFloat3().y);
	ImGui::Text("Character Pos Z: %f", character.GetPositionFloat3().z);
	ImGui::Text(" Charecter Move Pending: %d", character.movePending);
	ImGui::DragFloat("Character Speed", &characterSpeedModifier, 0.1f, 10, 120);
	if (ImGui::Button("Spawm Snake Child"))
		snake3D.CreateSnakeChild();
	ImGui::End();
	//Assemble Together Draw Data
	ImGui::Render();
	//Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	swapchain->Present(1, NULL);   // First argument VSync
}

RenderableGameObject* Graphics::CreateGameObject(RenderableGameObject* source, std::string filePath)
{
	RenderableGameObject* gameobject = new RenderableGameObject;

	// Load new model if filepath is given
	if (!filePath.empty())
		if (!gameobject->Initialize(filePath, device.Get(), deviceContext.Get(), cb_vs_vertexshader))
			return nullptr;

	if (source == nullptr)
		return nullptr;

	// Copy model and matrix from source
	gameobject->SetModel(source->GetModel());
	gameobject->SetWorldMatrix(source->GetWorldMatrix());

	return gameobject;
}

bool Graphics::InitializeDirectX(HWND hwnd)
{
	try
	{
		std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

		if (adapters.size() < 1)
		{
			ErrorLogger::Log("No IDXGI Adapters found.");
			return false;
		}

		DXGI_SWAP_CHAIN_DESC scd = { 0 };

		scd.BufferDesc.Width = windowWidth;
		scd.BufferDesc.Height = windowHeight;
		scd.BufferDesc.RefreshRate.Numerator = 115;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		scd.SampleDesc.Count = 8;
		scd.SampleDesc.Quality = 0;

		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.BufferCount = 1;
		scd.OutputWindow = hwnd;
		scd.Windowed = TRUE;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		HRESULT hr;
		hr = D3D11CreateDeviceAndSwapChain(adapters[0].pAdapter, //IDXGI Adapter
			D3D_DRIVER_TYPE_UNKNOWN,
			NULL, //FOR SOFTWARE DRIVER TYPE
			NULL, //FLAGS FOR RUNTIME LAYERS
			NULL, //FEATURE LEVELS ARRAY
			0, //# OF FEATURE LEVELS IN ARRAY
			D3D11_SDK_VERSION,
			&scd, //Swapchain description
			swapchain.GetAddressOf(), //Swapchain Address
			device.GetAddressOf(), //Device Address
			NULL, //Supported feature level
			deviceContext.GetAddressOf()); //Device Context Address

		COM_ERROR_IF_FAILED(hr, "Failed to create device and swapchain.");

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		COM_ERROR_IF_FAILED(hr, "GetBuffer Failed.");	

		hr = device->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create render target view.");

		//Describe our Depth/Stencil Buffer
		CD3D11_TEXTURE2D_DESC depthStencilTextureDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, windowWidth, windowHeight);
		depthStencilTextureDesc.SampleDesc.Count = 8;
		depthStencilTextureDesc.SampleDesc.Quality = 0;
		depthStencilTextureDesc.MipLevels = 1;
		depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = device->CreateTexture2D(&depthStencilTextureDesc, NULL, this->depthStencilBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil buffer.");

		hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), NULL, depthStencilView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil view.");

		deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

		//Create depth stencil state
		CD3D11_DEPTH_STENCIL_DESC depthstencildesc(D3D11_DEFAULT);
		depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

		hr = device->CreateDepthStencilState(&depthstencildesc, depthStencilState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil state.");

		D3D11_DEPTH_STENCIL_DESC dssDesc;
		ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		dssDesc.DepthEnable = true;
		dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		hr = device->CreateDepthStencilState(&dssDesc, depthStencilStateSky.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil state.");

		//Create & set the Viewport
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight));;
		deviceContext->RSSetViewports(1, &viewport);

		//Create Rasterizer State
		CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
		rasterizerDesc.MultisampleEnable = true;
		hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");

		//Create Rasterizer State for culling front
		CD3D11_RASTERIZER_DESC rasterizerDesc_CullFront(D3D11_DEFAULT);
		rasterizerDesc_CullFront.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
		hr = device->CreateRasterizerState(&rasterizerDesc_CullFront, rasterizerState_CullFront.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");

		CD3D11_RASTERIZER_DESC rasterizerDesc_CullNone(D3D11_DEFAULT);
		rasterizerDesc_CullNone.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
		hr = device->CreateRasterizerState(&rasterizerDesc_CullNone, rasterizerState_CullNone.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");

		//Create Blend State
		D3D11_RENDER_TARGET_BLEND_DESC rtbd = { 0 };
		rtbd.BlendEnable = true;
		rtbd.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		rtbd.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		rtbd.BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

		D3D11_BLEND_DESC blendDesc = { 0 };
		blendDesc.RenderTarget[0] = rtbd;

		hr = device->CreateBlendState(&blendDesc, blendState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create blend state.");

		spriteBatch = std::make_unique<DirectX::SpriteBatch>(deviceContext.Get());
		spriteFont = std::make_unique<DirectX::SpriteFont>(device.Get(), L"Data\\Fonts\\comic_sans_ms_16.spritefont");

		//Create sampler description for sampler state
		CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		hr = device->CreateSamplerState(&sampDesc, samplerState.GetAddressOf()); //Create sampler state
		COM_ERROR_IF_FAILED(hr, "Failed to create sampler state.");

		// Create a clamp texture sampler state description.
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

		// Create the texture sampler state.
		hr = device->CreateSamplerState(&sampDesc, samplerStateClamp.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create sampler clamp state.");

		// Setting samplers
		deviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());
		deviceContext->PSSetSamplers(1, 1, samplerStateClamp.GetAddressOf());
	}
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}

bool Graphics::InitializeShaders()
{

	std::wstring shaderfolder = L"";
#pragma region DetermineShaderPath
	if (IsDebuggerPresent() == TRUE)
	{
#ifdef _DEBUG //Debug Mode
#ifdef _WIN64 //x64
		shaderfolder = L"..\\x64\\Debug\\";
#else  //x86 (Win32)
		shaderfolder = L"..\\Debug\\";
#endif
#else //Release Mode
#ifdef _WIN64 //x64
		shaderfolder = L"..\\x64\\Release\\";
#else  //x86 (Win32)
		shaderfolder = L"..\\Release\\";
#endif
#endif
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	D3D11_INPUT_ELEMENT_DESC layout_depth[] =
	{
		{ "POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = ARRAYSIZE(layout);
	UINT numElementsDepth = ARRAYSIZE(layout_depth);

	if (!vertexshader.Initialize(device, shaderfolder + L"vertexshader.cso", layout, numElements))
		return false;

	if (!pixelshader.Initialize(device, shaderfolder + L"pixelshader.cso"))
		return false;

	if (!pixelshader_nolight.Initialize(device, shaderfolder + L"pixelshader_nolight.cso"))
		return false;

	if (!depthPixelShader.Initialize(device, shaderfolder + L"depth_pixelshader.cso"))
		return false;

	if (!depthVertexShader.Initialize(device, shaderfolder + L"depth_vertexshader.cso", layout_depth, numElementsDepth))
		return false;

	if (!renderTexture.Initialize(device.Get(), 1024*8, 1024*8, 20000, 10.0f)) // Create texture to render shadow map to
		return false;

	if (!skyboxVertexShader.Initialize(device, shaderfolder + L"Skybox_VS.cso"))
		return false;

	if (!skyboxPixelShader.Initialize(device, shaderfolder + L"Skybox_PS.cso"))
		return false;

	return true;
}

bool Graphics::InitializeScene()
{
	try
	{
		//Initialize Constant Buffer(s)
		HRESULT hr = cb_vs_vertexshader.Initialize(device.Get(), deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		hr = cb_vs_light.Initialize(device.Get(), deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		hr = cb_vs_fog.Initialize(device.Get(), deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		hr = cb_ps_light.Initialize(device.Get(), deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		hr = cb_ps_specBuffer.Initialize(device.Get(), deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		cb_ps_light.data.ambientLightColor = XMFLOAT3(0.76f, 0.67f, 0.61f);
		cb_ps_light.data.ambientLightStrenght = 0.48f;

		hr = cb_vs_depth.Initialize(device.Get(), deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		hr = cb_vs_skybox.Initialize(device.Get(), deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		hr = cb_vs_camera.Initialize(device.Get(), deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		// Load skybox texture
		if (!skyboxTexture.Initialize(device.Get(), deviceContext.Get(), "Data\\Textures\\Skybox"))
			return false;
		// Load skybox cube
		skybox.Initialize("Data\\Objects\\Skybox\\skybox.fbx", device.Get(), deviceContext.Get(), cb_vs_vertexshader);
		skybox.SetRotation(0.2617f, 0.0f, 0.0f);

		// Start Game Logic
		if (!snake3D.Initialize(device.Get(), deviceContext.Get(), cb_vs_vertexshader, &gameObjectList))
			return false;
		snake3D.CreateSnakeChild();

		/* ******************************************** Lights ******************************************* */
		if (!light.Initialize(device.Get(), deviceContext.Get(), cb_vs_vertexshader))
			return false;
		light.SetPosition(-3401.0f, 2902.0f, -3907.0f);
		light.SetLookAtPos(XMFLOAT3(0.0f, 2400.0f, 0.0f));
		light.SetProjectionValues(70.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 10.0f, 20000.0f);
		light.SetVisible(false);

		/* ******************************************** Scene ******************************************* */

		RenderableGameObject* scene = new RenderableGameObject;
		if (!scene->Initialize("Data\\Objects\\Scene\\scene.fbx", device.Get(), deviceContext.Get(), cb_vs_vertexshader))
			return false;
		gameObjectList.push_back(scene);

		/* ******************************************** Windmill ******************************************* */

		if (!windmillBlades.Initialize("Data\\Objects\\Windmill\\windmill_blades.fbx", device.Get(), deviceContext.Get(), cb_vs_vertexshader))
			return false;
		windmillBlades.SetPosition(1635.4f, 877.682f, 1818.74f);
		gameObjectList.push_back(&windmillBlades);

		/* ******************************************** Pickups ******************************************* */
		if (!pickupOrb.Initialize("Data\\Objects\\cheese.fbx", device.Get(), deviceContext.Get(), cb_vs_vertexshader))
			return false;

		// Creates the matrix for pickup orbs
		if (!CreatePickupMatrix())
			return false;

		/* ******************************************** Character ***************************************** */
		//if (!character2.Initialize(device.Get(), deviceContext.Get(), cb_vs_vertexshader))
		//	return false;
		//gameObjectList.push_back(&character2);

		if (!character.Initialize("Data\\Objects\\Snake2\\Snake_Head.fbx", device.Get(), deviceContext.Get(), cb_vs_vertexshader))
			return false;
		character.SetPosition(0.0f, 36.0f, 0.0f);
		gameObjectList.push_back(&character);

		// Load Snake Body
		if (!snakeBody.Initialize("Data\\Objects\\Snake2\\Snake_Middle.fbx", device.Get(), deviceContext.Get(), cb_vs_vertexshader))
			return false;

		// Load snake tail
		if (!snakeTail.Initialize("Data\\Objects\\Snake2\\Snake_Tail.fbx", device.Get(), deviceContext.Get(), cb_vs_vertexshader))
			return false;

		if (!CreateSnakeChild())
			return false;

		/* ******************************************** CAMERA ***************************************** */

		if (!cameraRig.Initialize("Data\\Objects\\debug_orb.fbx", device.Get(), deviceContext.Get(), cb_vs_vertexshader))
			return false;
		cameraRig.SetParent(&character);
		cameraRig.SetParentOffset(-350.0f, 300.0f, 0);
		cameraRig.SetParentRotationOffset(0.3f, 0.0f, 0.0f);
		cameraRig.SetVisible(false);
		gameObjectList.push_back(&cameraRig);

		camera.SetLookAtPos(XMFLOAT3(0.0f, 0.0f, 0.0f));
		camera.SetPosition(-161.0f, 1020.0f, -840.0f);
		camera.SetRotation(0.953005f, -0.000630f, 0.0f);
		camera.SetProjectionValues(70.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 5.0f, 30000.0f);
			
    }
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}

bool Graphics::CreateSnakeChild()
{	
	size_t numChilds = snakeBodyList.size();
	
	/* If list is empty it will just create a tail object
	   When its not empty it will add a tail object and replace the next childs model
	   with the middle body */
	if (!snakeBodyList.empty())
	{
		RenderableGameObject* snakeTail = CreateGameObject(&this->snakeTail);
		if (snakeTail == nullptr)
			return false;
		// Find position of parent in order to spawm child at correction position and orientation
		snakeTail->SetParentTracking(false);
		snakeTail->SetParent(snakeBodyList.at(numChilds - 1));		
		snakeTail->SetPosition(snakeTail->GetParent()->GetPositionFloat3());
		gameObjectList.push_back(snakeTail);
		snakeBodyList.push_back(snakeTail);

		// Set previous child to middle body
		
		snakeBodyList.at(numChilds - 1)->SetModel(snakeBody.GetModel());
		snakeBodyList.at(numChilds - 1)->SetWorldMatrix(snakeBody.GetWorldMatrix());
	}
	else
	{
		// Set tail
		RenderableGameObject* snakeTail = CreateGameObject(&this->snakeTail);
		if (snakeTail == nullptr)
			return false;
		snakeTail->SetPosition(0.0f, 36.0f, -73.0f);
		snakeTail->SetParentTracking(false);
		snakeTail->SetParent(&character);	
		gameObjectList.push_back(snakeTail);
		snakeBodyList.push_back(snakeTail);
	}
	return true;
}

void Graphics::SpawnPickup()
{
	size_t i = rand() % pickupOrbList.size();
	if (pickupOrbList.at(i)->IsVisible())
		SpawnPickup();
	pickupOrbList.at(i)->SetVisible(true);
}

void Graphics::EnableThirdPersonCamera(const bool& state)
{
	if (state)
	{
		camera.SetParent(&cameraRig);
		camera.SetRotation(-0.996339, 191.264130, 0);
	}
	else
	{
		camera.SetParent(nullptr);
		// Set to previous position TODO
	}
	thirdPersonCameraEnabled = state;
}

bool Graphics::IsThirdPersonCameraEnabled()
{
	return thirdPersonCameraEnabled;
}

bool Graphics::CreatePickupMatrix()
{
	// Set X/Z axis array
	for (size_t i = 0; i < 23; i++)
	{
		for (size_t j = 0; j < 24; j++)
		{
			RenderableGameObject* _pickupOrb = CreateGameObject(&pickupOrb);
			if (_pickupOrb == nullptr)
				return false;

			_pickupOrb->SetVisible(false);
			_pickupOrb->SetPosition(-930.0f + (80.0f * j), 5, 930.0f + (-85.0f * i));
			pickupOrbList.push_back(_pickupOrb);
			gameObjectList.push_back(_pickupOrb);
		}
	}

	return true;
}

void Graphics::Render(RenderableGameObject* gameObject, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix,
	                  ID3D11ShaderResourceView* shaderResource, ID3D11ShaderResourceView* shaderResource2)
{
	gameObject->Draw(viewMatrix, projectionMatrix, shaderResource, shaderResource2);
}

void Graphics::RenderDepthBuffer()
{
	{
		// Setting shaders to depth mode
		deviceContext->VSSetShader(depthVertexShader.GetShader(), NULL, 0);
		deviceContext->PSSetShader(depthPixelShader.GetShader(), NULL, 0);

		//deviceContext->VSSetConstantBuffers(0, 1, cb_vs_depth.GetAddressOf()); 
		deviceContext->VSSetConstantBuffers(0, 1, cb_vs_vertexshader.GetAddressOf());

		// Render to texture
		renderTexture.SetRenderTarget(deviceContext.Get());
		renderTexture.ClearRenderTarget(deviceContext.Get(), 0.0f, 0.0f, 0.0f, 1.0f);

		for (int i = 0; i < gameObjectList.size(); i++)
		{
			if (!gameObjectList.at(i)->IsVisible()) // Dont render objects that arent visible
				continue;

			Render(gameObjectList.at(i), light.GetViewMatrix(), light.GetProjectionMatrix(), renderTexture.GetShaderResourceView(), skyboxTexture.GetResourceView());

			cb_vs_depth.data.viewMatrix = cb_vs_vertexshader.data.viewMatrix;
			cb_vs_depth.data.projectionMatrix = cb_vs_vertexshader.data.projectionMatrix;
			cb_vs_depth.data.worldMatrix = cb_vs_vertexshader.data.worldMatrix;
			cb_vs_depth.ApplyChanges();
		}
	}
}

void Graphics::RenderSkybox()
{
	// Setting rending target and view to default camera
	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
	CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight));;
	deviceContext->RSSetViewports(1, &viewport);

	cb_vs_skybox.data.viewProjectionMatrix = camera.GetViewMatrix() * camera.GetProjectionMatrix();
	cb_vs_skybox.ApplyChanges();
	deviceContext->RSSetState(rasterizerState_CullNone.Get());
	deviceContext->OMSetDepthStencilState(depthStencilStateSky.Get(), 0);
	deviceContext->VSSetConstantBuffers(0, 1, cb_vs_skybox.GetAddressOf()); // Setting buffers in shader
	deviceContext->VSSetShader(skyboxVertexShader.GetShader(), NULL, 0);
	deviceContext->PSSetShader(skyboxPixelShader.GetShader(), NULL, 0);
	deviceContext->PSSetShaderResources(2, 1, skyboxTexture.GetResourceViewAddress());

	skybox.Draw(camera.GetViewMatrix(), camera.GetProjectionMatrix(), renderTexture.GetShaderResourceView(), skyboxTexture.GetResourceView());
}

void Graphics::RenderScene()
{
	deviceContext->RSSetState(rasterizerState.Get());
	deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);
	deviceContext->IASetInputLayout(vertexshader.GetInputLayout());
	deviceContext->VSSetConstantBuffers(0, 1, cb_vs_vertexshader.GetAddressOf()); // Setting buffers in shader
	deviceContext->VSSetShader(vertexshader.GetShader(), NULL, 0);
	deviceContext->PSSetShader(pixelshader.GetShader(), NULL, 0);
	{
		for (int i = 0; i < gameObjectList.size(); i++)
		{
			if (!gameObjectList.at(i)->IsVisible()) // Dont render objects that arent visible
				continue;

			Render(gameObjectList.at(i), camera.GetViewMatrix(), camera.GetProjectionMatrix(), renderTexture.GetShaderResourceView(), skyboxTexture.GetResourceView());
		}

		{
			//deviceContext->PSSetShader(pixelshader_nolight.GetShader(), NULL, 0);
			//Render(&light, camera.GetViewMatrix(), camera.GetProjectionMatrix(), renderTexture.GetShaderResourceView(), skyboxTexture.GetResourceView());
		}
	}
}
