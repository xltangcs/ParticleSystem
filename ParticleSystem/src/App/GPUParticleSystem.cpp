#include "GPUParticleSystem.h"

#include "Core/Random.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/matrix_transform.hpp>

GPUParticleSystem::GPUParticleSystem()
	:ParticleSystem(GPU),
	m_ParticleShader(std::make_unique<Shader>("assets/shaders/GPU_particle.vert", "assets/shaders/2DQuad.frag")),
	m_ComputeShader(std::make_unique<Shader>("assets/shaders/compute.comp")),
	snowImage(std::make_unique<Image>("assets/textures/snow.png"))
{
	m_ParticlePool.resize(maxQuantity);
	m_ParticleShader->use();
	m_ParticleShader->setInt("snowTexture", 0);

	std::vector<Vertex> vertices(maxQuantity * 4);
	std::vector<uint32_t> indices(maxQuantity * 6);

	for (size_t i = 0; i < maxQuantity; i++)
	{
		vertices[i * 4 + 0].position = glm::vec3(-0.5f, -0.5f, 0.0f);
		vertices[i * 4 + 1].position = glm::vec3( 0.5f, -0.5f, 0.0f);
		vertices[i * 4 + 2].position = glm::vec3( 0.5f,  0.5f, 0.0f);
		vertices[i * 4 + 3].position = glm::vec3(-0.5f,  0.5f, 0.0f);

		vertices[i * 4 + 0].texcoord = glm::vec2(0.0f, 0.0f);
		vertices[i * 4 + 1].texcoord = glm::vec2(1.0f, 0.0f);
		vertices[i * 4 + 2].texcoord = glm::vec2(1.0f, 1.0f);
		vertices[i * 4 + 3].texcoord = glm::vec2(0.0f, 1.0f);

		indices[i * 6 + 0] = 0 + i * 6;
		indices[i * 6 + 1] = 1 + i * 6;
		indices[i * 6 + 2] = 2 + i * 6;
		indices[i * 6 + 3] = 2 + i * 6;
		indices[i * 6 + 4] = 3 + i * 6;
		indices[i * 6 + 5] = 0 + i * 6;
	}

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	GLuint VBO, IBO;

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * maxQuantity * 4, nullptr, GL_DYNAMIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

	glGenBuffers(1, &vertBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void GPUParticleSystem::Emit(const ParticleProps& particleProps)
{
	Particle& particle = m_ParticlePool[(m_PoolIndex++) % maxQuantity];

	particle.Active = true;
	particle.Position = particleProps.Position;
	particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

	// Velocity
	particle.Velocity = particleProps.Velocity;
	particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float());
	particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float());
	particle.Velocity.z += particleProps.VelocityVariation.z * (Random::Float());

	particle.LifeTime = particleProps.LifeTime;
	particle.LifeRemaining = particleProps.LifeTime;
	particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
	particle.SizeEnd = particleProps.SizeEnd;


}
void GPUParticleSystem::OnUpdate(float ts)
{
	m_ComputeShader->use();
	m_ComputeShader->setFloat("ts", ts);

	glDispatchCompute(1, 1, 1);
	
	//this->lifeParticle = 0;
	//for (auto& particle : m_ParticlePool)
	//{
	//	if (!particle.Active)
	//	{
	//		continue;
	//	}
	//
	//	if (particle.LifeRemaining <= 0.0f)
	//	{
	//		particle.Active = false;
	//		continue;
	//	}
	//
	//	particle.LifeRemaining -= ts;
	//	particle.Position += particle.Velocity * (float)ts;
	//	particle.Rotation += 0.01f * ts;
	//
	//	this->lifeParticle++;
	//}
}

void GPUParticleSystem::OnRender(Camera& camera)
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
		if (particle.Active)
		{
			float life = particle.LifeRemaining / particle.LifeTime;
			float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

			glm::mat4 transform = glm::translate(glm::mat4(1.0f), { particle.Position.x, particle.Position.y, particle.Position.z })
				* glm::rotate(glm::mat4(1.0f), particle.Rotation, { 0.0f, 0.0f, 1.0f })
				* glm::scale(glm::mat4(1.0f), { size, size, 1.0f });

			m_ParticleShader->setMat4("u_Model", transform);

			glBindVertexArray(m_VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		}
	}
}
