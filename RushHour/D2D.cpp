#include "stdafx.h"
#include "D2D.h"
#include "CommonException.h"


D2D::D2D(D3D* d3d) : _d3d(d3d) {
	CreateDevice();
	CreateBitmapRenderTarget();
	InitializeTextFormats();
}


D2D::~D2D() {
	if (_dev) _dev->Release();
	if (_devCon) _devCon->Release();
	if (_factory) _factory->Release();
	if (_yellowBrush) _yellowBrush->Release();
	if (_whiteBrush) _whiteBrush->Release();
	if (_blackBrush) _blackBrush->Release();
	if (_textFormatFPS) _textFormatFPS->Release();
}

void D2D::CreateDevice() {
	if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&_writeFactory)))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create the DirectWrite factory!");
	}
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));
	//options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory2), &options, reinterpret_cast<LPVOID*>(&_factory)))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct2D Factory!");
	}
	if (FAILED(_factory->CreateDevice(_d3d->GetDXGIDevice(), &_dev))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create the Direct2D device!");
	}
	if (FAILED(_dev->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, &_devCon))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create the Direct2D device context!");
	}
}

void D2D::CreateBitmapRenderTarget() {
	// specify the desired bitmap properties
	D2D1_BITMAP_PROPERTIES1 bp;
	ZeroMemory(&bp, sizeof(D2D1_BITMAP_PROPERTIES1));
	bp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	bp.dpiX = 96.0f;
	bp.dpiY = 96.0f;
	bp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	bp.colorContext = nullptr;

	// Direct2D needs the DXGI version of the back buffer
	IDXGISurface* dxgiBuffer;
	if (FAILED(_d3d->GetSwapChain()->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<LPVOID*>(&dxgiBuffer)))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to retrieve the Direct2D back buffer!");
	}

	// create the bitmap
	ID2D1Bitmap1* targetBitmap;
	if (FAILED(_devCon->CreateBitmapFromDxgiSurface(dxgiBuffer, &bp, &targetBitmap))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create the Direct2D bitmap from the DXGI surface!");
	}

	// set the newly created bitmap as render target
	_devCon->SetTarget(targetBitmap);
}

void D2D::InitializeTextFormats() {
	// create standard brushes
	if (FAILED(_devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &_yellowBrush)))
		throw CommonException((LPWSTR)L"Critical error: Unable to create the yellow brush!");
	if (FAILED(_devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &_blackBrush)))
		throw CommonException((LPWSTR)L"Critical error: Unable to create the black brush!");
	if (FAILED(_devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &_whiteBrush)))
		throw CommonException((LPWSTR)L"Critical error: Unable to create the white brush!");

	// set up text formats
	// FPS text
	if (FAILED(_writeFactory->CreateTextFormat(L"Lucida Console", nullptr, DWRITE_FONT_WEIGHT_LIGHT, 
		DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-GB", &_textFormatFPS)))
		throw CommonException((LPWSTR)L"Critical error: Unable to create text format for FPS information!");
	if (FAILED(_textFormatFPS->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)))
		throw CommonException((LPWSTR)L"Critical error: Unable to set text alignment!");
	if (FAILED(_textFormatFPS->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)))
		throw CommonException((LPWSTR)L"Critical error: Unable to set paragraph alignment!");
}

void D2D::PrintInfo() {
	_devCon->BeginDraw();
	_devCon->DrawTextLayout(D2D1::Point2F(INFOPOSITIONX, INFOPOSITIONY), _textLayoutFPS, _yellowBrush);
	if (FAILED(_devCon->EndDraw())) {
		throw CommonException((LPWSTR)L"Critical error: Unable to draw FPS information!");
	}
}
