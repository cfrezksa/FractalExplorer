#include "vr\VRApplication.h"
#include "core\globalDefs.h"

// Include the Oculus SDK
#include "OVR_CAPI_GL.h"

#if defined(_WIN32)
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif

#include "fractals\FractalScene.h"
#include "gl\DepthBuffer.h"
#include "fractals\Fractal.h"


static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARBFunc = nullptr;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARBFunc = nullptr;

// Global OpenGL state
VRApplication VRApplication::Platform;

LRESULT CALLBACK VRApplication::WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	VRApplication *p = reinterpret_cast<VRApplication *>(GetWindowLongPtr(hWnd, 0));
	switch (Msg)
	{
	case WM_KEYDOWN:
		p->Key[wParam] = true;
		break;
	case WM_KEYUP:
		p->Key[wParam] = false;
		break;
	case WM_DESTROY:
		p->Running = false;
		break;
	default:
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	}
	if ((p->Key['Q'] && p->Key[VK_CONTROL]) || p->Key[VK_ESCAPE])
	{
		p->Running = false;
	}
	return 0;
}

VRApplication::VRApplication() {

	Window = nullptr;
	hDC = nullptr;
	WglContext = nullptr;
	hInstance = nullptr;

	Running = false;
	width = 0;
	height = 0;
	fboId = 0;

	// Clear input
	for (int i = 0; i < sizeof(Key) / sizeof(Key[0]); ++i)
		Key[i] = false;
}

VRApplication::~VRApplication()
{
	ReleaseDevice();
	CloseWindow();
}

bool VRApplication::InitWindow(HINSTANCE hInst, LPCWSTR title)
{
	hInstance = hInst;
	Running = true;

	WNDCLASSW wc;
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WindowProc;
	wc.cbWndExtra = sizeof(struct OGL *);
	wc.hInstance = GetModuleHandleW(NULL);
	wc.lpszClassName = L"ORT";
	RegisterClassW(&wc);

	// adjust the window size and show at InitDevice time
	Window = CreateWindowW(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hInstance, 0);
	if (!Window) return false;

	SetWindowLongPtr(Window, 0, LONG_PTR(this));

	hDC = GetDC(Window);

	return true;
}

void VRApplication::CloseWindow()
{
	if (Window)
	{
		if (hDC)
		{
			ReleaseDC(Window, hDC);
			hDC = nullptr;
		}
		DestroyWindow(Window);
		Window = nullptr;
		UnregisterClassW(L"OGL", hInstance);
	}
}

// Note: currently there is no way to get GL to use the passed pLuid
bool VRApplication::InitDevice(int w, int h, const LUID*, bool windowed)
{
	UNREFERENCED_PARAMETER(windowed);

	if (!InitWindow(w, h)) return false;
	InitGLContext();
	OVR::GLEContext::SetCurrentContext(&GLEContext);
	GLEContext.Init();
	InitFrameBuffer();

	return true;
}

void VRApplication::InitGLContext()
{

	InitWGLFunctions();

	// Now create the real context that we will be using.
	int iAttributes[] =
	{
		// WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 16,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
		0, 0
	};

	float fAttributes[] = { 0, 0 };
	int   pf = 0;
	UINT  numFormats = 0;

	VALIDATE(wglChoosePixelFormatARBFunc(hDC, iAttributes, fAttributes, 1, &pf, &numFormats),
		"wglChoosePixelFormatARBFunc failed.");

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	VALIDATE(SetPixelFormat(hDC, pf, &pfd), "SetPixelFormat failed.");

	GLint attribs[16];
	int   attribCount = 0;
	if (UseDebugContext)
	{
		attribs[attribCount++] = WGL_CONTEXT_FLAGS_ARB;
		attribs[attribCount++] = WGL_CONTEXT_DEBUG_BIT_ARB;
	}

	attribs[attribCount] = 0;

	WglContext = wglCreateContextAttribsARBFunc(hDC, 0, attribs);
	VALIDATE(wglMakeCurrent(hDC, WglContext), "wglMakeCurrent failed.");
}

void VRApplication::InitWGLFunctions()
{
	{
		// First create a context for the purpose of getting access to wglChoosePixelFormatARB / wglCreateContextAttribsARB.
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 16;
		int pf = ChoosePixelFormat(hDC, &pfd);
		VALIDATE(pf, "Failed to choose pixel format.");

		VALIDATE(SetPixelFormat(hDC, pf, &pfd), "Failed to set pixel format.");

		HGLRC context = wglCreateContext(hDC);
		VALIDATE(context, "wglCreateContextfailed.");
		VALIDATE(wglMakeCurrent(hDC, context), "wglMakeCurrent failed.");

		wglChoosePixelFormatARBFunc = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
		wglCreateContextAttribsARBFunc = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
		assert(wglChoosePixelFormatARBFunc && wglCreateContextAttribsARBFunc);

		wglDeleteContext(context);
	}
}

bool VRApplication::InitWindow(int w, int h)
{

	width = w;
	height = h;

	RECT size = { 0, 0, w, h };
	AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, false);
	const UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW;
	if (!SetWindowPos(Window, nullptr, 0, 0, size.right - size.left, size.bottom - size.top, flags)) {
		return false;
	}
	return true;
}

void VRApplication::InitFrameBuffer() {

	glGenFramebuffers(1, &fboId);

	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);

	if (UseDebugContext && GLE_ARB_debug_output)
	{
		glDebugMessageCallbackARB(DebugGLCallback, NULL);
		if (glGetError())
		{
			OVR_DEBUG_LOG(("glDebugMessageCallbackARB failed."));
		}

		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

		// Explicitly disable notification severity output.
		glDebugMessageControlARB(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	}
}

bool VRApplication::HandleMessages(void)
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return Running;
}

void VRApplication::Run()
{
	while (HandleMessages())
	{
		// true => we'll attempt to retry for ovrError_DisplayLost
		if (!MainLoop(true))
			break;
		// Sleep a bit before retrying to reduce CPU load while the HMD is disconnected
		Sleep(10);
	}
}

void VRApplication::ReleaseDevice()
{
	if (fboId)
	{
		glDeleteFramebuffers(1, &fboId);
		fboId = 0;
	}

	if (WglContext)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(WglContext);
		WglContext = nullptr;
	}

	GLEContext.Shutdown();
}

void GLAPIENTRY VRApplication::DebugGLCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	UNREFERENCED_PARAMETER(source);
	UNREFERENCED_PARAMETER(type);
	UNREFERENCED_PARAMETER(id);
	UNREFERENCED_PARAMETER(severity);
	UNREFERENCED_PARAMETER(length);
	UNREFERENCED_PARAMETER(message);
	UNREFERENCED_PARAMETER(userParam);
	OVR_DEBUG_LOG(("Message from OpenGL: %s\n", message));
}

ovrGraphicsLuid VRApplication::GetDefaultAdapterLuid()
{
	ovrGraphicsLuid luid = ovrGraphicsLuid();

#if defined(_WIN32)
	IDXGIFactory* factory = nullptr;

	if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
	{
		IDXGIAdapter* adapter = nullptr;

		if (SUCCEEDED(factory->EnumAdapters(0, &adapter)))
		{
			DXGI_ADAPTER_DESC desc;

			adapter->GetDesc(&desc);
			memcpy(&luid, &desc.AdapterLuid, sizeof(luid));
			adapter->Release();
		}

		factory->Release();
	}
#endif

	return luid;
}

static int Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs)
{
	return memcmp(&lhs, &rhs, sizeof(ovrGraphicsLuid));
}

// return true to retry later (e.g. after display lost)
bool  VRApplication::MainLoop(bool retryCreate)
{
	TextureBuffer * eyeRenderTexture[2] = { nullptr, nullptr };
	DepthBuffer   * eyeDepthBuffer[2] = { nullptr, nullptr };
	ovrMirrorTexture mirrorTexture = nullptr;
	GLuint          mirrorFBO = 0;
	FractalScene         * scene = nullptr;
	long long frameIndex = 0;

	ovrResult result = ovr_Create(&session, &luid);
	if (!OVR_SUCCESS(result))
		return retryCreate;

	if (Compare(luid, GetDefaultAdapterLuid())) // If luid that the Rift is on is not the default adapter LUID...
	{
		VALIDATE(false, "OpenGL supports only the default graphics adapter.");
	}

	ovrHmdDesc hmdDesc = ovr_GetHmdDesc(session);

	// Setup Window and Graphics
	// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
	ovrSizei windowSize = { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };
	if (!VRApplication::Platform.InitDevice(windowSize.w, windowSize.h, reinterpret_cast<LUID*>(&luid)))
		goto Done;

	// Make eye render buffers
	for (int eye = 0; eye < 2; ++eye)
	{
		ovrSizei idealTextureSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
		eyeRenderTexture[eye] = new TextureBuffer(session, true, true, idealTextureSize, 1, NULL);
		eyeDepthBuffer[eye] = new DepthBuffer(eyeRenderTexture[eye]->GetSize(), 0);

		if (!eyeRenderTexture[eye]->TextureChain())
		{
			if (retryCreate) goto Done;
			VALIDATE(false, "Failed to create texture.");
		}
	}

	ovrMirrorTextureDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = windowSize.w;
	desc.Height = windowSize.h;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

	// Create mirror texture and an FBO used to copy mirror texture to back buffer
	result = ovr_CreateMirrorTextureWithOptionsGL(session, &desc, &mirrorTexture);
	if (!OVR_SUCCESS(result))
	{
		if (retryCreate) goto Done;
		VALIDATE(false, "Failed to create mirror texture.");
	}

	// Configure the mirror read buffer
	GLuint texId;
	ovr_GetMirrorTextureBufferGL(session, mirrorTexture, &texId);

	glGenFramebuffers(1, &mirrorFBO);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// Turn off vsync to let the compositor do its magic
	wglSwapIntervalEXT(0);

	// Make scene - can simplify further if needed
	scene = new FractalScene();

	// FloorLevel will give tracking poses where the floor height is 0
	ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);

	// Main loop
	while (VRApplication::Platform.HandleMessages())
	{
		ovrSessionStatus sessionStatus;
		ovr_GetSessionStatus(session, &sessionStatus);
		if (sessionStatus.ShouldQuit)
		{
			// Because the application is requested to quit, should not request retry
			retryCreate = false;
			break;
		}
		if (sessionStatus.ShouldRecenter)
			ovr_RecenterTrackingOrigin(session);

		if (sessionStatus.IsVisible)
		{
			/*
			// Keyboard inputs to adjust player orientation

			*/
			// Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyePose) may change at runtime.
			ovrEyeRenderDesc eyeRenderDesc[2];
			eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
			eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

			// Get eye poses, feeding in correct IPD offset
			ovrPosef EyeRenderPose[2];
			ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose,	eyeRenderDesc[1].HmdToEyePose };

			double sensorSampleTime;    // sensorSampleTime is fed into the layer later
			ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyePose, EyeRenderPose, &sensorSampleTime);

			// Render Scene to Eye Buffers
			for (int eye = 0; eye < 2; ++eye)
			{
				// Switch to eye render target
				eyeRenderTexture[eye]->SetAndClearRenderSurface(eyeDepthBuffer[eye]);
				Matrix4f proj = ovrMatrix4f_Projection(hmdDesc.DefaultEyeFov[eye], 1.0f, 10.0f, ovrProjection_None);

				scene->Render(EyeRenderPose[eye], proj, eye);



				// Avoids an error when calling SetAndClearRenderSurface during next iteration.
				// Without this, during the next while loop iteration SetAndClearRenderSurface
				// would bind a framebuffer with an invalid COLOR_ATTACHMENT0 because the texture ID
				// associated with COLOR_ATTACHMENT0 had been unlocked by calling wglDXUnlockObjectsNV.
				eyeRenderTexture[eye]->UnsetRenderSurface();

				// Commit changes to the textures so they get picked up frame
				eyeRenderTexture[eye]->Commit();
			}

			// Do distortion rendering, Present and flush/sync

			ovrLayerEyeFov ld;
			ld.Header.Type = ovrLayerType_EyeFov;
			ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

			for (int eye = 0; eye < 2; ++eye)
			{
				ld.ColorTexture[eye] = eyeRenderTexture[eye]->TextureChain();
				ld.Viewport[eye] = Recti(eyeRenderTexture[eye]->GetSize());
				ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
				ld.RenderPose[eye] = EyeRenderPose[eye];
				ld.SensorSampleTime = sensorSampleTime;
			}

			ovrLayerHeader* layers = &ld.Header;
			result = ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);
			// exit the rendering loop if submit returns an error, will retry on ovrError_DisplayLost
			if (!OVR_SUCCESS(result))
				goto Done;

			frameIndex++;
		}

		// Blit mirror texture to back buffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		GLint w = windowSize.w;
		GLint h = windowSize.h;
		glBlitFramebuffer(0, h, w, 0,
			0, 0, w, h,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		VRApplication::Platform.Swap();
	}

Done:
	delete scene;
	if (mirrorFBO) glDeleteFramebuffers(1, &mirrorFBO);
	if (mirrorTexture) ovr_DestroyMirrorTexture(session, mirrorTexture);
	for (int eye = 0; eye < 2; ++eye)
	{
		delete eyeRenderTexture[eye];
		delete eyeDepthBuffer[eye];
	}
	VRApplication::Platform.ReleaseDevice();
	ovr_Destroy(session);

	// Retry on ovrError_DisplayLost
	return retryCreate || (result == ovrError_DisplayLost);
}
