#pragma once

#include <d3d11.h>
#include <DXGI.h>
#include <D2D1.h>
#include <D2D1_2.h>
#include <dwrite_2.h>
#include "D3D.h"

class D2D {
public:
	D2D() = delete;
	D2D(D3D* d3d);
	~D2D();

	void PrintInfo();                      // prints info to the screen
	IDWriteTextFormat* GetTextFormatFPS() { return _textFormatFPS; }
	IDWriteTextLayout* GetTextLayoutFPS() { return _textLayoutFPS; }
	IDWriteTextLayout** GetTextLayoutFPSAddr() { return &(_textLayoutFPS); }
	IDWriteFactory2 * GetWriteFactory() { return _writeFactory; }


private:
	// device, factory
	IDWriteFactory2 * _writeFactory;	// pointer to the DirectWrite factory
	ID2D1Factory2* _factory;			// pointer to the Direct2D factory
	ID2D1Device1* _dev;				    // pointer to the Direct2D device
	ID2D1DeviceContext1* _devCon;		// pointer to the device context
	D3D* _d3d;

	// brushes
	ID2D1SolidColorBrush* _yellowBrush;
	ID2D1SolidColorBrush* _whiteBrush;
	ID2D1SolidColorBrush* _blackBrush;

	// text formats
	IDWriteTextFormat* _textFormatFPS;

	// text layouts
	IDWriteTextLayout* _textLayoutFPS;

	void CreateDevice();				// creates the device and its context
	void CreateBitmapRenderTarget();	// creates the bitmap render target, set to be the same as the backbuffer already in use for Direct3D
	void InitializeTextFormats();		// initializes the different formats, for now, only a format to print FPS information will be created

};

