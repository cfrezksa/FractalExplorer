#include "core\globalDefs.h"

// Include the Oculus SDK
#include "OVR_CAPI_GL.h"

#if defined(_WIN32)
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif

#include "fractals\FractalScene.h"
#include "vr\VRApplication.h"
#include "gl\DepthBuffer.h"
#include "fractals\Fractal.h"
#include "base\FractalShader.h"
using namespace OVR;

//-------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPSTR, int)
{
	// Initializes LibOVR, and the Rift
	ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
	ovrResult result = ovr_Initialize(&initParams);
	VALIDATE(OVR_SUCCESS(result), "Failed to initialize libOVR.");

	VALIDATE(VRApplication::Platform.InitWindow(hinst, L"Fractal Explorer VR (GL)"), "Failed to open window.");

	VRApplication::Platform.Run();

	ovr_Shutdown();

	return(0);
}
