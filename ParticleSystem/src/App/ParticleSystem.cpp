#include "ParticleSystem.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Random.h"


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


	glBindVertexArray(m_QuadVA);
	
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, maxQuantity * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

	//glBufferData(GL_ARRAY_BUFFER, lifeParticle * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);


	// set attribute pointers for matrix (4 times vec4)
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);


	m_ParticleShader = std::make_unique<Shader>("assets/shaders/2DQuad.vert", "assets/shaders/2DQuad.frag");
	snowImage = std::make_unique<Image>("assets/textures/snow.png");

	m_ParticleShader->use();
	m_ParticleShader->setInt("texture1", 0);

	m_ParticlePool.resize(maxQuantity);
}

void ParticleSystem::OnUpdate(float ts)
{
	std::vector<glm::mat4> modelMatrices;

	for (auto& particle : m_ParticlePool)
	{
		if (!particle.Active)
		{
			continue;
		}

		if (particle.LifeRemaining <= 0.0f)
		{
			particle.Active = false;
			continue;
		}

		particle.LifeRemaining -= ts;
		particle.Position += particle.Velocity * (float)ts;
		particle.Rotation += 0.01f * ts;

		float life = particle.LifeRemaining / particle.LifeTime;
		float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { particle.Position.x, particle.Position.y, 0.0f })
			* glm::rotate(glm::mat4(1.0f), particle.Rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size, size, 1.0f });

		modelMatrices.push_back(transform);

	}

	lifeParticle = modelMatrices.size();

	glBindVertexArray(m_QuadVA);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lifeParticle * sizeof(glm::mat4), &modelMatrices[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleSystem::OnRender(Camera& camera)
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

	glBindVertexArray(m_QuadVA);
	glDrawElementsInstanced(GL_TRIANGLES, 6 * sizeof(uint32_t), GL_UNSIGNED_INT, 0, lifeParticle);
	glBindVertexArray(0);

}

void ParticleSystem::Emit(const ParticleProps& particleProps)
{
	Particle& particle = m_ParticlePool[(m_PoolIndex++) % maxQuantity];

	particle.Active = true;
	particle.Position = particleProps.Position;
	particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

	// Velocity
	particle.Velocity = particleProps.Velocity;
	particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
	particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);

	particle.LifeTime = particleProps.LifeTime;
	particle.LifeRemaining = particleProps.LifeTime;
	particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
	particle.SizeEnd = particleProps.SizeEnd;

}
