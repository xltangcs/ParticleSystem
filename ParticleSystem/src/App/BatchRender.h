#pragma once

#include "App/ParticleSystem.h"

class BatchRender : public ParticleSystem
{
public:
	BatchRender();

	virtual void Emit(const ParticleProps& particleProps) override;
	virtual void OnUpdate(float ts) override;
	virtual void OnRender(Camera& camera) override;

private:
	std::unique_ptr<Shader> m_ParticleShader = nullptr;
	std::unique_ptr<Image> snowImage = nullptr;

	std::vector<Particle> m_ParticlePool;
	
	int m_PoolIndex = 0;

	GLuint m_VAO = 0;
	GLuint quadVB, quadIB;
};
