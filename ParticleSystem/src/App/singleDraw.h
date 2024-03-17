#pragma once

#include "App/ParticleSystem.h"

class SingleDraw : public ParticleSystem
{
public:
	SingleDraw();

	virtual void OnUpdate(float ts, Camera& camera) override;
	virtual void OnRender(Camera& camera) override;

private:
	GLuint m_VAO = 0;
	GLuint IBO, VBO;
};
