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
	glm::vec3 Position;
	glm::vec3 Velocity, VelocityVariation;
	float SizeBegin, SizeEnd, SizeVariation;
	float LifeTime = 1.0f;
};

struct Particle
{
	glm::vec4 Position;
	glm::vec4 Velocity;

	float Rotation = 0.0f;
	float LifeRemaining = 0.0f;

	float SizeBegin;
	float SizeEnd;
	float LifeTime = 1.0f;
	bool Active = false;
};

enum ParticleType
{
	SingleDrawMode = 0,
	BatchRenderMode = 1,
	DrawInstanceMode = 2,
	ComputeShaderMode = 3
};

struct Vertex {
	glm::vec3 position;
	glm::vec2 texcoord;
};

class ParticleSystem
{
public:
	int lifeParticle = 0;
	ParticleType m_ParticleType;
	const int maxQuantity = 100000;

	glm::vec2 xBounds = glm::vec2(-10.0f, 10.0f);
	glm::vec2 yBounds = glm::vec2(3.0f, 4.0f);
	glm::vec2 zBounds = glm::vec2(-21.0f, -1.0f);


public:
	ParticleSystem(ParticleType type) : m_ParticleType(type) {};
	virtual ~ParticleSystem() = default;

	virtual void Emit(const ParticleProps& particleProps) { };
	virtual void OnUpdate(float ts) { };
	virtual void OnRender(Camera& camera) {};
};