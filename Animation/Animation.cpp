#include "Animation.h"
#include "..\\Graphics/GameObject.h"

AnimationSystem::AnimationSystem()
{
	deltaTimer.Start();
}

void AnimationSystem::Update()
{
	// List is empty, no work to be done
	if (animationList.empty())
		return;

	// Fixes framerate to value set in ANIMATION_FRAMERATE
	if (deltaTimer.GetMilisecondsElapsed() < (1000 / ANIMATION_FRAMERATE)) // 1000ms / 60 = 60FPS
		return;

	// Loops all currently active animations
	for (size_t i = 0; i < animationList.size(); i++)
	{
		auto* animation = animationList.at(i);
		auto* targetObject = animation->Parameter().targetObject;

		// Check if animation is currently running
		if (animation->Parameter().state != AnimationState::Running)
			return;

		/* Incase animation distance is so small that it has less than 1 frame it will just
		*  set the target object to target orientation and delete the animation */
		if (animation->Parameter().numFrames == 0)
		{
			animation->SetState(AnimationState::Finished);
			RemoveAnimation(animation);
			continue;
		}

		// Run a switch case on the type to determine which parameter to animate
		switch (animation->Parameter().type)
		{
		case AnimationType::Position:
		{
			targetObject->AdjustPosition((animation->Parameter().targetFloat3.x / animation->Parameter().numFrames),
				                         (animation->Parameter().targetFloat3.y / animation->Parameter().numFrames),
				                         (animation->Parameter().targetFloat3.z / animation->Parameter().numFrames));
			break;
		}

		case AnimationType::Scale:
		{
			targetObject->AdjustScale((animation->Parameter().targetFloat3.x / animation->Parameter().numFrames),
				                      (animation->Parameter().targetFloat3.y / animation->Parameter().numFrames),
				                      (animation->Parameter().targetFloat3.z / animation->Parameter().numFrames));
			break;
		}
		case AnimationType::Rotation:
		{
			targetObject->AdjustRotation((animation->Parameter().targetFloat3.x / animation->Parameter().numFrames),
			                           	 (animation->Parameter().targetFloat3.y / animation->Parameter().numFrames),
			                           	 (animation->Parameter().targetFloat3.z / animation->Parameter().numFrames));
			break;
		}
		}

		// If animation is not continous it will finish and delete the animation data when the last frame is done
		if ((animation->Parameter().currentFrame >= animation->Parameter().numFrames) && !animation->Parameter().continous)
		{
			SetFinalTarget(animation);
			animation->SetState(AnimationState::Finished);
			RemoveAnimation(animation);
		}
		else
		{
			animation->NextFrame();
		}
	}
	deltaTimer.Restart();
}

void AnimationSystem::PauseAllAnimations()
{
	if (animationList.empty())
		return;

	for (size_t i = 0; i < animationList.size(); i++)
	{
		// If animation was finished, dont change its status to prevent it from being deleted
		if (animationList.at(i)->Parameter().state == AnimationState::Finished)
			continue;

		animationList.at(i)->Pause();
	}
}

void AnimationSystem::ResumeAllAnimations()
{
	if (animationList.empty())
		return;

	for (size_t i = 0; i < animationList.size(); i++)
	{
		// If animation was finished, dont change its status to prevent it from being deleted
		if (animationList.at(i)->Parameter().state == AnimationState::Finished)
			continue;

		animationList.at(i)->Resume();
	}
}

AnimationProperty* AnimationSystem::CreateAnimation(GameObject* targetObject, AnimationType type, int speed, const DirectX::XMFLOAT3 targetFloat3, bool continous)
{
	if (targetObject->IsAnimationActive())
		return nullptr;

	if (targetObject == nullptr)
		return nullptr;

	// Create new animation object
	auto* animation = new AnimationProperty(targetObject, type, speed, targetFloat3, continous);
	targetObject->SetAnimationActive(true);
	animationList.push_back(animation);
	return animation;
}

AnimationProperty* AnimationSystem::CreateAnimation(GameObject* targetObject, AnimationType type, int speed, const DirectX::XMVECTOR targetVector, bool continous)
{
	if (targetObject->IsAnimationActive())
		return nullptr;

	if (targetObject == nullptr)
		return nullptr;

	// Create new animation object
	auto* animation = new AnimationProperty(targetObject, type, speed, targetVector, continous);
	targetObject->SetAnimationActive(true);
	animationList.push_back(animation);
	return animation;
}

bool AnimationSystem::RemoveAnimation(AnimationProperty* animation)
{
	if (animation == nullptr)
		return false;

	// Loops animation list to get animation pointer by pointer
	std::vector<AnimationProperty*>::iterator it;
	for (it = animationList.begin(); it != animationList.end(); it++)
	{
		if (*it == animation) // Animation found - deleting
		{
			animation->Parameter().targetObject->SetAnimationActive(false);
			animationList.erase(it);
			delete animation;
			animation = nullptr;
			return true;
		}
	}

	return false;
}

bool AnimationSystem::RemoveAnimation(GameObject* parentObject)
{
	if (parentObject == nullptr)
		return false;

	// Loops animation list to get animation pointer by object
	std::vector<AnimationProperty*>::iterator it;
	for (it = animationList.begin(); it != animationList.end(); it++)
	{
		auto* ptr = *it;
		if (ptr->Parameter().targetObject == parentObject)
		{
			animationList.erase(it);
			*it = nullptr;
			delete *it;
			return true;
		}
	}

	return false;
}

void AnimationSystem::SetFinalTarget(AnimationProperty* animation)
{
	/* Sets the final position / scale / rotation data to the actual target, this is done when animation is complete.
	*  Must be done due to precision errors in animation  */

	auto* gameObject = animation->Parameter().targetObject;
	DirectX::XMFLOAT3 startData;
	DirectX::XMFLOAT3 finalData = animation->Parameter().targetFloat3;
	
	switch (animation->Parameter().type)
	{
	case AnimationType::Position:
	{
		startData = animation->StartValues().position;
		gameObject->SetPosition(startData.x + finalData.x, startData.y + finalData.y, startData.z + finalData.z);
		break;
	}
	case AnimationType::Rotation:
	{
		startData = animation->StartValues().rotation;
		gameObject->SetRotation(startData.x + finalData.x, startData.y + finalData.y, startData.z + finalData.z);
		break;
	}
	case AnimationType::Scale:
	{
		startData = animation->StartValues().scale;
		gameObject->SetScale(startData.x + finalData.x, startData.y + finalData.y, startData.z + finalData.z);
		break;
		break;
	}
	}
}

AnimationProperty::AnimationProperty(GameObject* targetObject, AnimationType type, int speed, const DirectX::XMFLOAT3& targetFloat3, bool continous)
{
	parameter.targetFloat3 = targetFloat3;
	parameter.targetVector = DirectX::XMLoadFloat3(&parameter.targetFloat3);

	// Setting properties of the animation
	SetProperties(targetObject, type, speed, continous);
}

AnimationProperty::AnimationProperty(GameObject* targetObject, AnimationType type, int speed, const DirectX::XMVECTOR& targetVector, bool continous)
{
	// Sets up new animation parameters
	parameter.targetVector = targetVector;

	DirectX::XMStoreFloat3(&parameter.targetFloat3, parameter.targetVector);

	// Setting properties of the animation
	SetProperties(targetObject, type, speed, continous);
}

void AnimationProperty::Start()
{
	parameter.state = AnimationState::Running;
}

void AnimationProperty::Pause()
{
	parameter.state = AnimationState::Paused;
}

void AnimationProperty::Resume()
{
	parameter.state = AnimationState::Running;
}

void AnimationProperty::SetState(AnimationState state)
{
	parameter.state = state;
}

void AnimationProperty::NextFrame()
{
	parameter.currentFrame++;
}

AnimationProperty::StartValuesStruct AnimationProperty::StartValues()
{
	return startValue;
}

AnimationProperty::ParametersStruct AnimationProperty::Parameter()
{
	return parameter;
}

void AnimationProperty::CalculateAnimationDistance()
{
	/* Using Pythagorean Theorem to calculate the lenght from the target objects current position/rotation/scale to the target one
	   distance = sqrt(lenghtX^2 + lenghtY^2) */

	DirectX::XMFLOAT3 gameObjectFloat3(0.0f, 0.0f, 0.0f);

	// Get the current type of animation
	switch (parameter.type)
	{
	case AnimationType::Position:
		gameObjectFloat3 = parameter.targetObject->GetPositionFloat3();
		break;
	case AnimationType::Rotation:
		//gameObjectFloat3 = animation.targetObject->GetRotationFloat3(); 
		// global orentation used, so just keep gameobject at 0
		break;
	case AnimationType::Scale:
		gameObjectFloat3 = parameter.targetObject->GetScaleFloat3();
		break;
	}

	DirectX::XMFLOAT3 targetFloat3 = parameter.targetFloat3;

	parameter.animationDistance = sqrt(((gameObjectFloat3.x - targetFloat3.x) * (gameObjectFloat3.x - targetFloat3.x)) +
	                                   ((gameObjectFloat3.y - targetFloat3.y) * (gameObjectFloat3.y - targetFloat3.y)) +
	                     	           ((gameObjectFloat3.z - targetFloat3.z) * (gameObjectFloat3.z - targetFloat3.z)));
}

void AnimationProperty::SetProperties(GameObject* targetObject, AnimationType type, int speed, bool continous)
{
	// Sets up new animation parameters
	parameter.targetObject = targetObject;
	parameter.type = type;
	parameter.speed = speed;
	parameter.continous = continous;
	parameter.state = AnimationState::Paused;

	// Setting start values
	startValue.position = targetObject->GetPositionFloat3();
	startValue.rotation = targetObject->GetRotationFloat3();
	startValue.scale = targetObject->GetScaleFloat3();

	CalculateAnimationDistance();

	// Calculates the number of frame of the animation
	float frames = static_cast<float>((parameter.animationDistance / static_cast<float>(speed))) * ANIMATION_FRAMERATE;
	parameter.numFrames = (int)frames; // Precision loss is handled when animation is complete
}
