#include "stdafx.h"
#include "Marker.h"

using namespace std;
using namespace DirectX;

Marker::Marker(Model & m, DirectX::XMMATRIX scale) : Marker(m) {
	_scale = scale;
}

DirectX::XMMATRIX Marker::GetTransformation() const {
	XMMATRIX r = _rotation;
	XMMATRIX p = XMMatrixTranslation(static_cast<float>(GetPosition().x), static_cast<float>(GetPosition().y), static_cast<float>(GetPosition().z));
	return r * GetScale() * p;
}
