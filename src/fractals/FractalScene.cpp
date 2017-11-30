#pragma once
#include "fractals\FractalScene.h"
#pragma once

#include "core\globalDefs.h"
#include "fractals\Fractal.h"
#include <assert.h>
#include <fstream>
#include <time.h>
#include <strstream>
#include "gl\ShaderProgram.h"
#include "vr\VRApplication.h"

//------------------------------------------------------------------------- 

FractalScene::FractalScene() {

	elapsedTime = 0.0;
	time(&startTime);
	currentModel = 0;
	Fractal *model = NULL;

	camRotation = Matrix4f::Identity();
	camPosition = Vector3f(0.0f, 0.0f, -3.0f);

	model = new Fractal("../fractals/ApollonianCRS02.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/standard.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/Apollonian.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/ApollonianCRS.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/MandelBulb01.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/MandelBulb02.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/MandelBulb03.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/MandelBulb04b.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/MandelBulb05b.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/MandelBoxB.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/MandelBoxA.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/MandelBoxC.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/Hellfire.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/MengerSponge.fractal");
	models.push_back(model);
	model = new Fractal("../fractals/SeaCreature.fractal");
	models.push_back(model);
	//model = new Fractal("../fractals/SpecialA.fractal");
	//models.push_back(model);
	//model = new Fractal("../fractals/SpecialB.fractal");
	//models.push_back(model);

}

FractalScene::~FractalScene()
{
	for each (auto model in models) {
		delete model;
	}
}

void FractalScene::Update(ovrPosef eyePose) {

	static float Yaw(3.141592f);

	ovrInputState touchState;
	ovrResult touchOK = ovr_GetInputState(VRApplication::Platform.Session(), ovrControllerType_Touch, &touchState);

	if (OVR_SUCCESS(touchOK)) {
		Vector3f move = Vector3f(
			touchState.Thumbstick[0].x,
			touchState.Thumbstick[0].y,
			-touchState.Thumbstick[1].y) * 0.01f;

		camPosition += (camRotation * Matrix4f(eyePose.Orientation)).Transform(move);

		if (abs(touchState.Thumbstick[1].x) > 0.3f) {
			float offset = (touchState.Thumbstick[1].x > 0.0f) ? 0.3f : -0.3f;
			Yaw -= 0.01f * (touchState.Thumbstick[1].x - offset);
		}
				
		static bool isNDown = false;
		if (touchState.Buttons & ovrButton::ovrButton_RThumb) {
			if (isNDown == false) {
				Yaw = 3.141592f;
				camPosition = Vector3f(0, 0, -3.0f);
				nextModel();
				isNDown = true;
			}
		}
		else {
			isNDown = false;
		}

		static bool isPDown = false;
		if (touchState.Buttons & ovrButton::ovrButton_LThumb) {
			if (isPDown == false) {
				Yaw = 3.141592f;
				camPosition = Vector3f(0, 0, -3);
				prevModel();
				isPDown = true;
			}
		}
		else {
			isPDown = false;
		}
	}
	// Get view and projection matrices
	camRotation = Matrix4f::RotationY(Yaw);
}

void FractalScene::Render(ovrPosef eyePose, Matrix4f proj, int eye)
{
	Update(eyePose);

	Matrix4f finalRollPitchYaw = camRotation * Matrix4f(eyePose.Orientation);
	Vector3f finalUp = finalRollPitchYaw.Transform(Vector3f(0, 1, 0));
	Vector3f finalForward = finalRollPitchYaw.Transform(Vector3f(0, 0, -1));
	Vector3f shiftedEyePos = camPosition + camRotation.Transform(eyePose.Position);
	Matrix4f view = Matrix4f::LookAtRH(shiftedEyePos, camPosition + finalForward * 10.0f, finalUp);
	//Matrix4f view = Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);

	Render(view, proj, eye);
}
void FractalScene::Render(Matrix4f view, Matrix4f proj, int eye)
{	
	elapsedTime += 0.02f;

	float fovScale = 0.8f;// 2 + abs(sin(0.1*elapsedTime));
	float amount = 0.02f;// abs(sin(elapsedTime));
	Fractal *model = models[currentModel];

	// set standard parameters
	model->setVector("screenResolution", Vector2f(1344.0, 1600.0));
	model->setMatrix("matProj", proj);
	model->setMatrix("matView", view);
	model->setFloat("time", elapsedTime);
	model->setFloat("eyeShift", (eye == 0) ? -amount : amount);
	model->setFloat("fovScale", fovScale);

	model->draw();

}

void FractalScene::nextModel() {
	currentModel++;
	elapsedTime = 0.0;
	if (currentModel >= (int) models.size()) {
		currentModel = 0;
	}
}


void FractalScene::prevModel() {
	currentModel--;
	elapsedTime = 0.0;
	if (currentModel < 0) {
		currentModel = (int) models.size() - 1;
	}
}
