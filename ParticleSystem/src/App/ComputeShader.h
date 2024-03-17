#pragma once

#include "App/ParticleSystem.h"

struct ParticleGPU
{
	glm::vec4 Position;
	glm::vec4 Velocity;
	float Rotation = 0.0f;
	float LifeRemaining = 1.0f;

	float SizeBegin;
	bool Active = false;
};

struct VertexGPU {
	glm::vec4 position;
	glm::vec4 texcoord;
};

class ComputeShader : public ParticleSystem
{
public:
	ComputeShader();

	virtual void Emit(const ParticleProps& particleProps, int quantity) override;
	virtual void OnUpdate(float ts, Camera& camera) override;
	virtual void OnRender(Camera& camera) override;

private:
	std::unique_ptr<Shader> m_ComputeShader = nullptr;
	std::vector<ParticleGPU> m_ParticleGPUPool;

	GLuint m_VAO = 0;
	GLuint IBO, vertexBuffer, particleBuffer;
};
