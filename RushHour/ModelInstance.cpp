#include "stdafx.h"
#include "ModelInstance.h"

using namespace std;
using namespace DirectX;

ModelInstance::ModelInstance(Model & m, DirectX::XMMATRIX scale, DirectX::XMVECTOR alignment, DirectX::XMMATRIX rotation) : ModelInstance(m) {
	_scale = scale;
	_alignment = alignment;
	_rotation = rotation;
}

void ModelInstance::SetPosition(Coords2i_t coords) {
	_position = coords;
}

void ModelInstance::SetColor(DirectX::XMUINT3 c) {
	_color = MakeColorVector(c);
}

void ModelInstance::SetColor(DirectX::XMVECTOR c) {
	_color = c;
}
XMVECTOR ModelInstance::MakeColorVector(XMUINT3 rgb) {
	const float k = 1.0f / 255.0f;
	XMVECTOR ret = { rgb.x*k, rgb.y*k, rgb.z*k };
	return ret;
}

// Return rotation matrix reflecting initial orientation of instance based on IsRotated180() and GetOrientation() state.
// It does not reflect world rotation.
DirectX::XMMATRIX ModelInstance::GetInitRotation() const {
	XMMATRIX rMatrix = _rotation;
	if (GetOrientation() == ModelInstance::XAxis) {
		rMatrix *= XMMatrixRotationY(XMConvertToRadians(90));
	}
	return rMatrix;
}

DirectX::XMMATRIX ModelInstance::GetTransformation() const {
	XMMATRIX r = _rotation;
	XMMATRIX p = XMMatrixTranslation(static_cast<float>(GetPosition().x), 0.0f, static_cast<float>(GetPosition().z));
	if (GetOrientation() == ModelInstance::XAxis) {
		// rotate the instance 90degrees
		r *= XMMatrixRotationY(XMConvertToRadians(90.0f));
		// rotate the alignment and movementStep 90degrees
		p *= XMMatrixTranslation(GetAlignment().m128_f32[0], GetAlignment().m128_f32[1], GetAlignment().m128_f32[2]);
	} else {
		// no rotation
		p *= XMMatrixTranslation(GetAlignment().m128_f32[2], GetAlignment().m128_f32[1], GetAlignment().m128_f32[0]);
	}
	return r * GetScale() * p;
}

