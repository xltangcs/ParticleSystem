#pragma once

#include "App/ParticleSystem.h"

class CPUParticleSystem : public ParticleSystem
{
public:
	CPUParticleSystem();

	virtual void Emit(const ParticleProps& particleProps) override;
	virtual void OnUpdate(float ts) override;
	virtual void OnRender(Camera& camera) override;

private:
	const int maxQuantity = 10000;
	int m_PoolIndex = 0;

	std::vector<Particle> m_ParticlePool;

	GLuint m_QuadVA = 0;
	unsigned int buffer;
	std::unique_ptr<Shader> m_ParticleShader = nullptr;

	std::unique_ptr<Image> snowImage = nullptr;
};