#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include "D3D.h"
#include "Camera.h"
#include "Vehicle.h"

#define PICKINGRAYLEN 10

class VehiclePicker {
public:
	VehiclePicker() = delete;
	VehiclePicker(D3D* d3d, Camera* camera, DirectX::XMMATRIX& worldOffset) : _d3d(d3d), _camera(camera), _worldOffset(worldOffset) { }
	~VehiclePicker();

	// Generate bounding boxes from all vehicles
	void InitBoundingBoxes(std::map<std::string, Vehicle>* pVehicles);
	void SetPickingRay(LONG mouseX, LONG mouseY);

	// Test picking ray intersection with _vehicleBBs and return the closest one in bb if found
	bool GetHitVehicle(std::string& v) const;

	std::wstring GetDebugString() const { return _dbgString; }
	Coords3f_t GetDebugPosition() const { return { _pickingRayOrigin.m128_f32[0] + PICKINGRAYLEN*_pickingRayDirection.m128_f32[0], 
		_pickingRayOrigin.m128_f32[1] + PICKINGRAYLEN*_pickingRayDirection.m128_f32[1],
	    _pickingRayOrigin.m128_f32[2] + PICKINGRAYLEN*_pickingRayDirection.m128_f32[2] };
	}

private:

	// DirectX objects
	D3D* _d3d = nullptr;
	Camera* _camera = nullptr;

	// World offset
	DirectX::XMMATRIX _worldOffset;

	// Picking ray
	DirectX::XMVECTOR _pickingRayOrigin, _pickingRayDirection;

	// Vector of vehicle bounding boxes
	std::map<std::string, DirectX::BoundingBox> _vehicleBBs;

	// Methods
	std::vector<DirectX::XMFLOAT3> GetXMFLOAT3VectorFromModelVertices(const std::vector<VERTEX>& modelVertices) const;

	// Debug stuff
    std::wstring _dbgString;
	void SetDebugString(std::wstring s) { _dbgString = s; }

};

