#include "Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string  window_class, int width, int height)
{
	timer.Start();
	pickSpawnTimer.Start();

	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
		return false;

	if (!gfx.Initialize(this->render_window.GetHWND(), width, height))
		return false;

	// Animation Test
	AnimationProperty* rotateWindmill = animationSystem.CreateAnimation(&gfx.windmillBlades, AnimationType::Rotation, 1, XMFLOAT3(0.0f, 0.0f, XM_2PI), true);
	if (rotateWindmill == nullptr)
		OutputDebugStringA("Animation failed to create\n");
	else
		OutputDebugStringA("Created animation\n");
	rotateWindmill->Start();

	// Pass anomator pointer to game logic
	gfx.snake3D.SetAnimator(&animationSystem);
	// Pass keyboard pointer to game logic
	gfx.snake3D.SetKeyboard(&keyboard);

	return true;
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update()
{
	float dt = timer.GetMilisecondsElapsed();
	timer.Restart();

	// Updates the childs position in relation to parent
	ParentChildPositionUpdater();

	// Animation Updates
	animationSystem.Update();

	// Game Updates
	gfx.snake3D.Update(dt);

	while (!keyboard.CharBufferIsEmpty())
	{
		unsigned char ch = keyboard.ReadChar();
	}

	while (!keyboard.KeyBufferIsEmpty())
	{
		KeyboardEvent kbe = keyboard.ReadKey();
		unsigned char keycode = kbe.GetKeyCode();
	}

	while (!mouse.EventBufferIsEmpty())
	{
		MouseEvent me = mouse.ReadEvent();
		if (!gfx.debugCameraEnabled)
			break;
		if (mouse.IsRightDown())
		{
			if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				this->gfx.camera.AdjustRotation((float)me.GetPosY() * 0.01f, (float)me.GetPosX() * 0.01f, 0);
			}
		}
	}

	float cameraSpeed = 0.4f;
	//this->gfx.gameObject.AdjustRotation(0.0f, 0.001f * dt, 0.0f);
	if (gfx.debugCameraEnabled)
	{
		if (keyboard.KeyIsPressed(VK_SHIFT))
		{
			cameraSpeed = 0.8f;
		}

		if (keyboard.KeyIsPressed('W'))
		{
			this->gfx.camera.AdjustPosition(this->gfx.camera.GetForwardVector() * cameraSpeed * dt);
		}
		if (keyboard.KeyIsPressed('S'))
		{
			this->gfx.camera.AdjustPosition(this->gfx.camera.GetBackwardVector() * cameraSpeed * dt);
		}
		if (keyboard.KeyIsPressed('A'))
		{
			this->gfx.camera.AdjustPosition(this->gfx.camera.GetLeftVector() * cameraSpeed * dt);
		}
		if (keyboard.KeyIsPressed('D'))
		{
			this->gfx.camera.AdjustPosition(this->gfx.camera.GetRightVector() * cameraSpeed * dt);
		}
		if (keyboard.KeyIsPressed(VK_SPACE))
		{
			this->gfx.camera.AdjustPosition(0.0f, cameraSpeed * dt, 0.0f);
		}
		if (keyboard.KeyIsPressed('Z'))
		{
			this->gfx.camera.AdjustPosition(0.0f, -cameraSpeed * dt, 0.0f);
		}

		if (keyboard.KeyIsPressed('C'))
		{
			XMVECTOR lightPosition = this->gfx.camera.GetPositionVector();
			lightPosition += this->gfx.camera.GetForwardVector();
			this->gfx.light.SetPosition(lightPosition);
			this->gfx.light.SetRotation(this->gfx.camera.GetRotationFloat3());
		}

		return;
	}

	if (gfx.IsThirdPersonCameraEnabled())
	{
		if (keyboard.KeyIsPressed(VK_LEFT))
			gfx.camera.AdjustParentRotationOffset(0.0f, (-cameraSpeed * dt) / 200, 0.0f);

		if (keyboard.KeyIsPressed(VK_RIGHT))
			gfx.cameraRig.AdjustParentRotationOffset(0.0f, (cameraSpeed * dt) / 200, 0.0f);
	}

	/* Start Game */
	static bool gameStarted = false;
	static bool gameOver = false;
	static bool wasTurned = false;

	if (keyboard.KeyIsPressed(VK_RETURN))
	{
		gameStarted = true;
	}
	if (!gameStarted)// || gameOver)
		return;

	// Charecter 2 test
	//gfx.snake3D.GetCharacter()->MoveForward(dt);

	if (keyboard.KeyIsPressed('D'))
	{
		gfx.snake3D.GetCharacter()->RotateRight(dt);
	}
	if (keyboard.KeyIsPressed('A'))
	{
		gfx.snake3D.GetCharacter()->RotateLeft(dt);
	}

	/* Game over handling - If characters goes beyond play area then game over*/
	if (gfx.character.GetPositionFloat3().x > 980.0 || gfx.character.GetPositionFloat3().x < -980)
		gameOver = true;
	if (gfx.character.GetPositionFloat3().z > 980.0 || gfx.character.GetPositionFloat3().x < -980)
		gameOver = true;

	// Handle pickup spawns
	int randomTime = rand() % 6000 + 4500;
	if (pickSpawnTimer.GetMilisecondsElapsed() > randomTime)
	{
		gfx.SpawnPickup();
		pickSpawnTimer.Restart();
	}

	// Handle pickup collisions
	for (size_t i = 0; i < gfx.pickupOrbList.size(); i++)
	{
		if (Engine::CompareFloat(gfx.pickupOrbList.at(i)->GetPositionFloat3(), gfx.character.GetPositionFloat3(), 40) && gfx.pickupOrbList.at(i)->IsVisible())
		{   // Orb was picked up
			gfx.pickupOrbList.at(i)->SetVisible(false);
			gfx.score++;
			gfx.CreateSnakeChild();
			gfx.characterSpeedModifier += 0.2f;
		}
	}

	// Handling Child movement/collisions
	for (size_t i = 0; i < gfx.snakeBodyList.size(); i++)
	{
		GameObject3D* parent = gfx.snakeBodyList.at(i)->GetParent();

		// Handle head/child collisions
		if (Engine::CompareFloat(gfx.character.GetPositionFloat3(), gfx.snakeBodyList.at(i)->GetPositionFloat3(), 73/2))
			gameOver = true;

		// Body moves in parents fwd vector if it doesnt change direction
		if (!parent->movePending)
		{
			XMVECTOR targetVector = parent->GetForwardVector();
			//gfx.snakeBodyList.at(i)->SetRotation(parent->GetRotationVector());
			gfx.snakeBodyList.at(i)->AdjustPosition(((targetVector / 100) * gfx.characterSpeedModifier) * dt);
		}

		// Body moves straight while parent is changing direction
		if (parent->movePending)
		{
			XMVECTOR targetVector = gfx.snakeBodyList.at(i)->GetForwardVector();
			gfx.snakeBodyList.at(i)->AdjustPosition(((targetVector / 100) * gfx.characterSpeedModifier) * dt);

			// Save position of parent when it turns
			if (!gfx.snakeBodyList.at(i)->turnPending)
			{
				gfx.snakeBodyList.at(i)->positionAtTurn = parent->GetPositionFloat3(); 
				gfx.snakeBodyList.at(i)->turnPending = true;
			}		

			// Checks if body has entered where parent turned
			if (gfx.snakeBodyList.at(i)->turnPending)
			{
				XMFLOAT3 currentDirection;
				bool passedTarget = false;
				XMStoreFloat3(&currentDirection, gfx.snakeBodyList.at(i)->GetForwardVector());

				// Check if child has passed its target depending on its direction
				if (currentDirection.x > 0.1f)
					if (gfx.snakeBodyList.at(i)->positionAtTurn.x - gfx.snakeBodyList.at(i)->GetPositionFloat3().x < 0)
						passedTarget = true;

				if (currentDirection.x < -0.1f)
					if (gfx.snakeBodyList.at(i)->GetPositionFloat3().x - gfx.snakeBodyList.at(i)->positionAtTurn.x < 0)
						passedTarget = true;

				if (currentDirection.z > 0.1f)
					if (gfx.snakeBodyList.at(i)->positionAtTurn.z - gfx.snakeBodyList.at(i)->GetPositionFloat3().z < 0)
						passedTarget = true;

				if (currentDirection.z < -0.1f)
					if (gfx.snakeBodyList.at(i)->GetPositionFloat3().z - gfx.snakeBodyList.at(i)->positionAtTurn.z < 0)
						passedTarget = true;


				// Check if FWD vector exceeds 73

				if (passedTarget)
				{ // Has moved 73 paces
					// Sets last childs movepending to false
					if (i == gfx.snakeBodyList.size() - 1) // Last child
						gfx.snakeBodyList.at(i)->movePending = false;

					gfx.snakeBodyList.at(i)->movePending = true;
					parent->movePending = false;	

					gfx.snakeBodyList.at(i)->turnPending = false;
	    			gfx.snakeBodyList.at(i)->SetRotation(parent->GetRotationVector()); 

					// Adjust position of child to perfectly match behind parent
					XMFLOAT3 parentDirection;
					XMStoreFloat3(&parentDirection, parent->GetForwardVector());
					if (parentDirection.z > 0.1f)
						gfx.snakeBodyList.at(i)->SetPosition(parent->GetPositionFloat3().x, parent->GetPositionFloat3().y, parent->GetPositionFloat3().z - 73.0f);
					else if (parentDirection.z < -0.1f)
						gfx.snakeBodyList.at(i)->SetPosition(parent->GetPositionFloat3().x, parent->GetPositionFloat3().y, parent->GetPositionFloat3().z + 73.0f);
					else if (parentDirection.x > 0.1f)
						gfx.snakeBodyList.at(i)->SetPosition(parent->GetPositionFloat3().x - 73.0f, parent->GetPositionFloat3().y, parent->GetPositionFloat3().z);
					else if (parentDirection.x < -0.1f)
						gfx.snakeBodyList.at(i)->SetPosition(parent->GetPositionFloat3().x + 73.0f, parent->GetPositionFloat3().y, parent->GetPositionFloat3().z);
						
					wasTurned = true;
				}
			}
		}
	}

	// Move Character - Speed is modified with characterSpeedModifier
	gfx.character.AdjustPosition(((gfx.character.GetForwardVector() / 100) * gfx.characterSpeedModifier) * dt);

	if (wasTurned)
	{
		wasTurned = false;
		return;
	}

	if (gfx.character.movePending)
		return;

	if (gfx.character.IsAnimationActive())
	{
		return;
	}

	/* ******* Character Movement ******* */
	static std::string characterDirection = "UP";
	if (keyboard.KeyIsPressed('D') && characterDirection != "LEFT" && characterDirection != "RIGHT")
	{
		if (gfx.IsThirdPersonCameraEnabled())
		{
			AnimationProperty* turnRightAnim = animationSystem.CreateAnimation(&gfx.character, AnimationType::Rotation, 12, XMFLOAT3(0.0f, XM_PI / 2.0f, 0.0f));
			if (turnRightAnim == nullptr)
				return;
			turnRightAnim->Start();
			//gfx.character.AdjustRotation(0.0f, XM_PI / 2, 0.0f);
			gfx.character.movePending = true;
			return;
		}
		gfx.character.SetRotation(0.0f, XM_PI / 2.0f, 0.0f);
		characterDirection = "RIGHT";
		gfx.character.movePending = true;
	}
	if (keyboard.KeyIsPressed('A') && characterDirection != "RIGHT" && characterDirection != "LEFT")
	{
		if (gfx.IsThirdPersonCameraEnabled())
		{
			AnimationProperty* turnLeftAnim = animationSystem.CreateAnimation(&gfx.character, AnimationType::Rotation, 12, XMFLOAT3(0.0f, -XM_PI / 2.0f, 0.0f));
			if (turnLeftAnim == nullptr)
				return;
			turnLeftAnim->Start();
			//gfx.character.AdjustRotation(0.0f, -XM_PI / 2, 0.0f);
			gfx.character.movePending = true;
			return;
		}
		gfx.character.SetRotation(0.0f, -XM_PI / 2.0f, 0.0f);		
		characterDirection = "LEFT";
		gfx.character.movePending = true;
	}

	if (keyboard.KeyIsPressed('W') && characterDirection != "UP" && characterDirection != "DOWN")
	{
		if (gfx.IsThirdPersonCameraEnabled())
			return;
		gfx.character.SetRotation(0.0f, 0.0f, 0.0f);		
		characterDirection = "UP";
		gfx.character.movePending = true;
	}

	if (keyboard.KeyIsPressed('S') && characterDirection != "UP" && characterDirection != "DOWN")
	{
		if (gfx.IsThirdPersonCameraEnabled())
			return;
		gfx.character.SetRotation(0.0f, XM_PI, 0.0f);	
		characterDirection = "DOWN";
		gfx.character.movePending = true;
	}
}

void Engine::RenderFrame()
{
	gfx.RenderFrame();
}

void Engine::ParentChildPositionUpdater()
{
	for (size_t i = 0; i < GameObject3D::GetParentedObjects().size(); i++)
	{
		GameObject3D* parent = GameObject3D::GetParentedObjects().at(i)->GetParent();
		GameObject3D* gameObject = GameObject3D::GetParentedObjects().at(i);
		if (parent != nullptr && gameObject->IsParentTracking())
		{
			// Set position of child object to parent with offsets added.
			gameObject->SetPosition(parent->GetPositionVector() + (
				(parent->GetForwardVector() * gameObject->GetParentOffset().x) +
				(parent->GetUpVector() * gameObject->GetParentOffset().y) +
				(parent->GetRightVector() * gameObject->GetParentOffset().z)));

			// Set rotation of child object to parent with offsets added.
			gameObject->SetRotation(parent->GetRotationVector() + gameObject->GetParentRotationOffsetVector());
		}
	}
}

bool Engine::CompareFloat(XMFLOAT3 current, XMFLOAT3 previous, float epsilon) {
	float largestXValue = 0;
	float smallestXValue = 0;
	float largestZValue = 0;
	float smallestZValue = 0;

	// Get largest X value
	if (current.x > previous.x)
	{
		largestXValue = current.x;
		smallestXValue = previous.x;
	}
	else
	{
		largestXValue = previous.x;
		smallestXValue = current.x;
	}

	// Get largest Z value
	if (current.z > previous.z)
	{
		largestZValue = current.z;
		smallestZValue = previous.z;
	}
	else
	{
		largestZValue = previous.z;
		smallestZValue = current.z;
	}

	if ((fabs(largestXValue - smallestXValue) < epsilon) && (fabs(largestZValue - smallestZValue) < epsilon))
	{
		return true; //they are same
	}
	return false; //they are not same
}
