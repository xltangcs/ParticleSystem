#pragma once

#include "App/ParticleSystem.h"

class singleDraw : public ParticleSystem
{
public:
	singleDraw();

	virtual void Emit(const ParticleProps& particleProps) override;
	virtual void OnUpdate(float ts) override;
	virtual void OnRender(Camera& camera) override;

private:
	std::unique_ptr<Shader> m_ParticleShader = nullptr;
	std::unique_ptr<Image> snowImage = nullptr;

	const int maxQuantity = 10000;
	int m_PoolIndex = 0;

	std::vector<Particle> m_ParticlePool;
	Particle m_Particle;

	GLuint m_VAO = 0;

	GLuint quadVB, quadIB;

	GLuint vertBuffer;
};
