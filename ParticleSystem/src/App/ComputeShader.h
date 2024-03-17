#pragma once

#include "App/ParticleSystem.h"

class ComputeShader : public ParticleSystem
{
public:
	ComputeShader();

	virtual void Emit(const ParticleProps& particleProps) override;
	virtual void OnUpdate(float ts) override;
	virtual void OnRender(Camera& camera) override;

private:
	std::unique_ptr<Shader> m_ComputeShader = nullptr;
	std::unique_ptr<Shader> m_ParticleShader = nullptr;
	std::unique_ptr<Image> snowImage = nullptr;

	int m_PoolIndex = 0;

	std::vector<Particle> m_ParticlePool;

	GLuint m_VAO = 0;

	GLuint vertBuffer;
};
