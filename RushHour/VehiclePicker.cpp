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

	// Transform mouse cursor from viewport coordinates into normalised device coordinates (into the -1 to +1 range)
	pointX = ((2.0f * static_cast<float>(mouseX)) / static_cast<float>(SCREEN_WIDTH)) - 1.0f;
	pointY = (((2.0f * static_cast<float>(mouseY)) / static_cast<float>(SCREEN_HEIGHT)) - 1.0f) * -1.0f;
	// Transform into homogeneous clip space
	XMFLOAT4 ray_clip = { pointX, pointY, 1.0, 1.0 };
	XMVECTOR ray_clip_v = XMLoadFloat4(&ray_clip);
	// Transform into eye (camera) space
	XMMATRIX invProjMtrx = XMMatrixInverse(nullptr, _d3d->GetProjectionMatrix());
	XMVECTOR ray_eye_v = XMVector4Transform(ray_clip_v, invProjMtrx);
	// Unproject x,y components, so set the z,w part to mean "forwards, and not a point". 
	XMFLOAT4 ray_eye;
	XMStoreFloat4(&ray_eye, ray_eye_v);
	ray_eye = { ray_eye.x, ray_eye.y, 1.0, 0.0 };
	ray_eye_v = XMLoadFloat4(&ray_eye);
	// Transform into world space
	XMMATRIX invViewMtrx = XMMatrixInverse(nullptr, _camera->GetViewMatrix());
	XMFLOAT4 ray_wrld;
	XMStoreFloat4(&ray_wrld, XMVector4Transform(ray_eye_v, invViewMtrx));
	// Cast the "picking" ray from camera position in the direction defined by normalized direction vector
	XMVECTOR ray_wrld_v = XMLoadFloat3(&XMFLOAT3(ray_wrld.x, ray_wrld.y, ray_wrld.z));
	_pickingRayDirection = XMVector3Normalize(ray_wrld_v);
	_pickingRayOrigin = _camera->GetPosition();
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
/*
		// Bounding box hit, now check all triangles
		Vehicle vehicle = _pVehicles->at(vehicleBB.first);
		vector<UINT> indices = vehicle.GetModel().GetModelIndices();
		auto i = indices.begin();
		while (i != indices.end()) {
			VERTEX v1, v2, v3;
			XMMATRIX tm = vehicle.GetTransformation() * _worldOffset;
			v1 = vehicle.GetModel().GetModelVertices().at(*(i++));
			v2 = vehicle.GetModel().GetModelVertices().at(*(i++));
			v3 = vehicle.GetModel().GetModelVertices().at(*(i++));
			if (DirectX::TriangleTests::Intersects(_pickingRayOrigin, _pickingRayDirection,
				XMVector3Transform(XMLoadFloat3(&v1.pos), tm),
				XMVector3Transform(XMLoadFloat3(&v2.pos), tm),
				XMVector3Transform(XMLoadFloat3(&v3.pos), tm),
				distance2)) {
				if (distance2 < minDistance) {
					minDistance = distance2;
					v = vehicleBB.first;
					found = true;
					break; // We found THE triangle, we don't need to check the others
				}
			}
		}
	}
*/

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
	_pVehicles = pVehicles;
	_vehicleBBs.clear();
	// Create bounding boxes for all displayed vehicles
	for (auto vehicle : *_pVehicles) {
		if (!(vehicle.second.IsHidden())) {
			// Get bounding box of the model without transformation
			DirectX::BoundingBox bb;
			// BUS bounding boxes are for some reason created too big - it helps to divide number of vertices with a magic number ;)
			int modelVerticesDivisor = 1; // for normal object do not divide
			if (vehicle.first.substr(0,3) == "bus") {
				modelVerticesDivisor = 4; // for bus, divide by magic number
			}
			BoundingBox::CreateFromPoints(bb, vehicle.second.GetModel().GetModelVertices().size()/modelVerticesDivisor, GetXMFLOAT3VectorFromModelVertices(vehicle.second.GetModel().GetModelVertices()).data(), sizeof(XMFLOAT3));
			bb.Transform(bb, vehicle.second.GetTransformation() * _worldOffset);
			_vehicleBBs.insert(make_pair(vehicle.first, bb));
		}
	}
}

void VehiclePicker::UpdateBoundingBox(std::string vehicleName) {
	int modelVerticesDivisor = 1; // for normal object do not divide
	if (vehicleName.substr(0,3) == "bus") {
		modelVerticesDivisor = 4; // for bus, divide by magic number
	}
	DirectX::BoundingBox bb;
	BoundingBox::CreateFromPoints(bb, _pVehicles->at(vehicleName).GetModel().GetModelVertices().size()/modelVerticesDivisor, GetXMFLOAT3VectorFromModelVertices(_pVehicles->at(vehicleName).GetModel().GetModelVertices()).data(), sizeof(XMFLOAT3));
	bb.Transform(bb, _pVehicles->at(vehicleName).GetTransformation() * _worldOffset);
	_vehicleBBs[vehicleName] = bb;
}

