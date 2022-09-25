#pragma once
#include <DirectXMath.h>
#include <vector>

#include "..\\Timer.h"

constexpr int ANIMATION_FRAMERATE = 60; // Framerate of the running animations(FPS)

class AnimationProperty;
class GameObject;

enum class AnimationType {
	Scale,
	Position,
	Rotation
};

enum class AnimationState {
	Running,
	Paused,
	Finished // Use for event system later
};

class AnimationSystem
{
public:
	AnimationSystem();
	void Update();

	void PauseAllAnimations();
	void ResumeAllAnimations();

	AnimationProperty* CreateAnimation(GameObject* targetObject, AnimationType type, int speed, const DirectX::XMFLOAT3 targetFloat3, bool continous = false);
	AnimationProperty* CreateAnimation(GameObject* targetObject, AnimationType type, int speed, const DirectX::XMVECTOR targetVector, bool continous = false);

	bool RemoveAnimation(AnimationProperty* animation);
	bool RemoveAnimation(GameObject* parentObject);

private:
	void SetFinalTarget(AnimationProperty* animation);

	Timer deltaTimer;

	std::vector<AnimationProperty*> animationList;
};

class AnimationProperty
{
public:
	struct StartValuesStruct {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 rotation;
		DirectX::XMFLOAT3 scale;
	};

	struct ParametersStruct {
		GameObject* targetObject;
		DirectX::XMFLOAT3 targetFloat3;
		DirectX::XMVECTOR targetVector;

		AnimationType type;
		AnimationState state = AnimationState::Paused;

		int speed, numFrames;
		int currentFrame = 0;
		bool continous;			
		float animationDistance;
	};

	AnimationProperty(GameObject* targetObject, AnimationType type, int speed, const DirectX::XMFLOAT3& targetFloat3, bool continous);
	AnimationProperty(GameObject* targetObject, AnimationType type, int speed, const DirectX::XMVECTOR& targetVector, bool continous);

	void Start();
	void Pause();
	void Resume();

	void SetState(AnimationState state);
	void NextFrame();

	StartValuesStruct StartValues();
	ParametersStruct Parameter();

private:
	void CalculateAnimationDistance();
	void SetProperties(GameObject* targetObject, AnimationType type, int speed, bool continous);

	StartValuesStruct startValue;

	AnimationType animationType;
	ParametersStruct parameter;
};