#pragma once
#include "GameObjects/Character.h"
#include "GameObjects/CharacterMiddle.h"
#include "GameObjects/CharacterTail.h"
#include "..\\Timer.h"
#include "..\\Animation\Animation.h"

class KeyboardClass;

class Snake3D
{
public:
	struct GameStruct
	{
		bool isCharacterLoaded = false;
		bool isGameStarted = false;
	};

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader,
		            std::vector<RenderableGameObject*>* gameObjectList);
	void Update(float dt);
	void UpdateSnakeKinematics(float dt);
	void SetAnimator(AnimationSystem* animator);
	void SetKeyboard(KeyboardClass* keyboard);
	void LoadScene();
	void KeyboardEvents();
	Character* GetCharacter();

	GameStruct Game();
	
	bool CreateSnakeChild();

private:
	bool LoadCharacter();
	RenderableGameObject* CreateGameObject(RenderableGameObject* source = nullptr, std::string filePath = "");

	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	std::vector<RenderableGameObject*>* gameObjectList;

	Character character;
	CharacterMiddle snakeBody;
	CharacterTail snakeTail;
	std::vector<RenderableGameObject*> snakeChildList;

	KeyboardClass* keyboard;
	Timer timer;
	AnimationSystem* animator = nullptr;

	GameStruct game;
};