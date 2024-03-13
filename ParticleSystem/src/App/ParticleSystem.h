#pragma once

#include <list>
#include <memory>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Core/Shader.h"
#include "Core/Camera.h"
#include "Core/Image.h"


struct ParticleProps
{
	glm::vec2 Position;
	glm::vec2 Velocity, VelocityVariation;
	glm::vec4 ColorBegin, ColorEnd;
	float SizeBegin, SizeEnd, SizeVariation;
	float LifeTime = 1.0f;
};

class ParticleSystem
{
public:
	ParticleSystem();

	void OnUpdate(float ts);
	void OnRender(Camera& camera);

	void Emit(const ParticleProps& particleProps);

	int ParticleQuantity() { return lifeParticle; }

private:
	struct Particle
	{
		glm::vec2 Position;
		glm::vec2 Velocity;
		glm::vec4 ColorBegin, ColorEnd;
		float Rotation = 0.0f;
		float SizeBegin, SizeEnd;

		float LifeTime = 1.0f;
		float LifeRemaining = 0.0f;

		bool Active = false;
	};
	const int maxQuantity = 20000;

	int lifeParticle = 0;
	int m_PoolIndex = 0;

	std::vector<Particle> m_ParticlePool;

	GLuint m_QuadVA = 0;
	std::unique_ptr<Shader> m_ParticleShader = nullptr;

	std::unique_ptr<Image> snowImage = nullptr;
};