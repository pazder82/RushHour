#include "stdafx.h"
#include <DirectXMath.h>
#include "Game.h"
#include "D3D.h"
#include "CommonException.h"
#include "Marker.h"

using namespace DirectX;
using namespace std;

Game::Game(HWND hWnd) { 
	_d3d = new D3D(hWnd); 
	_d2d = new D2D(_d3d); 
	_camera = new Camera();
	_vehiclePicker = new VehiclePicker(_d3d, _camera, _worldOffset);
	_depthRenderer = new DepthRenderer(_d3d); 
	_shadowRenderer = new ShadowRenderer(_d3d); 
	_downsampledWindow = new OrthoWindow(_d3d, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	_fullsizedWindow = new OrthoWindow(_d3d, SCREEN_WIDTH, SCREEN_HEIGHT);
	_dsOrthoWindowRenderer = new OrthoWindowRenderer(_d3d, _downsampledWindow);
	_fsOrthoWindowRenderer = new OrthoWindowRenderer(_d3d, _fullsizedWindow);
}

Game::~Game() { 
	if (_d2d) delete _d2d; 
	if (_d3d) delete _d3d; 
	if (_camera) delete _camera; 
	if (_vehiclePicker) delete _vehiclePicker;
	if (_depthRenderer) delete _depthRenderer;
	if (_shadowRenderer) delete _shadowRenderer;
	if (_dsOrthoWindowRenderer) delete _dsOrthoWindowRenderer;
	if (_fsOrthoWindowRenderer) delete _fsOrthoWindowRenderer;
	if (_downsampledWindow) delete _downsampledWindow;
	if (_fullsizedWindow) delete _fullsizedWindow;
}

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

	_vehicles.at(actVehicle).SetMovementStep(static_cast<float>(_frameTime * MOVE_SPEED * dirCoef));
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
		Coords2i_t testCoords;
		int c = (dir == Game::Forward) ? mi1.GetLength() : -1;
		Coords2i_t p = mi1.GetPosition();
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
	Coords2i_t coords = mi.GetPosition();

	float newMovementStep;
	int dirCoef;
	if (mi.GetMovementStep() == 0.0f) {
		// no movement requested
		return;
	} else if (mi.GetMovementStep() < 0.0f) {
		// moving backward
		newMovementStep = mi.GetMovementStep() - (static_cast<float>(_frameTime * MOVE_SPEED));
		dirCoef = -1;
	} else {
		// moving forward
		newMovementStep = mi.GetMovementStep() + (static_cast<float>(_frameTime * MOVE_SPEED));
		dirCoef = 1;
	}

	if (abs(newMovementStep) >= 1.0f) {
		// vehicle finished its move to new position, so replace movementStep with new vehicle position and update its BoundingBox
		if (mi.GetOrientation() == Vehicle::XAxis) {
			mi.SetPosition(coords.x + dirCoef, coords.z);
		} else { // (mi.GetOrientation() == Vehicle::ZAxis)
			mi.SetPosition(coords.x, coords.z + dirCoef);
		}

		newMovementStep = 0.0f;
		UnlockActiveVehicle();
	}
	mi.SetMovementStep(newMovementStep);
	_vehiclePicker->UpdateBoundingBox(GetActiveVehicle());
}

// Update glow level of active vehicle blinking
void Game::UpdateGlowLevel() {
	const float maxGlowLevel = 2.0f;
	const float glowLevelStep = static_cast<float>(_frameTime * GLOWSPEED);
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
	float newGlowLevel = glowLevel + (glowLevelStep * glowDirCoef);
	if (newGlowLevel > maxGlowLevel) {
		newGlowLevel = maxGlowLevel;
	} else if (newGlowLevel < 0.0f) {
		newGlowLevel = 0.0f;
	}
	ai.SetGlowLevel(newGlowLevel);
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

// Print info text
void Game::PrintInfoString() const {
	wstring s = _infoString + _debugInfoString;
	if (FAILED(_d2d->GetWriteFactory()->CreateTextLayout(s.c_str(), (UINT32)s.size(), _d2d->GetTextFormatFPS(), 
		(float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, _d2d->GetTextLayoutFPSAddr()))) {
		throw CommonException((LPWSTR)L"Critical error: Failed to create the text layout for FPS information!");
	}
}

void Game::ClearInfoString() {
	_infoString.clear();
}

void Game::AddInfoString(wstring wstr) {
	_infoString += wstr;
}

void Game::SetDebugInfoString(std::wstring wstr) {
	_debugInfoString = wstr;
}

// Renders one frame using the vertex and pixel shaders.
void Game::Render() {
	CBUFFER cBuffer;

	// *** CAMERA SECTION
	XMMATRIX matView;
	_camera->SetNewPosition(_rotation);
	matView = _camera->GetViewMatrix();
	cBuffer.cameraPosition = _camera->GetPosition();

	// Set projection matrix
	XMMATRIX matPerspective = _d3d->GetProjectionMatrix();

	// *** LIGHTS SECTION
	XMMATRIX lightView, lightPerspective;
	XMVECTOR lightPosition = LIGHTPOSITION;

	// Set light view matrix
	lightView = XMMatrixLookAtLH(
		lightPosition,                        // the light position
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),  // the look-at position
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)   // the up direction
	);
	cBuffer.lightPosition = lightPosition;

	// Set light projection matrix
	lightPerspective = XMMatrixPerspectiveFovLH((FLOAT)XMConvertToRadians(45), (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, 1.0f, 100.0f);

	// *** RENDER SECTION
	// Render depth texture
	_depthRenderer->ConfigureRendering();
	RenderScene(&cBuffer, matView, matPerspective, lightView, lightPerspective);

	// Render shadow into texture
	_shadowRenderer->ConfigureRendering();
	//_shadowRenderer->ConfigureRenderingDebug();
	_d3d->GetDeviceContext()->PSSetShaderResources(1, 1, _depthRenderer->GetRenderTextureSRVAddr()); // provide depth texture to shader
	RenderScene(&cBuffer, matView, matPerspective, lightView, lightPerspective);

	// Downsample shadow texture into OrthoWindow
	_dsOrthoWindowRenderer->ConfigureRendering();
	//_dsOrthoWindowRenderer->ConfigureRenderingDebug();
	_d3d->GetDeviceContext()->PSSetShaderResources(0, 1, _shadowRenderer->GetRenderTextureSRVAddr()); // provide shadow texture to shader
	RenderOrthoWindow(_downsampledWindow, _downsampledWindow->GetViewMatrix(), _downsampledWindow->GetOrthoMatrix());

	// Upsample shadow texture into OrthoWindow
	_fsOrthoWindowRenderer->ConfigureRendering();
	//_fsOrthoWindowRenderer->ConfigureRenderingDebug();
	_d3d->GetDeviceContext()->PSSetShaderResources(0, 1, _dsOrthoWindowRenderer->GetRenderTextureSRVAddr()); // provide downsampled texture to shader
	RenderOrthoWindow(_fullsizedWindow, _fullsizedWindow->GetViewMatrix(), _fullsizedWindow->GetOrthoMatrix());

	// Render final scene
	_d3d->ConfigureRenderering();
	// Use the below line instead of the next when Ortographic projection of shadows is working properly:
	//_d3d->GetDeviceContext()->PSSetShaderResources(2, 1, _fsOrthoWindowRenderer->GetRenderTextureSRVAddr()); // provide upsampled texture to shader
	_d3d->GetDeviceContext()->PSSetShaderResources(2, 1, _shadowRenderer->GetRenderTextureSRVAddr()); // provide upsampled texture to shader
	RenderScene(&cBuffer, matView, matPerspective, lightView, lightPerspective);
/*  FIXME
*/
	// print FPS info
	_d2d->PrintInfo();

	// switch the back buffer and the front buffer
	_d3d->GetSwapChain()->Present(0, 0);
}

void Game::RenderOrthoWindow(OrthoWindow* orthoWindow, DirectX::XMMATRIX matView, DirectX::XMMATRIX matPerspective) {
	OrthoWindow::CBUFFER cBuffer;
	cBuffer.screenHeight = (float) orthoWindow->GetWindowHeight();
	cBuffer.screenWidth = (float) orthoWindow->GetWindowWidth();
	cBuffer.mvp = XMMatrixIdentity() * matView * matPerspective;

	// Send constant buffer
	_d3d->GetDeviceContext()->UpdateSubresource(orthoWindow->GetCBuffer(), 0, 0, &cBuffer, 0, 0);
	_d3d->GetDeviceContext()->DrawIndexed(orthoWindow->GetNumOfIndices(), 0, 0);
}

void Game::RenderScene(CBUFFER* pcBuffer, XMMATRIX matView, XMMATRIX matPerspective, XMMATRIX lightView, XMMATRIX lightPerspective) {
	// Set lights
	pcBuffer->diffuseColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	pcBuffer->ambientColor = XMVectorSet(_ambientColorIntensity, _ambientColorIntensity, _ambientColorIntensity, 1.0f);
	pcBuffer->specularColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

	// Draw model instances
	for (auto it = _minstances.begin(); it != _minstances.end(); it++) {
		ModelInstance mi = it->second;

		// Store instance transformation into constant buffer
		XMMATRIX worldMatrix = mi.GetTransformation() * _worldOffset;
		XMMATRIX mvpMatrix = worldMatrix * matView * matPerspective;
		XMMATRIX lightMvpMatrix = worldMatrix * lightView * lightPerspective;
		XMMATRIX invTrWorld = XMMatrixInverse(nullptr, XMMatrixTranspose(worldMatrix));
		pcBuffer->world = worldMatrix;
		pcBuffer->mvp = mvpMatrix;
		pcBuffer->lightmvp = lightMvpMatrix;
		pcBuffer->invTrWorld = invTrWorld;
		pcBuffer->specularPower = 100000.0f;
		// Send constant buffer
		_d3d->GetDeviceContext()->UpdateSubresource(_d3d->GetCBuffer(), 0, 0, pcBuffer, 0, 0);

		for (auto i : mi.GetModel().GetMeshEntries()) {
			// select texture
			_d3d->GetDeviceContext()->PSSetShaderResources(0, 1, &i._pTexture);
			_d3d->GetDeviceContext()->DrawIndexed(i._numIndices, i._baseIndex, i._baseVertex);
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
		XMMATRIX mvpMatrix = worldMatrix * matView * matPerspective;
		XMMATRIX lightMvpMatrix = worldMatrix * lightView * lightPerspective;
		XMMATRIX invTrWorld = XMMatrixInverse(nullptr, XMMatrixTranspose(worldMatrix));
		pcBuffer->world = worldMatrix;
		pcBuffer->mvp = mvpMatrix;
		pcBuffer->lightmvp = lightMvpMatrix;
		pcBuffer->invTrWorld = invTrWorld;
		// Store vehicle color into constant buffer
		XMVECTOR vehicleColor = mi.GetColor();
		pcBuffer->diffuseColor = vehicleColor;
		pcBuffer->specularColor = vehicleColor;
		float glowIntensity = _ambientColorIntensity + mi.GetGlowLevel();
		pcBuffer->ambientColor = XMVectorSet(vehicleColor.m128_f32[0] * glowIntensity,
			vehicleColor.m128_f32[1] * glowIntensity,
			vehicleColor.m128_f32[2] * glowIntensity,
			vehicleColor.m128_f32[3] * glowIntensity);
		pcBuffer->specularPower = 20.0f;
		// Send constant buffer
		_d3d->GetDeviceContext()->UpdateSubresource(_d3d->GetCBuffer(), 0, 0, pcBuffer, 0, 0);

		for (auto i : mi.GetModel().GetMeshEntries()) {
			// select texture
			_d3d->GetDeviceContext()->PSSetShaderResources(0, 1, &i._pTexture);
			_d3d->GetDeviceContext()->DrawIndexed(i._numIndices, i._baseIndex, i._baseVertex);
		}
	}

	// Draw markers
	for (auto it = _markers.begin(); it != _markers.end(); it++) {
		Marker mi = it->second;
		if (mi.IsHidden()) {
			continue;
		}
		// Store marker transformation into constant buffer
		XMMATRIX worldMatrix = mi.GetTransformation();
		XMMATRIX mvpMatrix = worldMatrix * matView * matPerspective;
		XMMATRIX lightMvpMatrix = worldMatrix * lightView * lightPerspective;
		XMMATRIX invTrWorld = XMMatrixInverse(nullptr, XMMatrixTranspose(worldMatrix));
		pcBuffer->world = worldMatrix;
		pcBuffer->mvp = mvpMatrix;
		pcBuffer->lightmvp = lightMvpMatrix;
		pcBuffer->invTrWorld = invTrWorld;
		pcBuffer->specularPower = 100000.0f;
		// Send constant buffer
		_d3d->GetDeviceContext()->UpdateSubresource(_d3d->GetCBuffer(), 0, 0, pcBuffer, 0, 0);

		for (auto i : mi.GetModel().GetMeshEntries()) {
			// select texture
			_d3d->GetDeviceContext()->PSSetShaderResources(0, 1, &i._pTexture);
			_d3d->GetDeviceContext()->DrawIndexed(i._numIndices, i._baseIndex, i._baseVertex);
		}
	}

}

void Game::Init() {
	// Load all models
	_models.emplace(make_pair(string("bus"), Model("models/bus.obj", _d3d->GetDevice())));
	_models.emplace(make_pair(string("car"), Model("models/taxi_cab.obj", _d3d->GetDevice())));
	_models.emplace(make_pair(string("board"), Model("models/board.3DS", _d3d->GetDevice())));
	_models.emplace(make_pair(string("wall"), Model("models/oldWall.obj", _d3d->GetDevice())));
	_models.emplace(make_pair(string("ball"), Model("models/10536_soccerball_V1_iterations-2.obj", _d3d->GetDevice())));

	// Create base model instances
	const float carScale = 0.0075f;
	const float busScale = 0.105f;
	const float boardScale1 = 1.0f;
	const float boardScale2 = 0.73f;
	const float wallScale = 0.1253f;
	const float ballScale = 0.01f;
	Vehicle miCar(_models.at("car"), XMMatrixScaling(carScale, carScale, carScale), XMVectorSet(0.5f, 0.0f, 0.0f, 0.0f), false, CARLEN);
	Vehicle miBus(_models.at("bus"), XMMatrixScaling(busScale, busScale, busScale), XMVectorSet(1.2f, 0.0f, 0.0f, 0.0f), true, BUSLEN);
	ModelInstance miBoard(_models.at("board"), XMMatrixScaling(boardScale1, boardScale1, boardScale2), XMVectorSet(-0.5f, -0.23f, -0.38f, 0.0f), XMMatrixRotationX(XMConvertToRadians(90.0f)));
	ModelInstance miWallZ(_models.at("wall"), XMMatrixScaling(wallScale, wallScale, wallScale), XMVectorSet(0.57f, -0.0f, 0.49f, 0.0f), XMMatrixIdentity());
	ModelInstance miWallX(_models.at("wall"), XMMatrixScaling(wallScale, wallScale, wallScale), XMVectorSet(0.55f, -0.0f, 0.64f, 0.0f), XMMatrixIdentity());
	Marker marker(_models.at("ball"), XMMatrixScaling(ballScale, ballScale, ballScale));

	// Create base board as copy of base instance miBoard
	_minstances.insert(make_pair(string("board"), miBoard));
	MI(board).SetPosition(0, 0);
	MI(board).SetOrientation(ModelInstance::XAxis);

	// Create markers
	//_markers.insert(make_pair(string("marker1"), marker));

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
	_d3d->CreateVertexBuffer(Model::GetModelVertices());
	_d3d->CreateIndexBuffer(Model::GetModelIndices());

	// Configure VehiclePicker
	_vehiclePicker->InitBoundingBoxes(&_vehicles);

}

