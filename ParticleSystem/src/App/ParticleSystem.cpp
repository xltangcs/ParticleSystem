#include "ParticleSystem.h"

#include "Core/Random.h"

void ParticleSystem::Emit(const ParticleProps& particleProps, int quantity)
{
	for (size_t i = 0; i < quantity; i++)
	{
		float x = Random::Float(this->xBounds.x, this->xBounds.y);
		float y = Random::Float(this->yBounds.x, this->yBounds.y);
		float z = Random::Float(this->zBounds.x, this->zBounds.y);

		Particle& particle = m_ParticlePool[(m_PoolIndex++) % maxQuantity];

		particle.Active = true;
		particle.Position = glm::vec3(x, y, z);
		particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

		particle.Velocity = particleProps.Velocity;
		particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float());
		particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float());
		particle.Velocity.z += particleProps.VelocityVariation.z * (Random::Float());

		particle.LifeTime = particleProps.LifeTime;
		particle.LifeRemaining = particleProps.LifeTime;
		particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
		particle.SizeEnd = particleProps.SizeEnd;
	}
}
