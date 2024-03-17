#include "SingleDraw.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Random.h"


SingleDraw::SingleDraw()
	: ParticleSystem(SingleDrawMode),
	m_ParticleShader(std::make_unique<Shader>("assets/shaders/SingleDraw.vert", "assets/shaders/2DQuad.frag")),
	snowImage(std::make_unique<Image>("assets/textures/snow.png"))
{
	m_ParticleShader->use();
	m_ParticleShader->setInt("snowTexture", 0);
	m_ParticlePool.resize(maxQuantity);

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(1, &quadVB);
	glBindBuffer(GL_ARRAY_BUFFER, quadVB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, nullptr, GL_DYNAMIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, &vert[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texcoord));


	uint32_t indices[] = {
	0, 1, 2, 2, 3, 0
	};

	glGenBuffers(1, &quadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

}

void SingleDraw::OnUpdate(float ts)
{
	this->lifeParticle = 0;
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
		this->lifeParticle++;
	}
}

void SingleDraw::OnRender(Camera& camera)
{
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// activate shader
	m_ParticleShader->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, snowImage->GetTextureID());

	
	m_ParticleShader->setMat4("u_Projection", camera.GetProjection());
	m_ParticleShader->setMat4("u_View", camera.GetView());


	for (auto& particle : m_ParticlePool)
	{
		if (!particle.Active) continue;

		float life = particle.LifeRemaining / particle.LifeTime;
		float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

		glm::vec3 right = camera.GetRightDirection();
		glm::vec3 up = glm::normalize(glm::cross(right, camera.GetDirection()));
		glm::mat3 rotateMatrix = glm::rotate(glm::mat4(1.0f), particle.Rotation, camera.GetPosition() - glm::vec3(particle.Position));


		std::vector<Vertex> vert(4);
		vert[0].position = glm::vec3(particle.Position) + (rotateMatrix * (-right * size * 0.5f - up * size * 0.5f));
		vert[1].position = glm::vec3(particle.Position) + (rotateMatrix * (+right * size * 0.5f - up * size * 0.5f));
		vert[2].position = glm::vec3(particle.Position) + (rotateMatrix * (+right * size * 0.5f + up * size * 0.5f));
		vert[3].position = glm::vec3(particle.Position) + (rotateMatrix * (-right * size * 0.5f + up * size * 0.5f));
		
		vert[0].texcoord = glm::vec2(0.0f, 0.0f);
		vert[1].texcoord = glm::vec2(1.0f, 0.0f);
		vert[2].texcoord = glm::vec2(1.0f, 1.0f);
		vert[3].texcoord = glm::vec2(0.0f, 1.0f);
		
		glBindBuffer(GL_ARRAY_BUFFER, quadVB);
		//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vert), &vert[0].position.x);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * 4, &vert[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

	}
}

void SingleDraw::Emit(const ParticleProps& particleProps)
{
	Particle& particle = m_ParticlePool[(m_PoolIndex++) % maxQuantity];

	particle.Active = true;
	particle.Position = glm::vec4(particleProps.Position, 1.0f);
	particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

	// Velocity
	particle.Velocity = glm::vec4(particleProps.Velocity, 0.0f);
	particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float());
	particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float());
	particle.Velocity.z += particleProps.VelocityVariation.z * (Random::Float());

	particle.LifeTime = particleProps.LifeTime;
	particle.LifeRemaining = particleProps.LifeTime;
	particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
	particle.SizeBegin = particleProps.SizeBegin;
	particle.SizeEnd = particleProps.SizeEnd;
}
