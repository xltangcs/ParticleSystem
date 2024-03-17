#include "ComputeShader.h"

#include "Core/Random.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/matrix_transform.hpp>

ComputeShader::ComputeShader()
	:ParticleSystem(ComputeShaderMode),
	m_ComputeShader(std::make_unique<Shader>("assets/shaders/compute.comp"))
{
	m_ParticleShader = std::make_unique<Shader>("assets/shaders/computeShader.vert", "assets/shaders/2DQuad.frag");
	snowImage = std::make_unique<Image>("assets/textures/snow.png");

	m_ParticleGPUPool.resize(maxQuantity);
	m_ParticleShader->use();
	m_ParticleShader->setInt("snowTexture", 0);

	float vertices[] = {
	 -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
	  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
	  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
	 -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
	};

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	std::vector<uint32_t> indices(maxQuantity * 6);
	for (size_t i = 0; i < maxQuantity; i++)
	{
		indices[i * 6 + 0] = 0 + 4 * i;
		indices[i * 6 + 1] = 1 + 4 * i;
		indices[i * 6 + 2] = 2 + 4 * i;
		indices[i * 6 + 3] = 2 + 4 * i;
		indices[i * 6 + 4] = 3 + 4 * i;
		indices[i * 6 + 5] = 0 + 4 * i;
	}

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * maxQuantity * 6, &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);


	//ParticleGPU p[3];
	//p[0].Position = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	//p[0].Velocity = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	//
	//p[1].Position = glm::vec4(2.0f, 0.0f, 0.0f, 0.0f);
	//p[1].Velocity = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	//
	//p[2].Position = glm::vec4(3.0f, 0.0f, 0.0f, 0.0f);
	//p[2].Velocity = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);


	glGenBuffers(1, &particleBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ParticleGPU) * maxQuantity, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(VertexGPU) * maxQuantity * 4, nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void ComputeShader::Emit(const ParticleProps& particleProps, int quantity)
{
	for (size_t i = 0; i < quantity; i++)
	{
		float x = Random::Float(this->xBounds.x, this->xBounds.y);
		float y = Random::Float(this->yBounds.x, this->yBounds.y);
		float z = Random::Float(this->zBounds.x, this->zBounds.y);

		ParticleGPU& particle = m_ParticleGPUPool[(m_PoolIndex++) % maxQuantity];

		particle.Position = glm::vec4(x, y ,z, 1.0f);

		particle.Velocity = glm::vec4(particleProps.Velocity, 0.0f);
		particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float());
		particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float());
		particle.Velocity.z += particleProps.VelocityVariation.z * (Random::Float());

		particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

		particle.LifeRemaining = particleProps.LifeTime;

		particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);

		particle.Active = true;

		m_ComputeShader->use();
		m_ComputeShader->setFloat("LifeTime", particleProps.LifeTime);

	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ParticleGPU) * maxQuantity, &m_ParticleGPUPool[0]);

}
void ComputeShader::OnUpdate(float ts, Camera& camera)
{
	m_ComputeShader->use();
	m_ComputeShader->setFloat("ts", ts);
	m_ComputeShader->setVec3("cameraDirection", camera.GetDirection());
	m_ComputeShader->setVec3("cameraPosition", camera.GetPosition());

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertexBuffer);

	glDispatchCompute(maxQuantity, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ParticleGPU) * maxQuantity, &m_ParticleGPUPool[0]);

}

void ComputeShader::OnRender(Camera& camera)
{
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// activate shader
	m_ParticleShader->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, snowImage->GetTextureID());

	m_ParticleShader->setMat4("u_Projection", camera.GetProjection());
	m_ParticleShader->setMat4("u_View", camera.GetView());

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexGPU), (void*)offsetof(VertexGPU, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexGPU), (void*)offsetof(VertexGPU, texcoord));

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, 6 * maxQuantity, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}
