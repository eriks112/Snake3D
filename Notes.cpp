// Vector modifying
//DirectX::XMFLOAT3 eyePosFloat3;
//DirectX::XMStoreFloat3(&eyePosFloat3, eyePos);
//eyePosFloat3.y += 0.01f;
//eyePos = DirectX::XMLoadFloat3(&eyePosFloat3);

//cb_vs_vertexshader.data.mat = world * camera.GetViewMatrix() * camera.GetProjectionMatrix();
//constantBuffer.data.mat = DirectX::XMMatrixTranslation(xOffset, yOffset, 0.0f); // Translate
//constantBuffer.data.mat = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, DirectX::XM_PIDIV2); // Rotate
//constantBuffer.data.mat = DirectX::XMMatrixScaling(0.5f, 0.5f, 1); // Scale
//cb_vs_vertexshader.data.mat = DirectX::XMMatrixTranspose(cb_vs_vertexshader.data.mat);
//constantBuffer.data.mat = xOffset;
//constantBuffer.data.yOffset = yOffset;