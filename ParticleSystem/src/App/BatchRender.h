#pragma once

#include "App/ParticleSystem.h"

class BatchRender : public ParticleSystem
{
public:
	BatchRender();

	virtual void OnUpdate(float ts, Camera& camera) override;
	virtual void OnRender(Camera& camera) override;

private:
	GLuint m_VAO = 0;
	GLuint quadVB, quadIB;
};
