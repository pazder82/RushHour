#pragma once
#include <vector>
#include <map>
#include <string>
#include <DirectXMath.h>
#include "D3DSupplementary.h"
#include "Model.h"
#include "Vehicle.h"
#include "D2D.h"
#include "D3D.h"
#include "Camera.h"
#include "DepthRenderer.h"
#include "ShadowRenderer.h"
#include "OrthoWindow.h"
#include "OrthoWindowRenderer.h"
#include "VehiclePicker.h"
#include "Marker.h"

#define VEH(X) _vehicles.at(std::string(#X))
#define MI(X) _minstances.at(std::string(#X))
#define BUSLEN 3
#define CARLEN 2

#define BOARD_MAX_COORD 2
#define BOARD_MIN_COORD -3

class Game {
public:
	Game() = delete;
	Game(HWND hWnd);
	~Game();
	enum Direction_t { Forward, Backward };

	void Init();
	void Update(double frameTime);
	void Render();
	void Rotate(float direction);

	// Methods for operating active vehicle
	bool SetActiveVehicle(std::string idstr);
	bool UnsetActiveVehicle();
	void SetNextActiveVehicle();
	std::string GetActiveVehicle() const { return _activeVehicle; }
	bool MoveActiveVehicle(Game::Direction_t dir);

	// Methods for vehicles tests
	bool IsVehicleFreeForMove(std::string idstr) const;
	bool IsVehicleFreeForMoveForwardOrBackward(std::string idstr, Game::Direction_t dir) const;

	// Info text methods
	void PrintInfoString() const;
	void AddInfoString(std::wstring wstr);
	void SetDebugInfoString(std::wstring wstr);
	void ClearInfoString();

	// Vehicle picker
	VehiclePicker* GetVehiclePicker() const { return _vehiclePicker; }


private:
	// Timer vars
	double _frameTime = 0.0;

	// Info wstring
	std::wstring _infoString;
	std::wstring _debugInfoString;

	// DirectX objects
	D2D* _d2d = nullptr;
	D3D* _d3d = nullptr;
	Camera* _camera = nullptr;
	DepthRenderer* _depthRenderer = nullptr;
	ShadowRenderer* _shadowRenderer = nullptr;
	OrthoWindowRenderer* _dsOrthoWindowRenderer = nullptr;
	OrthoWindowRenderer* _fsOrthoWindowRenderer = nullptr;
	OrthoWindow* _downsampledWindow = nullptr;
	OrthoWindow* _fullsizedWindow = nullptr;


	// Models and objects containers
	std::map<std::string, Model> _models;
	std::map<std::string, ModelInstance> _minstances;
	std::map<std::string, Vehicle> _vehicles;
	std::map<std::string, Marker> _markers;

	// Active vehicle vars
	std::string _activeVehicle;
	bool _activeVehicleLock = false;

	// Vehicle picker
	VehiclePicker* _vehiclePicker = nullptr;

	// World position vars
	float _rotationAngle = 0;
	DirectX::XMMATRIX _rotation = DirectX::XMMatrixIdentity();
	// as we index model position on grid in interval <-3, 2>, we need to move the board to be kept in the middle of the screen
	//DirectX::XMMATRIX _worldOffset = DirectX::XMMatrixTranslation(0.5f, 0.0f, 0.5f);
	DirectX::XMMATRIX _worldOffset = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	// Color settings vars
	const float _ambientColorIntensity = AMBIENTCOLORINTENSITY;
	bool _glowLevelUp = true;

	// Rendering methods
	void RenderScene(CBUFFER* pcBuffer, DirectX::XMMATRIX matView, DirectX::XMMATRIX matPerspective, DirectX::XMMATRIX lightView, DirectX::XMMATRIX lightPerspective);
	void RenderOrthoWindow(OrthoWindow* orthoWindow, DirectX::XMMATRIX matView, DirectX::XMMATRIX matPerspective);

	// Active vehicle methods
	void UpdateMovementStep(); // If some movement activated, update movement step
	void UpdateGlowLevel();    // Makes the glow of active vehicle pulse
	bool IsActiveVehicleLocked() { return _activeVehicleLock; }
	bool LockActiveVehicle();
	void UnlockActiveVehicle();

};

