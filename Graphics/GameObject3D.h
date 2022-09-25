#pragma once
#include "GameObject.h"

class GameObject3D : public GameObject
{
public:
	void SetLookAtPos(XMFLOAT3 lookAtPos);
	const XMVECTOR& GetForwardVector(bool omitY = false);
	const XMVECTOR& GetRightVector(bool omitY = false);
	const XMVECTOR& GetBackwardVector(bool omitY = false);
	const XMVECTOR& GetLeftVector(bool omitY = false);
	const XMVECTOR& GetUpVector();
	const XMVECTOR& GetDownVector();

	// Parenting
	void SetParent(GameObject3D* gameObject);
	void SetParentOffset(const XMFLOAT3& offset);
	void SetParentOffset(float x, float y, float z);
	void SetParentOffset(const XMVECTOR& offset);
	void SetParentRotationOffset(const XMFLOAT3& offset);
	void SetParentRotationOffset(float x, float y, float z);
	void SetParentRotationOffset(const XMVECTOR& offset);
	void AdjustParentRotationOffset(float x, float y, float z);
	void SetParentTracking(bool state);

	const XMFLOAT3& GetParentOffset();
	const XMVECTOR& GetParentOffsetVector();
	const XMFLOAT3& GetParentRotationOffset();
	const XMVECTOR& GetParentRotationOffsetVector();
	const bool IsParentTracking();
	GameObject3D* GetParent();

	typedef std::vector<GameObject3D*> parentList;

	static parentList& GetParentedObjects()
	{
		static parentList list;
		return list;
	};

protected:
	virtual void UpdateMatrix();

	void UpdateDirectionVectors();

	const XMVECTOR DEFAULT_FORWARD_VECTOR = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	const XMVECTOR DEFAULT_BACKWARD_VECTOR = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	const XMVECTOR DEFAULT_UP_VECTOR = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const XMVECTOR DEFAULT_DOWN_VECTOR = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
	const XMVECTOR DEFAULT_LEFT_VECTOR = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
	const XMVECTOR DEFAULT_RIGHT_VECTOR = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMVECTOR vec_forward;
	XMVECTOR vec_left;
	XMVECTOR vec_right;
	XMVECTOR vec_backward;
	XMVECTOR vec_up;
	XMVECTOR vec_down;

	XMVECTOR vec_forward_noY;
	XMVECTOR vec_left_noY;
	XMVECTOR vec_right_noY;
	XMVECTOR vec_backward_noY;

	GameObject3D* parent = nullptr;
	bool trackParentPosition = true;
	XMFLOAT3 parentPositionOffset;
	XMVECTOR parentPositionOffsetVector;
	XMFLOAT3 parentRotationOffsetFloat3;
	XMVECTOR parentRotationOffsetVector;
};