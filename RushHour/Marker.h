#pragma once
#include <DirectXMath.h>
#include "Model.h"

class Marker {
public:
	// Constructors
	Marker() = delete;
	Marker(Model& m) : _model(m) {}
	Marker(Model& m, DirectX::XMMATRIX scale);
	Marker(const Marker&) = default;
	Marker(Marker&&) = default;
	Marker& operator= (const Marker&) = default;
	Marker& operator= (Marker&&) = default;
	~Marker() {}

	// Transformation methods
	void SetPosition(Coords3f_t coords) { _position = coords; }
	void SetPosition(float x, float y, float z) { SetPosition({ x, y, z }); }
	DirectX::XMMATRIX GetTransformation() const;
	DirectX::XMMATRIX GetInitRotation() const { return _rotation; }
	Coords3f_t GetPosition() const { return _position; }
	DirectX::XMMATRIX GetScale() const { return _scale; }

	// Other methods
	Model& GetModel() const { return _model; }
	void Hide() { _displayed = false; } // Do not display this marker
	void Show() { _displayed = true; }  // Display this marker
	bool IsHidden() const { return !_displayed; }
private:
	// Reference to polygon model 
	Model& _model;

	// Transformation matrices and vars
	DirectX::XMMATRIX _scale = DirectX::XMMatrixIdentity(); // scale instance size
	Coords3f_t _position = { 0.0f, 0.0f, 0.0f }; // positon instance
	DirectX::XMMATRIX _rotation = DirectX::XMMatrixIdentity(); // rotate the instance into the initial position

	// Graphics settings vars
	bool _displayed = true;
};

