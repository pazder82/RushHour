#include "stdafx.h"
#include "VehiclePicker.h"
#include "RushHour.h"

using namespace DirectX;
using namespace std;

VehiclePicker::~VehiclePicker() {
}

// Create picking ray from camera position and mouse click
void VehiclePicker::SetPickingRay(LONG mouseX, LONG mouseY) {
	float pointX, pointY;

	// Move the mouse cursor coordinates into the -1 to +1 range.
	pointX = ((2.0f * static_cast<float>(mouseX)) / static_cast<float>(SCREEN_WIDTH)) - 1.0f;
	pointY = (((2.0f * static_cast<float>(mouseY)) / static_cast<float>(SCREEN_HEIGHT)) - 1.0f) * -1.0f;

	// Adjust the points using the projection matrix to account for the aspect ratio of the viewport.
	XMFLOAT4X4 prMtrx;
	// Convert projectionMatrix from XMMATRIX into XMFLOAT4X4 in order to pick its members
	XMStoreFloat4x4(&prMtrx, _d3d->GetProjectionMatrix());
	pointX = pointX / prMtrx._11;
	pointY = pointY / prMtrx._22;

	// Get the inverse of the view matrix.
	XMFLOAT4X4 invViewMtrx;
	// Convert inverseViewMatrix from XMMATRIX into XMFLOAT4X4 in order to pick its members
	XMStoreFloat4x4(&invViewMtrx, XMMatrixInverse(nullptr, _camera->GetViewMatrix()));

	// Calculate the direction of the picking ray in view space.
	XMFLOAT3 dir;
	dir.x = (pointX * invViewMtrx._11) + (pointY * invViewMtrx._21) + invViewMtrx._31;
	dir.y = (pointX * invViewMtrx._12) + (pointY * invViewMtrx._22) + invViewMtrx._32;
	dir.z = (pointX * invViewMtrx._13) + (pointY * invViewMtrx._23) + invViewMtrx._33;
	_pickingRayDirection = XMLoadFloat3(&dir);

	// Get the origin of the picking ray which is the position of the camera.
	_pickingRayOrigin = _camera->GetPosition();
}

// Convert modelVertices into array of XMFLOAT3
vector<XMFLOAT3> VehiclePicker::GetXMFLOAT3VectorFromModelVertices(const std::vector<VERTEX>& modelVertices) const {
	vector<XMFLOAT3> xmfloatVector;
	xmfloatVector.reserve(modelVertices.size());
	for (auto modelVertex : modelVertices) {
		xmfloatVector.push_back(modelVertex.pos);
	}
	return xmfloatVector;
}

// Create bounding box for each displayed vehicle
void VehiclePicker::InitBoundingBoxes(std::map<std::string, Vehicle>* pVehicles) {
	// Create bounding boxes for all displayed vehicles
	for (auto vehicle : *pVehicles) {
		if (!(vehicle.second.IsHidden())) {
			DirectX::BoundingBox bb;
			// FIXME: multiple the vertex positions by GetTransformation * Game::_worldOffset
			BoundingBox::CreateFromPoints(bb, vehicle.second.GetModel().GetModelVertices().size(), GetXMFLOAT3VectorFromModelVertices(vehicle.second.GetModel().GetModelVertices()).data(), sizeof(XMFLOAT3));
			_vehicleBBs.push_back(bb);
		}
	}
}

