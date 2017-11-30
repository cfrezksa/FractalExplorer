#pragma once

#include "fractals\Fractal.h"

//------------------------------------------------------------------------- 
class FractalScene
{
public:
	FractalScene();
	virtual ~FractalScene();
	void nextModel();
	void prevModel();
	void Render(ovrPosef eyePose, Matrix4f proj, int eye);

private:
	void Update(ovrPosef eyePose);
	void Render(Matrix4f view, Matrix4f proj, int eye);
	int currentModel;
	std::vector<Fractal *> models;
	time_t startTime;
	float elapsedTime;

	Matrix4f camRotation;
	Vector3f camPosition;
};
