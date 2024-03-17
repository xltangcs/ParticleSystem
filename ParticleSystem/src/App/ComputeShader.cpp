#include "ComputeShader.h"

#include "Core/Random.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/matrix_transform.hpp>

ComputeShader::ComputeShader()
	:ParticleSystem(ComputeShaderMode),
	m_ParticleShader(std::make_unique<Shader>("assets/shaders/computeShader.vert", "assets/shaders/2DQuad.frag")),
	m_ComputeShader(std::make_unique<Shader>("assets/shaders/compute.comp")),
	snowImage(std::make_unique<Image>("assets/textures/snow.png"))
{
	m_ParticlePool.resize(maxQuantity);
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

	//glGenBuffers(1, &vertexBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	////glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(VertexGPU) * maxQuantity * 4 , nullptr, GL_STATIC_DRAW);
	//
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	//
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

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


	ParticleGPU p[3];
	p[0].Position = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	p[0].Velocity = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

	p[1].Position = glm::vec4(2.0f, 0.0f, 0.0f, 0.0f);
	p[1].Velocity = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

	p[2].Position = glm::vec4(3.0f, 0.0f, 0.0f, 0.0f);
	p[2].Velocity = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	glGenBuffers(1, &particleBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ParticleGPU) * 3, p, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(VertexGPU) * 12, nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void ComputeShader::Emit(const ParticleProps& particleProps)
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
void ComputeShader::OnUpdate(float ts)
{
	m_ComputeShader->use();
	m_ComputeShader->setFloat("ts", ts);

	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertexBuffer);

	glDispatchCompute(3, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	ParticleGPU p[3];
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ParticleGPU) * 3, p);

	VertexGPU m[3 * 4];
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(VertexGPU) * 12, m);

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
	glDrawElements(GL_TRIANGLES, 6 * 3, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}
