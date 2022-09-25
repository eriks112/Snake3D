#include "Snake3D.h"

bool Snake3D::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader,
	                     std::vector<RenderableGameObject*>* gameObjectList)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->cb_vs_vertexshader = cb_vs_vertexshader;
	this->animator = animator;
	this->gameObjectList = gameObjectList;

	if (!LoadCharacter())
		return false;

	timer.Start();

	return true;
}

void Snake3D::Update(float dt)
{
	if (!game.isGameStarted)
		return;

	UpdateSnakeKinematics(dt);
}

/*
*  Updates the snake childs position.
* 
*  It has 2 vectors containing rotation and position vector.
*  Whenever the character(head) has moved the size of the object it will populate the
*  vectors with its rotation and position data making a path for the children to follow.
* 
*  The vector is the same size as amount of children(snake bodies).
*  The snake child will rotate and move towards its corresponding position/rotation
*  in the vector.
*/
void Snake3D::UpdateSnakeKinematics(float dt)
{
	character.MoveForward(dt); // Moves the character at a constant speed

	static std::vector<XMVECTOR> pathPositionVector; // Vector containing position data
	static std::vector<XMVECTOR> pathRotationVector; // Vector containng rotation data
	static bool firstPositionSet = false;
	static XMFLOAT3 pathPos;

	/* Position/Rotation is inserted into vector if the character has
	   moved 73 units from its previous path point */
	XMFLOAT3 charCurrentPos = character.GetPositionFloat3();
	if (!firstPositionSet)
	{
		pathPos = charCurrentPos;
		firstPositionSet = true;
	}

	// Calculate distance between current and the last path position.
	float lenght = 0.0f;
	lenght = sqrt(((charCurrentPos.x - pathPos.x) * (charCurrentPos.x - pathPos.x)) +
		          ((charCurrentPos.y - pathPos.y) * (charCurrentPos.y - pathPos.y)) +
		          ((charCurrentPos.z - pathPos.z) * (charCurrentPos.z - pathPos.z)));

	// Character has moved 73 units
	if (lenght >= 73)
	{
		pathRotationVector.insert(pathRotationVector.begin(), character.GetRotationVector()); // Adds new rotation into vector
		pathPositionVector.insert(pathPositionVector.begin(), character.GetPositionVector()); // Adds new position into vector
		firstPositionSet = false;
	}

	// Deletes the data added last if vector is same size as number of childs.
	if (pathPositionVector.size() >= snakeChildList.size() + 1 && !pathPositionVector.empty())
	{
		pathRotationVector.pop_back();
		pathPositionVector.pop_back(); // Deletes the last position in vector if path size is same as number of childs
	}

	// Loops trough all the children to update their position and rotation.
	for (size_t i = 0; i < snakeChildList.size(); i++)
	{
		RenderableGameObject* child = snakeChildList.at(i);
		GameObject3D* parent = snakeChildList.at(i)->GetParent();
		XMVECTOR dirVector = child->GetForwardVector(); // direction vector for the child

		// If the path has a valid data for this child
		if (pathPositionVector.size() > i)
		{
			// Creates an animation for the rotation to smoothly move to target position
			AnimationProperty* animateRotation = animator->CreateAnimation(child, AnimationType::Rotation, 0.3 * character.GetSpeed(), pathRotationVector.at(i) - child->GetRotationVector());
			if (animateRotation != nullptr) // Will return nullptr if animation already exists
				animateRotation->Start();

			// Gets the direction vector between current childs position and the target position
			dirVector = pathPositionVector.at(i) - child->GetPositionVector();
			dirVector = XMVector3NormalizeEst(dirVector); // Normalize it to keep it same speed as character
			child->AdjustPosition(((dirVector / 100) * character.GetSpeed()) * dt);
		}
		else
			child->AdjustPosition(((dirVector / 100) * character.GetSpeed()) * dt); // If no valid data in vector, just move forward.	
	}
}

void Snake3D::SetAnimator(AnimationSystem* animator)
{
	this->animator = animator;
}

void Snake3D::SetKeyboard(KeyboardClass* keyboard)
{
	this->keyboard = keyboard;
}

bool Snake3D::LoadCharacter()
{
	// Load character
	if (!character.Initialize(device, deviceContext, cb_vs_vertexshader))
		return false;

	// Load snake middle body model
	if (!snakeBody.Initialize(device, deviceContext, cb_vs_vertexshader))
		return false;

	// Load snake tail body model
	if (!snakeTail.Initialize(device, deviceContext, cb_vs_vertexshader))
		return false;

    // Character loaded successfully
	gameObjectList->push_back(&character);
	game.isCharacterLoaded = true;

	return true;
}

Character* Snake3D::GetCharacter()
{
	return &character;
}

Snake3D::GameStruct Snake3D::Game()
{
	return game;
}

bool Snake3D::CreateSnakeChild()
{
	size_t numChilds = snakeChildList.size();

	/* If list is empty it will just create a tail object
	   When its not empty it will add a tail object and replace the next childs model
	   with the middle body */
	if (!snakeChildList.empty())
	{
		RenderableGameObject* snakeTail = CreateGameObject(&this->snakeTail);
		if (snakeTail == nullptr)
			return false;
		// Find position of parent in order to spawm child at correction position and orientation
		snakeTail->SetParentTracking(false);
		snakeTail->SetParent(snakeChildList.at(numChilds - 1));
		snakeTail->SetPosition(snakeTail->GetParent()->GetPositionVector() - (snakeTail->GetParent()->GetForwardVector() * 73));
		snakeTail->SetRotation(snakeTail->GetParent()->GetRotationVector());
		gameObjectList->push_back(snakeTail);
		snakeChildList.push_back(snakeTail);

		// Set previous child to middle body

		snakeChildList.at(numChilds - 1)->SetModel(snakeBody.GetModel());
		snakeChildList.at(numChilds - 1)->SetWorldMatrix(snakeBody.GetWorldMatrix());

		character.SetSpeed(character.GetSpeed() + 0.4f); // Advance the speed by 0.4
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
		gameObjectList->push_back(snakeTail);
		snakeChildList.push_back(snakeTail);
	}
	return true;
}

RenderableGameObject* Snake3D::CreateGameObject(RenderableGameObject* source, std::string filePath)
{
	RenderableGameObject* gameobject = new RenderableGameObject;

	// Load new model if filepath is given
	if (!filePath.empty())
		if (!gameobject->Initialize(filePath, device, deviceContext, cb_vs_vertexshader))
			return nullptr;

	if (source == nullptr)
		return nullptr;

	// Copy model and matrix from source
	gameobject->SetModel(source->GetModel());
	gameobject->SetWorldMatrix(source->GetWorldMatrix());

	return gameobject;
}