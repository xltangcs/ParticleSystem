#include "ParticleSystem.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Random.h"


ParticleSystem::ParticleSystem()
{
	float vertices[] = {
	 -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
	  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
	  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
	 -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
	};

	uint32_t indices[] = {
		0, 1, 2, 2, 3, 0
	};

	glGenVertexArrays(1, &m_QuadVA);
	glBindVertexArray(m_QuadVA);

	GLuint quadVB, quadIB;
	glGenBuffers(1, &quadVB);
	glBindBuffer(GL_ARRAY_BUFFER, quadVB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &quadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	m_ParticleShader = std::make_unique<Shader>("assets/shaders/2DQuad.vert", "assets/shaders/2DQuad.frag");
	snowImage = std::make_unique<Image>("assets/textures/snow.png");

	m_ParticleShader->use();
	m_ParticleShader->setInt("texture1", 0);

	m_ParticlePool.resize(1000);
}

void ParticleSystem::OnUpdate(float ts)
{
	for (auto& particle : m_ParticlePool)
	{
		if (!particle.Active)
			continue;

		if (particle.LifeRemaining <= 0.0f)
		{
			particle.Active = false;
			continue;
		}

		particle.LifeRemaining -= ts;
		particle.Position += particle.Velocity * (float)ts;
		particle.Rotation += 0.01f * ts;
	}
}

void ParticleSystem::OnRender(OrthographicCamera& camera)
{
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// activate shader
	m_ParticleShader->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, snowImage->GetTextureID());

	glm::mat4 projection = camera.GetProjection();
	m_ParticleShader->setMat4("u_Projection", projection);

	glm::mat4 view = camera.GetView();
	m_ParticleShader->setMat4("u_View", view);

	for (auto& particle : m_ParticlePool)
	{
		if (!particle.Active)
			continue;

		// Fade away particles
		float life = particle.LifeRemaining / particle.LifeTime;
		glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);
		//color.a = color.a * life;

		float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

		// Render
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { particle.Position.x, particle.Position.y, 0.0f })
			* glm::rotate(glm::mat4(1.0f), particle.Rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size, size, 1.0f });

		m_ParticleShader->setMat4("u_Model", transform);
		m_ParticleShader->setVec4("u_Color", color);



		glBindVertexArray(m_QuadVA);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}
}

void ParticleSystem::Emit(const ParticleProps& particleProps)
{
	Particle& particle = m_ParticlePool[m_PoolIndex];
	particle.Active = true;
	particle.Position = particleProps.Position;
	particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

	// Velocity
	particle.Velocity = particleProps.Velocity;
	particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
	particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);

	// Color
	particle.ColorBegin = particleProps.ColorBegin;
	particle.ColorEnd = particleProps.ColorEnd;

	particle.LifeTime = particleProps.LifeTime;
	particle.LifeRemaining = particleProps.LifeTime;
	particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
	particle.SizeEnd = particleProps.SizeEnd;

	m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
	m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
}
