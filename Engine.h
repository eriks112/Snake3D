#pragma once
#include "WindowContainer.h"
#include "Timer.h"
#include "Animation/Animation.h"

class Engine : WindowContainer
{
public:
	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	bool ProcessMessages();
	void Update();
	void RenderFrame();
	void ParentChildPositionUpdater();
	static bool CompareFloat(XMFLOAT3 current, XMFLOAT3 previous, float epsilon = 1.0f);
private:
	Timer timer;
	Timer pickSpawnTimer;

	AnimationSystem animationSystem;
};