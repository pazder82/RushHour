#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <WinUser.h>
#include <d3d11.h>
#include <chrono>
#include <thread>
#include <string>
#include "RushHour.h"
#include "D3DSupplementary.h"
#include "Game.h"
#include "Timer.h"
#include "CommonException.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

Game* pGame = nullptr;

/* ************ 
   *** MAIN ***
   ************ */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	try {
		HWND hWnd;
		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(WNDCLASSEX));

		// fill in the struct with the needed information
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = L"WindowClass1";

		// register the window class
		RegisterClassEx(&wc);

		RECT wr = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

		// create the window and use the result as the handle
		hWnd = CreateWindowEx(NULL, L"WindowClass1", L"Rush Hour Game",
			WS_OVERLAPPEDWINDOW, 0, 0, wr.right - wr.left, wr.bottom - wr.top, NULL,
			NULL, hInstance, NULL);

		ShowWindow(hWnd, nCmdShow);
		Game game(hWnd);
		pGame = &game;
		MSG msg = { 0 };
		game.Init();

		// Init timer
		Timer timer;
		timer.StartTimer();
		timer.GetFrameTime();

		/* ********* */
		/* MAIN LOOP */
		/* ********* */
		int frameCount = 0;
		int fps = 0;
		while (TRUE) {
			// Check to see if any messages are waiting in the queue
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
					break;
			}

			// Count fps
			frameCount++;
			if (timer.GetTime() > 1.0) {
				fps = frameCount;
				frameCount = 0;
				timer.StartTimer();
			}
			game.AddInfoString(L"FPS: " + std::to_wstring(fps));
			game.PrintInfoString();
			game.Update(timer.GetFrameTime());
			game.Render();
			game.ClearInfoString();

		}
		pGame = nullptr;
		return msg.wParam;
	} catch (CommonException e) {
		MessageBox( NULL, (LPCWSTR)e.What(), (LPCWSTR)L"Error", MB_ICONERROR |  MB_OK );
		return 1;
	}
}

// Message handler
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (pGame) {
		switch (message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
			break;
		case WM_LBUTTONDOWN:
			switch (wParam) {
			case MK_LBUTTON:
				POINT cursorPos;
				if (GetCursorPos(&cursorPos)) {
					pGame->GetVehiclePicker()->SetPickingRay(cursorPos.x, cursorPos.y);
					std::string vehicleName;
					if (pGame->GetVehiclePicker()->GetHitVehicle(vehicleName)) {
						pGame->SetActiveVehicle(vehicleName);
						pGame->SetDebugInfoString(L" New selection");
					} else {
						pGame->SetDebugInfoString(L" Nothing selected");
					}
				}
			}
		case WM_KEYDOWN:
			switch (wParam) {
			case VK_DOWN:
				pGame->MoveActiveVehicle(Game::Backward);
				break;
			case VK_UP:
				pGame->MoveActiveVehicle(Game::Forward);
				break;
			case VK_LEFT:
				pGame->Rotate(-1.0f);
				break;
			case VK_RIGHT:
				pGame->Rotate(1.0f);
				break;
			case VK_TAB:
				pGame->SetNextActiveVehicle();
				break;
			}
		default:
			// Handle any messages the switch statement didn't
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	} else {
		// game is not initialized yet or is terminating
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
