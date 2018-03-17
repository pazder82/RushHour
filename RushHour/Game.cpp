#include "stdafx.h"
#include "Game.h"
#include <DirectXMath.h>


using namespace DirectX;
using namespace std;

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Game::Update() {
	UpdateMovementStep();
	UpdateGlowLevel();
}

// Rotate the 3D scene a set amount of radians.
void Game::Rotate(float radians) {
	_rotationAngle += radians;
	// Prepare to pass the updated model matrix to the shader
	_rotation = XMMatrixRotationY(_rotationAngle);
}

// Set vehicle given by idstr as active and return true. Return false if such vehicle does not exist.
bool Game::SetActiveVehicle(string idstr) {
	if (IsActiveVehicleLocked()) {
		return false;
	}
	if (_vehicles.find(idstr) == _vehicles.end()) {
		return false;
	}
	if (!_activeVehicle.empty()) {
		_vehicles.at(_activeVehicle).SetGlowLevel(0.0f);
	}
	_activeVehicle = idstr;
	return true;
}

bool Game::UnsetActiveVehicle() {
	if (IsActiveVehicleLocked()) {
		return false;
	}
	if (!_activeVehicle.empty()) {
		_vehicles.at(_activeVehicle).SetGlowLevel(0.0f);
	}
	_activeVehicle.clear();
	return true;
}

// Find new shown vehicle the can be moved and set it as active
void Game::SetNextActiveVehicle() {
	// get _vehicles iterator for activeInstace
	auto miItStart = (_activeVehicle.empty()) ? _vehicles.begin() : _vehicles.find(_activeVehicle);
	// and move iterator to next vehicle as a start position for searching
	miItStart++;
	for (auto it = miItStart; it != _vehicles.end(); it++) {
		// search behind the start position
		if (IsVehicleFreeForMove(it->first) && !it->second.IsHidden()) {
			SetActiveVehicle(it->first);
			return;
		}
	}
	for (auto it = _vehicles.begin(); it != miItStart; it++) {
		// search before the start position
		if (IsVehicleFreeForMove(it->first) && !it->second.IsHidden()) {
			SetActiveVehicle(it->first);
			return;
		}
	}
}

// Moves vehicle one field forward (dir > 0), or backward (dir < 0)
// Return true if success. Return false if the vehicle can't be moved.
bool Game::MoveActiveVehicle(Game::Direction_t dir) {
	string actVehicle = GetActiveVehicle();

	if (actVehicle.empty() || !IsVehicleFreeForMoveForwardOrBackward(actVehicle, dir) || !LockActiveVehicle()) {
		return false;
	}

	int dirCoef = 1; // coeficient for movementStep direction
	if (dir == Game::Backward) {
		dirCoef = -1;
	}

	_vehicles.at(actVehicle).SetMovementStep(MOVE_SPEED * dirCoef);
	return true;
}

// Return true if instace can move in either direction (forward or backward).
bool Game::IsVehicleFreeForMove(std::string idstr) const {
	return IsVehicleFreeForMoveForwardOrBackward(idstr, Game::Backward) ||
		IsVehicleFreeForMoveForwardOrBackward(idstr, Game::Forward);
}

// Return true if instace can move in specified direction.
// Test whether position in required direction is occupyied by some vehicle or is blocked by grid limits.
bool Game::IsVehicleFreeForMoveForwardOrBackward(std::string idstr, Game::Direction_t dir) const {
	const Vehicle& mi1 = _vehicles.at(idstr);
	for (auto it : _vehicles) {
		if (it.first == idstr) {
			// don't test itself
			continue;
		}
		const Vehicle& mi2 = it.second;
		Coords_t testCoords;
		int c = (dir == Game::Forward) ? mi1.GetLength() : -1;
		Coords_t p = mi1.GetPosition();
		int newx, newz;
		if (mi1.GetOrientation() == Vehicle::XAxis) {
			newx = p.x + c;
			newz = p.z;
		} else {
			newx = p.x;
			newz = p.z + c;
		}
		if ((newx < BOARD_MIN_COORD) || (newx > BOARD_MAX_COORD) || (newz < BOARD_MIN_COORD) || (newz > BOARD_MAX_COORD)) {
			return false;
		}
		testCoords = { newx, newz };
		if (mi2.IsOccupyingPosition(testCoords)) {
			return false;
		}
	}
	return true;
}

// Perform the movement started by MoveActiveVehicle() method step by step
void Game::UpdateMovementStep() {
	if (GetActiveVehicle().empty()) {
		return;
	}
	Vehicle& mi = _vehicles.at(GetActiveVehicle());
	Coords_t coords = mi.GetPosition();

	float newMovementStep;
	int dirCoef;
	if (mi.GetMovementStep() == 0.0f) {
		// no movement requested
		return;
	} else if (mi.GetMovementStep() < 0.0f) {
		// moving backward
		newMovementStep = mi.GetMovementStep() - MOVE_SPEED;
		dirCoef = -1;
	} else {
		// moving forward
		newMovementStep = mi.GetMovementStep() + MOVE_SPEED;
		dirCoef = 1;
	}

	if (abs(newMovementStep) >= 1.0f) {
		// vehicle finished its move to new position, so replace movementStep with new vehicle position
		if (mi.GetOrientation() == Vehicle::XAxis) {
			mi.SetPosition(coords.x + dirCoef, coords.z);
		} else { // (mi.GetOrientation() == Vehicle::ZAxis)
			mi.SetPosition(coords.x, coords.z + dirCoef);
		}

		newMovementStep = 0.0f;
		UnlockActiveVehicle();
	}
	mi.SetMovementStep(newMovementStep);
}

// Update glow level of active vehicle blinking
void Game::UpdateGlowLevel() {
	const float maxGlowLevel = 2.0f;
	const float glowLevelStep = 0.1f;
	if (GetActiveVehicle().empty()) {
		return;
	}
	Vehicle& ai = _vehicles.at(GetActiveVehicle());
	float glowLevel = ai.GetGlowLevel();
	if (glowLevel >= maxGlowLevel) {
		_glowLevelUp = false;
	} else if (glowLevel <= 0.0f) {
		_glowLevelUp = true;
	}

	int glowDirCoef = (_glowLevelUp) ? 1 : -1;
	ai.SetGlowLevel(glowLevel + (glowLevelStep * glowDirCoef));
}

bool Game::LockActiveVehicle() {
	if (_activeVehicleLock) {
		return false;
	}
	return _activeVehicleLock = true;
}

void Game::UnlockActiveVehicle() {
	_activeVehicleLock = false;
}

// Renders one frame using the vertex and pixel shaders.
void Game::Render() {
	CBUFFER cBuffer;
	D3DXMATRIX matRotate, matView, matPerspective;
	D3DXMATRIX matFinal;

	// create a view matrix
	D3DXMatrixLookAtLH(&matView,
		&D3DXVECTOR3(0.0f, 4.0f, 10.0f),   // the camera position
		&D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // the look-at position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f));   // the up direction
	cBuffer.view = matView;

	// create a projection matrix
	D3DXMatrixPerspectiveFovLH(&matPerspective, (FLOAT)D3DXToRadian(45), (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, 1.0f, 100.0f);
	cBuffer.projection = matPerspective;

	cBuffer.diffuseVector = XMVectorSet(-1.0f, 1.0f, 1.0f, 0.0f);
	cBuffer.diffuseColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	cBuffer.ambientColor = XMVectorSet(_ambientColorIntensity, _ambientColorIntensity, _ambientColorIntensity, 1.0f);

	/*
	XMFLOAT4X4 matOrientation = XMFLOAT4X4(DX::DeviceResources().GetOrientationTransform3D());
	XMStoreFloat4x4(&matProjection, XMLoadFloat4x4(&matOrientation) * XMLoadFloat4x4(&(XMFLOAT4X4(matOrientation))));
	cBuffer.projection = matProjection;
	*/

	// clear the back buffer to a deep blue
	devcon->ClearRenderTargetView(backbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));

	// clear the depth buffer
	devcon->ClearDepthStencilView(zbuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// select which vertex buffer to display
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	devcon->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);

	// select which primtive type we are using
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devcon->IASetInputLayout(pLayout);

	// select texture
	devcon->PSSetShaderResources(0, 1, &pTexture);

	// Draw model instances
	for (auto it = _minstances.begin(); it != _minstances.end(); it++) {
		ModelInstance mi = it->second;

		// Store instance transformation into constant buffer
		XMMATRIX worldMatrix = mi.GetTransformation() * _worldOffset * _rotation;
		D3DMATRIX matWorld = *reinterpret_cast<D3DXMATRIX*>(&worldMatrix) * matView * matPerspective;
		cBuffer.world = matWorld;
		XMStoreFloat4x4(&(cBuffer.rotation), _rotation * mi.GetInitRotation());
		cBuffer.diffuseColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		cBuffer.ambientColor = XMVectorSet(_ambientColorIntensity, _ambientColorIntensity, _ambientColorIntensity, 1.0f);
		// Send constant buffer
		devcon->UpdateSubresource(pCBuffer, 0, 0, &cBuffer, 0, 0);

		for (auto i : mi.GetModel().GetMeshEntries()) {
			devcon->DrawIndexed(i._numIndices, i._baseIndex, i._baseVertex);
		}
	}

	// Draw vehicles
	for (auto it = _vehicles.begin(); it != _vehicles.end(); it++) {
		Vehicle mi = it->second;
		if (mi.IsHidden()) {
			continue;
		}
		// Store vehicle transformation into constant buffer
		XMMATRIX worldMatrix = mi.GetTransformation() * _worldOffset * _rotation;
		D3DMATRIX matWorld = *reinterpret_cast<D3DXMATRIX*>(&worldMatrix) * matView * matPerspective;
		cBuffer.world = matWorld;
		XMStoreFloat4x4(&(cBuffer.rotation), _rotation * mi.GetInitRotation());
		// Store vehicle color into constant buffer
		XMVECTOR vehicleColor = mi.GetColor();
		cBuffer.diffuseColor = vehicleColor;
		float glowIntensity = _ambientColorIntensity + mi.GetGlowLevel();
		cBuffer.ambientColor = XMVectorSet(vehicleColor.m128_f32[0] * glowIntensity,
			vehicleColor.m128_f32[1] * glowIntensity,
			vehicleColor.m128_f32[2] * glowIntensity,
			vehicleColor.m128_f32[3] * glowIntensity);
		// Send constant buffer
		devcon->UpdateSubresource(pCBuffer, 0, 0, &cBuffer, 0, 0);

		for (auto i : mi.GetModel().GetMeshEntries()) {
			devcon->DrawIndexed(i._numIndices, i._baseIndex, i._baseVertex);
		}
	}

	// switch the back buffer and the front buffer
	swapchain->Present(0, 0);
}

void Game::Init() {
	// Load all models
	_models.emplace(make_pair(string("bus"), Model("bus2.obj")));
	_models.emplace(make_pair(string("car"), Model("taxi_cab.obj")));
	_models.emplace(make_pair(string("board"), Model("board.obj")));
	_models.emplace(make_pair(string("wall"), Model("oldWall.obj")));

	// Create base model instances
	const float carScale = 0.008f;
	const float busScale = 0.11f;
	const float boardScale = 0.06f;
	const float wallScale = 0.1f;
	Vehicle miCar(_models.at("car"), XMMatrixScaling(carScale, carScale, carScale), XMVectorSet(0.5f, 0.0f, 0.0f, 0.0f), true, CARLEN);
	Vehicle miBus(_models.at("bus"), XMMatrixScaling(busScale, busScale, busScale), XMVectorSet(1.2f, 0.0f, 0.0f, 0.0f), false, BUSLEN);
	ModelInstance miBoard(_models.at("board"), XMMatrixScaling(boardScale, boardScale, boardScale), XMVectorSet(-1.5f, -0.17f, -0.9f, 0.0f), XMMatrixIdentity()/*XMMatrixRotationX(D3DXToRadian(180))*/);
	ModelInstance miWall(_models.at("wall"), XMMatrixScaling(wallScale, wallScale, wallScale), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMMatrixIdentity());

	// Create base board as copy of base instance miBoard
	_minstances.insert(make_pair(string("board"), miBoard));
	MI(board).SetPosition(0, 0);
	MI(board).SetOrientation(ModelInstance::ZAxis);

	// Create walls as copies of base instance miWall
	_minstances.insert(make_pair(string("wall"), miWall));
	MI(wall).SetPosition(-4, -4);
	MI(wall).SetOrientation(ModelInstance::ZAxis);

	// Create vehicles as copies of base instances miCar and miBus
	_vehicles.insert(make_pair(string("car1"), miCar));
	_vehicles.insert(make_pair(string("car2"), miCar));
	_vehicles.insert(make_pair(string("car3"), miCar));
	_vehicles.insert(make_pair(string("car4"), miCar));
	_vehicles.insert(make_pair(string("car5"), miCar));
	_vehicles.insert(make_pair(string("car6"), miCar));
	_vehicles.insert(make_pair(string("car7"), miCar));
	_vehicles.insert(make_pair(string("car8"), miCar));
	_vehicles.insert(make_pair(string("car9"), miCar));
	_vehicles.insert(make_pair(string("car10"), miCar));
	_vehicles.insert(make_pair(string("car11"), miCar));
	_vehicles.insert(make_pair(string("car12"), miCar));
	_vehicles.insert(make_pair(string("bus1"), miBus));
	_vehicles.insert(make_pair(string("bus2"), miBus));
	_vehicles.insert(make_pair(string("bus3"), miBus));
	_vehicles.insert(make_pair(string("bus4"), miBus));

	// Set color of each vehicle
	VEH(car1).SetColor(XMUINT3{ 255, 0, 0 }); // Player's car
	VEH(car2).SetColor(XMUINT3{ 253, 194, 0 });
	VEH(car3).SetColor(XMUINT3{ 102, 102, 0 });
	VEH(car4).SetColor(XMUINT3{ 153, 0, 153 });
	VEH(car5).SetColor(XMUINT3{ 0, 255, 255 });
	VEH(car6).SetColor(XMUINT3{ 102, 51, 0 });
	VEH(car7).SetColor(XMUINT3{ 0, 153, 0 });
	VEH(car8).SetColor(XMUINT3{ 209, 184, 132 });
	VEH(car9).SetColor(XMUINT3{ 90, 90, 90 });
	VEH(car10).SetColor(XMUINT3{ 255, 102, 255 });
	VEH(car11).SetColor(XMUINT3{ 252, 255, 128 });
	VEH(car12).SetColor(XMUINT3{ 153, 255, 204 });
	VEH(bus1).SetColor(XMUINT3{ 255, 239, 0 });
	VEH(bus2).SetColor(XMUINT3{ 0, 0, 255 });
	VEH(bus3).SetColor(XMUINT3{ 240, 0, 255 });
	VEH(bus4).SetColor(XMUINT3{ 0, 255, 162 });

	/* DEBUG - umistovani instanci bude resit trida Level */
	// Positon vehicles into the grid
	// Available coordinates: <-3, 2> for x and z axis
	VEH(bus1).SetPosition(-3, -2);
	VEH(bus1).SetOrientation(Vehicle::ZAxis);
	VEH(bus1).Show();
	VEH(bus2).SetPosition(0, -2);
	VEH(bus2).SetOrientation(Vehicle::ZAxis);
	VEH(bus2).Show();
	VEH(bus3).SetPosition(2, -3);
	VEH(bus3).SetOrientation(Vehicle::ZAxis);
	VEH(bus3).Show();
	VEH(bus4).SetPosition(-1, 2);
	VEH(bus4).SetOrientation(Vehicle::XAxis);
	VEH(bus4).Show();

	VEH(car1).SetPosition(-2, -1);
	VEH(car1).SetOrientation(Vehicle::XAxis);
	VEH(car1).Show();
	VEH(car2).SetPosition(-3, -3);
	VEH(car2).SetOrientation(Vehicle::XAxis);
	VEH(car2).Show();
	VEH(car3).SetPosition(-3, 1);
	VEH(car3).SetOrientation(Vehicle::ZAxis);
	VEH(car3).Show();
	VEH(car4).SetPosition(1, 1);
	VEH(car4).SetOrientation(Vehicle::XAxis);
	VEH(car4).Show();

	/* DEBUG END */
	// Select first active vehicle
	this->SetNextActiveVehicle();

	/* HOP TODO - vyhodit do samostatne tridy */
	// load the texture
	LoadTextures();
	/*HOP end*/

	// Once both shaders are loaded, create the mesh.
	CreateVertexBuffer(Model::GetModelVertices());
	CreateIndexBuffer(Model::GetModelIndices());


}

