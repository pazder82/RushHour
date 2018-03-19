#pragma once
#include <DirectXMath.h>
#include "Model.h"

class ModelInstance {
public:
	enum Orientation_t { XAxis, ZAxis };

	// Constructors
	ModelInstance() = delete;
	ModelInstance(Model& m) : _model(m) {}
	ModelInstance(Model& m, DirectX::XMMATRIX scale, DirectX::XMVECTOR alignment, DirectX::XMMATRIX rotation);
	~ModelInstance() {}

	// Transformation methods
	void SetOrientation(ModelInstance::Orientation_t dir) { _orientation = dir; }
	void SetPosition(Coords_t coords);
	void SetPosition(int x, int z) { SetPosition({ x, z }); }
	void SetAlignment(DirectX::XMVECTOR alignment) { _alignment = alignment; }
	DirectX::XMMATRIX GetTransformation() const;
	DirectX::XMMATRIX GetInitRotation() const;
	ModelInstance::Orientation_t GetOrientation() const { return _orientation; }
	Coords_t GetPosition() const { return _position; }
	DirectX::XMVECTOR GetAlignment() const { return _alignment; }
	DirectX::XMMATRIX GetScale() const { return _scale; }

	// Graphics settings methods
	DirectX::XMVECTOR GetColor() const { return _color; }
	void SetColor(DirectX::XMUINT3 c);
	void SetColor(DirectX::XMVECTOR c);

	// Other methods
	Model& GetModel() const { return _model; }

private:
	// Reference to polygon model 
	Model& _model;

	// Transformation matrices and vars
	DirectX::XMMATRIX _scale = DirectX::XMMatrixIdentity(); // scale instance size
	Coords_t _position = { 0, 0 }; // positon instance
	DirectX::XMVECTOR _alignment = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); // align the instance to the grid field
	DirectX::XMMATRIX _rotation = DirectX::XMMatrixIdentity(); // rotate the instance into the initial position
	Orientation_t _orientation = Orientation_t::XAxis;

    // Graphics settings vars
	DirectX::XMVECTOR _color = DirectX::XMVectorSet(0.2f, 0.2f, 0.2f, 0.0f);

	// Auxiliary methods
	DirectX::XMVECTOR MakeColorVector(DirectX::XMUINT3 rgb);
};

