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

void VehiclePicker::InitBoundingBoxes(std::map<std::string, Vehicle>* pVehicles) {
	// Extract pointers to all vehicles
	vector<Vehicle*> vehicles;
	vehicles.reserve(pVehicles->size());
	for (auto vehicle : *pVehicles) {
		vehicles.push_back(&(vehicle.second));
	}

	// Create bounding boxes for all displayed vehicles
	_vehicleBBs.reserve(vehicles.size());
	for (Vehicle* vehicle : vehicles) {
		if (!vehicle->IsHidden()) {
			BoundingBox bb;
			//vehicle->GetMo
		}
	}
}

