#include "stdafx.h"
#include "VehiclePicker.h"
#include "RushHour.h"
#include <limits>

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
	SetDebugString(L", X: " + std::to_wstring(pointX) + L", Y:" + std::to_wstring(pointY));

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
	_dbgPos = { dir.x, dir.y, dir.z };

	// Get the origin of the picking ray which is the position of the camera.
	_pickingRayOrigin = _camera->GetPosition();

	// Now transform the ray origin and the ray direction from view space to world space.
	XMMATRIX invWorldOffset = XMMatrixInverse(nullptr, XMMatrixIdentity());
	_pickingRayOrigin = XMVector3Transform(_pickingRayOrigin, invWorldOffset);
	_pickingRayDirection = XMVector3Transform(_pickingRayDirection, invWorldOffset);
	_pickingRayDirection = XMVector3Normalize(_pickingRayDirection);
}

bool VehiclePicker::GetHitVehicle(string& v) const {
	float minDistance = (numeric_limits<float>::max)();
	bool found = false;
	for (auto vehicleBB : _vehicleBBs) {
		float distance;
		if (vehicleBB.second.Intersects(_pickingRayOrigin, _pickingRayDirection, distance)) {
			if (distance < minDistance) {
				minDistance = distance;
				v = vehicleBB.first;
				found = true;
			}
		}
	}
	return found;
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
	// DEBUG
	XMFLOAT3 dbgPoints[6];
	dbgPoints[0] = { 1.0f, 0.0f, 0.0f };
	dbgPoints[1] = { 0.0f, 1.0f, 0.0f };
	dbgPoints[2] = { 0.0f, 0.0f, 1.0f };
	dbgPoints[3] = { 0.0f, 0.0f, -1.0f };
	dbgPoints[4] = { 0.0f, -1.0f, 0.0f };
	dbgPoints[5] = { -1.0f, 0.0f, 0.0f };
	BoundingBox dbgBb;
	BoundingBox::CreateFromPoints(dbgBb, 6, dbgPoints, sizeof(XMFLOAT3));
//	dbgBb.Transform(dbgBb, XMMatrixTranslation(1.0f, 0.0f, 0.0f));
	XMFLOAT3 dbgCorners[BoundingBox::CORNER_COUNT];
	dbgBb.GetCorners(dbgCorners);
	XMVECTOR dbgRayOrigin = { 0.0f, 0.0f, 2.0f, 1.0f };
	XMVECTOR dbgRayDir = { 0.0f, 0.0f, -1.0f };
	float dbgF;
	bool dbgRes;
	dbgRes = dbgBb.Intersects(dbgRayOrigin, dbgRayDir, dbgF);
	// DEBUG END

	// Create bounding boxes for all displayed vehicles
	for (auto vehicle : *pVehicles) {
		if (!(vehicle.second.IsHidden())) {
			// Get bounding box of the model without transformation
			DirectX::BoundingBox bb;
			BoundingBox::CreateFromPoints(bb, vehicle.second.GetModel().GetModelVertices().size(), GetXMFLOAT3VectorFromModelVertices(vehicle.second.GetModel().GetModelVertices()).data(), sizeof(XMFLOAT3));
			bb.Transform(bb, vehicle.second.GetTransformation() * _worldOffset);
			_vehicleBBs.insert(make_pair(vehicle.first, bb));
		}
	}
}

