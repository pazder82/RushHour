#pragma once
#include <DirectXMath.h>
#include "RushHour.h"

class Camera {
public:
	Camera();
	~Camera();

	void SetNewPosition(DirectX::XMMATRIX rotation);

	DirectX::XMVECTOR GetPosition() const { return _camPosition; }
	DirectX::XMMATRIX GetViewMatrix() const;

private:
	DirectX::XMVECTOR _camPosition;
};

