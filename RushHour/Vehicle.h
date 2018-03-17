#pragma once

#include "Model.h"
#include "ModelInstance.h"
#include "RushHour.h"

class Vehicle : public ModelInstance {
public:
	// Constructors
	Vehicle() = delete;
	Vehicle(Model& m) : ModelInstance(m) {}
	Vehicle(Model& m, DirectX::XMMATRIX scale, DirectX::XMVECTOR alignment, bool rotate180, unsigned short length);
	Vehicle(const Vehicle&) = default;
	Vehicle(Vehicle&&) = default;
	Vehicle& operator= (const Vehicle&) = default;
	Vehicle& operator= (Vehicle&&) = default;
	~Vehicle() {};

	// Transformation methods
	void SetMovementStep(float s);
	DirectX::XMMATRIX GetTransformation() const;
	DirectX::XMMATRIX GetInitRotation() const;
	float GetMovementStep() const { return _movementStep; }
	bool IsRotated180() const { return _rotate180; }

	// Graphics settings methods
	float GetGlowLevel() const { return _glowLevel; }
	void SetGlowLevel(float glowLevel) { _glowLevel = glowLevel; }

	// Collision testing methods
	bool IsOccupyingPosition(Coords_t pos) const;
	unsigned short GetLength() const { return _length; }

	// Other methods
	void Hide() { _displayed = false; } // Do not display this vehicle
	void Show() { _displayed = true; }  // Display this vehicle
	bool IsHidden() const { return !_displayed; }

private:
	// Transformation matrices and vars
	bool _rotate180 = false;
	float _movementStep = 0.0f;

	// Auxiliary vars
	unsigned short _length = 1;

	// Graphics settings vars
	bool _displayed = false;
	float _glowLevel = 0.0f;

};

