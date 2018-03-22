#include "stdafx.h"
#include "Game.h"
#include <DirectXMath.h>


using namespace DirectX;
using namespace std;

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Game::Update(double frameTime) {
	_frameTime = frameTime;
	UpdateMovementStep();
	UpdateGlowLevel();
}

// Rotate the 3D scene a set amount of radians.
void Game::Rotate(float direction) {
	_rotationAngle += static_cast<float>(ROTATESPEED * direction * _frameTime);
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

	_vehicles.at(actVehicle).SetMovementStep(_frameTime * MOVE_SPEED * dirCoef);
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
		newMovementStep = mi.GetMovementStep() - (_frameTime * MOVE_SPEED);
		dirCoef = -1;
	} else {
		// moving forward
		newMovementStep = mi.GetMovementStep() + (_frameTime * MOVE_SPEED);
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
	const float glowLevelStep = _frameTime * GLOWSPEED;
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
	XMMATRIX matView, matPerspective;
	XMVECTOR camPosition = XMVectorSet(0.0f, 4.0f, 10.0f, 0.0f);
	camPosition = XMVector4Transform(camPosition, _rotation);

	//Set the View matrix
	matView = XMMatrixLookAtLH(
		camPosition,                          // the camera position (rotating around the center of the board)
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),  // the look-at position
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)   // the up direction
	);
	XMStoreFloat4x4(&cBuffer.view, matView);

	// create a projection matrix
	matPerspective = XMMatrixPerspectiveFovLH((FLOAT)XMConvertToRadians(45), (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, 1.0f, 100.0f);
	XMStoreFloat4x4(&cBuffer.projection, matPerspective);

	cBuffer.diffuseVector = XMVectorSet(-1.0f, 1.0f, 1.0f, 0.0f);
	cBuffer.diffuseColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	cBuffer.ambientColor = XMVectorSet(_ambientColorIntensity, _ambientColorIntensity, _ambientColorIntensity, 1.0f);

	// clear the back buffer to a deep blue
	FLOAT bgColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	devcon->ClearRenderTargetView(backbuffer, bgColor);

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

	// select Rasterizer and Sampler configuration
	devcon->RSSetState(pRS);
	devcon->PSSetSamplers(0, 1, &pSS);

	// Draw model instances
	for (auto it = _minstances.begin(); it != _minstances.end(); it++) {
		ModelInstance mi = it->second;

		// Store instance transformation into constant buffer
		XMMATRIX worldMatrix = mi.GetTransformation() * _worldOffset;
		XMStoreFloat4x4(&cBuffer.world, worldMatrix * matView * matPerspective);
		XMStoreFloat4x4(&cBuffer.rotation, mi.GetInitRotation());
		cBuffer.diffuseColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		cBuffer.ambientColor = XMVectorSet(_ambientColorIntensity, _ambientColorIntensity, _ambientColorIntensity, 1.0f);
		// Send constant buffer
		devcon->UpdateSubresource(pCBuffer, 0, 0, &cBuffer, 0, 0);

		for (auto i : mi.GetModel().GetMeshEntries()) {
			// select texture
			devcon->PSSetShaderResources(0, 1, &i._pTexture);
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
		XMMATRIX worldMatrix = mi.GetTransformation() * _worldOffset;
		XMStoreFloat4x4(&cBuffer.world, worldMatrix * matView * matPerspective);
		XMStoreFloat4x4(&cBuffer.rotation, mi.GetInitRotation());
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
			// select texture
			devcon->PSSetShaderResources(0, 1, &i._pTexture);
			devcon->DrawIndexed(i._numIndices, i._baseIndex, i._baseVertex);
		}
	}

	// switch the back buffer and the front buffer
	swapchain->Present(0, 0);
}

void Game::Init() {
	// Load all models
	_models.emplace(make_pair(string("bus"), Model("models/bus.obj")));
	_models.emplace(make_pair(string("car"), Model("models/taxi_cab.obj")));
	_models.emplace(make_pair(string("board"), Model("models/board.3DS")));
	_models.emplace(make_pair(string("wall"), Model("models/oldWall.obj")));

	// Create base model instances
	const float carScale = 0.0075f;
	const float busScale = 0.105f;
	const float boardScale1 = 1.0f;
	const float boardScale2 = 0.73f;
	const float wallScale = 0.1253f;
	Vehicle miCar(_models.at("car"), XMMatrixScaling(carScale, carScale, carScale), XMVectorSet(0.5f, 0.0f, 0.0f, 0.0f), false, CARLEN);
	Vehicle miBus(_models.at("bus"), XMMatrixScaling(busScale, busScale, busScale), XMVectorSet(1.2f, 0.0f, 0.0f, 0.0f), true, BUSLEN);
	ModelInstance miBoard(_models.at("board"), XMMatrixScaling(boardScale1, boardScale1, boardScale2), XMVectorSet(-0.5f, -0.23f, -0.38f, 0.0f), XMMatrixRotationX(XMConvertToRadians(90.0f)));
	ModelInstance miWallZ(_models.at("wall"), XMMatrixScaling(wallScale, wallScale, wallScale), XMVectorSet(0.57f, 0.0f, 0.49f, 0.0f), XMMatrixIdentity());
	ModelInstance miWallX(_models.at("wall"), XMMatrixScaling(wallScale, wallScale, wallScale), XMVectorSet(0.55f, 0.0f, 0.64f, 0.0f), XMMatrixIdentity());

	// Create base board as copy of base instance miBoard
	_minstances.insert(make_pair(string("board"), miBoard));
	MI(board).SetPosition(0, 0);
	MI(board).SetOrientation(ModelInstance::XAxis);

	// Create walls as copies of base instance miWall
	// Wall side 1
	_minstances.insert(make_pair(string("wall1"), miWallZ));
	_minstances.insert(make_pair(string("wall2"), miWallZ));
	_minstances.insert(make_pair(string("wall3"), miWallZ));
	_minstances.insert(make_pair(string("wall4"), miWallZ));
	_minstances.insert(make_pair(string("wall5"), miWallZ));
	_minstances.insert(make_pair(string("wall6"), miWallZ));
	_minstances.insert(make_pair(string("wall7"), miWallZ));
	MI(wall1).SetPosition(-4, -4);
	MI(wall1).SetOrientation(ModelInstance::ZAxis);
	MI(wall2).SetPosition(-3, -4);
	MI(wall2).SetOrientation(ModelInstance::ZAxis);
	MI(wall3).SetPosition(-2, -4);
	MI(wall3).SetOrientation(ModelInstance::ZAxis);
	MI(wall4).SetPosition(-1, -4);
	MI(wall4).SetOrientation(ModelInstance::ZAxis);
	MI(wall5).SetPosition(0, -4);
	MI(wall5).SetOrientation(ModelInstance::ZAxis);
	MI(wall6).SetPosition(1, -4);
	MI(wall6).SetOrientation(ModelInstance::ZAxis);
	MI(wall7).SetPosition(2, -4);
	MI(wall7).SetOrientation(ModelInstance::ZAxis);

	// Wall side 2
	_minstances.insert(make_pair(string("wall8"), miWallX));
	_minstances.insert(make_pair(string("wall9"), miWallX));
	_minstances.insert(make_pair(string("wall10"), miWallX));
	_minstances.insert(make_pair(string("wall11"), miWallX));
	_minstances.insert(make_pair(string("wall12"), miWallX));
	_minstances.insert(make_pair(string("wall13"), miWallX));
	_minstances.insert(make_pair(string("wall14"), miWallX));
	MI(wall8).SetPosition(-4, -4);
	MI(wall8).SetOrientation(ModelInstance::XAxis);
	MI(wall9).SetPosition(-4, -3);
	MI(wall9).SetOrientation(ModelInstance::XAxis);
	MI(wall10).SetPosition(-4, -2);
	MI(wall10).SetOrientation(ModelInstance::XAxis);
	MI(wall11).SetPosition(-4, -1);
	MI(wall11).SetOrientation(ModelInstance::XAxis);
	MI(wall12).SetPosition(-4, 0);
	MI(wall12).SetOrientation(ModelInstance::XAxis);
	MI(wall13).SetPosition(-4, 1);
	MI(wall13).SetOrientation(ModelInstance::XAxis);
	MI(wall14).SetPosition(-4, 2);
	MI(wall14).SetOrientation(ModelInstance::XAxis);

	// Wall side 3
	_minstances.insert(make_pair(string("wall15"), miWallZ));
	_minstances.insert(make_pair(string("wall16"), miWallZ));
	_minstances.insert(make_pair(string("wall17"), miWallZ));
	_minstances.insert(make_pair(string("wall18"), miWallZ));
	_minstances.insert(make_pair(string("wall19"), miWallZ));
	_minstances.insert(make_pair(string("wall20"), miWallZ));
	_minstances.insert(make_pair(string("wall21"), miWallZ));
	MI(wall15).SetPosition(-4, 2);
	MI(wall15).SetOrientation(ModelInstance::ZAxis);
	MI(wall16).SetPosition(-3, 2);
	MI(wall16).SetOrientation(ModelInstance::ZAxis);
	MI(wall17).SetPosition(-2, 2);
	MI(wall17).SetOrientation(ModelInstance::ZAxis);
	MI(wall18).SetPosition(-1, 2);
	MI(wall18).SetOrientation(ModelInstance::ZAxis);
	MI(wall19).SetPosition(0, 2);
	MI(wall19).SetOrientation(ModelInstance::ZAxis);
	MI(wall20).SetPosition(1, 2);
	MI(wall20).SetOrientation(ModelInstance::ZAxis);
	MI(wall21).SetPosition(2, 2);
	MI(wall21).SetOrientation(ModelInstance::ZAxis);

	// Wall side 4
	_minstances.insert(make_pair(string("wall22"), miWallX));
	_minstances.insert(make_pair(string("wall23"), miWallX));
	_minstances.insert(make_pair(string("wall24"), miWallX));
	_minstances.insert(make_pair(string("wall25"), miWallX));
	_minstances.insert(make_pair(string("wall26"), miWallX));
	MI(wall22).SetPosition(2, -4);
	MI(wall22).SetOrientation(ModelInstance::XAxis);
	MI(wall23).SetPosition(2, -3);
	MI(wall23).SetOrientation(ModelInstance::XAxis);
	MI(wall24).SetPosition(2, 0);
	MI(wall24).SetOrientation(ModelInstance::XAxis);
	MI(wall25).SetPosition(2, 1);
	MI(wall25).SetOrientation(ModelInstance::XAxis);
	MI(wall26).SetPosition(2, 2);
	MI(wall26).SetOrientation(ModelInstance::XAxis);

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

	// Fill Vertex and Index buffers with data from Models
	CreateVertexBuffer(Model::GetModelVertices());
	CreateIndexBuffer(Model::GetModelIndices());


}

