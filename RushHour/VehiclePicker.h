#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <vector>
#include <map>
#include "D3D.h"
#include "Camera.h"
#include "Vehicle.h"

class VehiclePicker {
public:
	VehiclePicker() = delete;
	VehiclePicker(D3D* d3d, Camera* camera) : _d3d(d3d), _camera(camera) { }
	~VehiclePicker();

	// Generate bounding boxes from all vehicles
	void InitBoundingBoxes(std::map<std::string, Vehicle>* pVehicles);
	void SetPickingRay(LONG mouseX, LONG mouseY);

private:

	// DirectX objects
	D3D* _d3d = nullptr;
	Camera* _camera = nullptr;

	// Picking ray
	DirectX::XMVECTOR _pickingRayOrigin, _pickingRayDirection;

	// Vector of vehicle boundig boxes
	std::vector<DirectX::BoundingBox> _vehicleBBs;

};

