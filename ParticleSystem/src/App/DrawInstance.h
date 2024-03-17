#pragma once

#include "App/ParticleSystem.h"

class DrawInstance : public ParticleSystem
{
public:
	DrawInstance();

	virtual void OnUpdate(float ts, Camera& camera) override;
	virtual void OnRender(Camera& camera) override;

private:
	GLuint m_QuadVA = 0;
	unsigned int buffer;
};