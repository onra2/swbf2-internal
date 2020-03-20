#pragma once

#include<windows.h>
#include<d2d1.h>
#include<dwrite.h>
#include<dwmapi.h>
#include<string>
#include<fstream>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dwrite.lib")


class Direct2DOverlay;

typedef void(*DrawCallback)(Direct2DOverlay* dis);

class Direct2DOverlay
{
public:
	Direct2DOverlay(HINSTANCE hInstance, DrawCallback callbacker);
	~Direct2DOverlay();
	bool Initialize(std::string gameWindowName, std::string overlayWindowName);
	bool IsGood();
	void StartLoop();
	void BeginDraw();
	void ClearScreen();
	void ClearScreen(float r, float g, float b, float a);
	void DrawBox(float x, float y, float width, float height, float thicc, float r, float g, float b, float a, bool filled);
	void DrawCircle(float x, float y, float radius, float thicc, float r, float g, float b, float a, bool filled);
	void DrawLine(float x1, float y1, float x2, float y2, float thicc, float r, float g, float b);
	void DrawString(std::wstring str, float fontSize, float x, float y, float r, float g, float b);
	void hack_DrawCrosshair1(float r, float g, float b, float thicc, float len, float xoffset, float yoffset);
	void EndDraw();
	RECT GetOverlayRect();
private:
	ID2D1Factory* factory;
	ID2D1HwndRenderTarget* target;
	ID2D1SolidColorBrush* solid_brush;
	IDWriteFactory* w_factory;
	IDWriteTextFormat* w_format;
	IDWriteTextLayout* w_layout;
	bool good;
	bool loopRunning;
	bool drawing;
	std::string gameWindowName;
	HWND overlayWindow;
	HINSTANCE appInstance;
	DrawCallback callback;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam);
};