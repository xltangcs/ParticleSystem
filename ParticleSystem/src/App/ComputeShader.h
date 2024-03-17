#pragma once

#include "App/ParticleSystem.h"

struct ParticleGPU
{
	glm::vec4 Position;
	glm::vec4 Velocity;
	float Rotation = 0.0f;
	float LifeRemaining = 1.0f;

	float SizeBegin;
	bool Active = true;
};

struct VertexGPU {
	glm::vec4 position;
	glm::vec4 texcoord;
};

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

	std::vector<Particle> m_ParticlePool;
	int m_PoolIndex = 0;

	GLuint m_VAO = 0;
	GLuint IBO, vertexBuffer, particleBuffer, modelBuffer;
};
