#pragma once
#include "Model.h"

class GameObject
{
public:
	const XMVECTOR& GetPositionVector() const;
	const XMFLOAT3& GetPositionFloat3() const;
	const XMVECTOR& GetRotationVector() const;
	const XMFLOAT3& GetRotationFloat3() const;
	const XMFLOAT3& GetScaleFloat3() const;

	void SetPosition(const XMVECTOR& pos);
	void SetPosition(const XMFLOAT3& pos);
	void SetPosition(float x, float y, float z);
	void SetScale(const XMFLOAT3& pos);
	void SetScale(float x, float y, float z);
	void AdjustScale(const XMFLOAT3& pos);
	void AdjustScale(float x, float y, float z);
	void AdjustPosition(const XMVECTOR& pos);
	void AdjustPosition(const XMFLOAT3& pos);
	void AdjustPosition(float x, float y, float z);
	void SetRotation(const XMVECTOR& rot);
	void SetRotation(const XMFLOAT3& rot);
	void SetRotation(float x, float y, float z);
	void AdjustRotation(const XMVECTOR& rot);
	void AdjustRotation(const XMFLOAT3& rot);
	void AdjustRotation(float x, float y, float z);

	XMVECTOR previousRotation;
	XMVECTOR previousPosition;

	// animation
	bool IsAnimationActive();
	void SetAnimationActive(bool state);

	bool movePending = false;
	XMFLOAT3 positionAtTurn;
	bool turnPending = false;

	XMFLOAT3 pos; // move to public so it can be debugged easily, put in protected after

protected:
	virtual void UpdateMatrix();

	XMVECTOR posVector;
	XMVECTOR rotVector;
	
	XMFLOAT3 rot;
	XMFLOAT3 scale;

private:
	bool isAnimationActive = false;

};