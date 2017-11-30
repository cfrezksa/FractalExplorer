#pragma once

#include "../GLE/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include <assert.h>
#include <iostream>

//-------------------------------------------------------------------------------------------
class VRApplication
{
public:
	// Global OpenGL state
	static VRApplication Platform;
	// Note: currently there is no way to get GL to use the passed pLuid
	bool InitDevice(int vpW, int vpH, const LUID* /*pLuid*/, bool windowed = true);

	void Run();
	void ReleaseDevice();
	bool InitWindow(HINSTANCE hInst, LPCWSTR title);
	inline void Swap() { SwapBuffers(hDC); }
	inline bool IsKeyDown(char keyCode) { return Key[keyCode]; }
	inline ovrSession Session() { return session; }

protected:
	static LRESULT CALLBACK WindowProc(
		_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

	VRApplication();
	virtual ~VRApplication();

	void CloseWindow();
	void InitGLContext();
	void InitWGLFunctions();
	void InitFrameBuffer();
	bool HandleMessages(void);
	bool InitWindow(int w, int h);
	bool  MainLoop(bool retryCreate);
	static ovrGraphicsLuid GetDefaultAdapterLuid();

	static void GLAPIENTRY DebugGLCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

	static const bool       UseDebugContext = false;

	HWND                    Window;
	HDC                     hDC;
	HGLRC                   WglContext;
	OVR::GLEContext         GLEContext;
	bool                    Running;
	bool                    Key[256];
	int                     width;
	int                     height;
	GLuint                  fboId;
	HINSTANCE               hInstance;
	ovrSession session;
	ovrGraphicsLuid luid;
};

