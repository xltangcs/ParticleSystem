#include "BatchRender.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Random.h"


BatchRender::BatchRender()
	: ParticleSystem(BatchRenderMode)
{
	m_ParticleShader = std::make_unique<Shader>("assets/shaders/SingleDraw.vert", "assets/shaders/2DQuad.frag");
	snowImage = std::make_unique<Image>("assets/textures/snow.png");

	m_ParticleShader->use();
	m_ParticleShader->setInt("snowTexture", 0);
	m_ParticlePool.resize(maxQuantity);

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(1, &quadVB);
	glBindBuffer(GL_ARRAY_BUFFER, quadVB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4 * maxQuantity, nullptr, GL_DYNAMIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, &vert[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

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

	glGenBuffers(1, &quadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * maxQuantity * 6, &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

}

void BatchRender::OnUpdate(float ts, Camera& camera)
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

void BatchRender::OnRender(Camera& camera)
{
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// activate shader
	m_ParticleShader->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, snowImage->GetTextureID());


	m_ParticleShader->setMat4("u_Projection", camera.GetProjection());
	m_ParticleShader->setMat4("u_View", camera.GetView());

	std::vector<Vertex> vertexs;
	//Vertex* vertexs = new Vertex[lifeParticle * 4];
	
	for (auto& particle : m_ParticlePool)
	{
		if (!particle.Active) continue;

		float life = particle.LifeRemaining / particle.LifeTime;
		float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

		glm::vec3 right = camera.GetRightDirection();
		glm::vec3 up = glm::normalize(glm::cross(right, camera.GetDirection()));
		glm::mat3 rotateMatrix = glm::rotate(glm::mat4(1.0f), particle.Rotation, camera.GetPosition() - glm::vec3(particle.Position));

		Vertex vert[4];
		vert[0].position = glm::vec3(particle.Position) + (rotateMatrix * (-right * size * 0.5f - up * size * 0.5f));
		vert[1].position = glm::vec3(particle.Position) + (rotateMatrix * (+right * size * 0.5f - up * size * 0.5f));
		vert[2].position = glm::vec3(particle.Position) + (rotateMatrix * (+right * size * 0.5f + up * size * 0.5f));
		vert[3].position = glm::vec3(particle.Position) + (rotateMatrix * (-right * size * 0.5f + up * size * 0.5f));

		vert[0].texcoord = glm::vec2(0.0f, 0.0f);
		vert[1].texcoord = glm::vec2(1.0f, 0.0f);
		vert[2].texcoord = glm::vec2(1.0f, 1.0f);
		vert[3].texcoord = glm::vec2(0.0f, 1.0f);
		vertexs.push_back(vert[0]);
		vertexs.push_back(vert[1]);
		vertexs.push_back(vert[2]);
		vertexs.push_back(vert[3]);

	}

	glBindBuffer(GL_ARRAY_BUFFER, quadVB);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertexs.size(), &vertexs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, 6 * lifeParticle, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

}


