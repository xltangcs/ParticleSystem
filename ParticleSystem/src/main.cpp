#include <memory>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Camera.h"
#include "Core/Application.h"
#include "Core/Time.h"
#include "Core/Random.h"

#include "App/ParticleSystem.h"
#include "App/SingleDraw.h"
#include "App/BatchRender.h"
#include "App/DrawInstance.h"
#include "App/ComputeShader.h"

class MyImGuiLayer : public ImGuiLayer
{
public:
	MyImGuiLayer()
		: m_Camera(45.0f, 0.1f, 100.0f), m_ParticleSystem(std::make_unique<BatchRender>()), m_ParticleType(SingleDrawMode)
	{
		glEnable(GL_DEPTH);
		// Init here
		m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
		m_Particle.LifeTime = 10.0f;
		m_Particle.Velocity = { 0.0f, 0.0f, 0.0f };
		m_Particle.VelocityVariation = { 0.00f, -3.00f, 0.00f};
		m_Particle.Position = { 0.0f, 0.0f, 0.0f};
	}

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
	}

	virtual void ShowUI(float ts) override
	{
		int width, height;
		auto window = Application::Get().GetGLFWwindow();
		glfwGetFramebufferSize(window, &width, &height);
		m_Width = width; m_Height = height;


		ImGui::Begin("Settings");
		ImGui::Text("The average fps: %.3f", ImGui::GetIO().Framerate);
		ImGui::Text("Total Time : %.3f", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("Update Time : %.3f", t1);
		ImGui::Text("Render Time : %.3f", t2);
		ImGui::Text("Number of particles : %d", m_ParticleSystem->lifeParticle);
		ImGui::Separator();

		ImGui::DragFloat3("Camera Position", glm::value_ptr(m_Camera.GetPosition()), 0.01f, -100.00f, 100.00f);
		ImGui::DragFloat3("Camera Direction", glm::value_ptr(m_Camera.GetDirection()), 0.01f, -100.00f, 100.00f);


		ImGui::Separator();

		//ImGui::Text("Particle Type");
		//ImGui::RadioButton("Single Draw", &m_ParticleType, SingleDrawMode); ImGui::SameLine();
		//ImGui::RadioButton("Batch Render", &m_ParticleType, BatchRenderMode); ImGui::SameLine();
		//ImGui::RadioButton("Draw Instance", &m_ParticleType, DrawInstanceMode); ImGui::SameLine();
		//ImGui::RadioButton("Compute Shader", &m_ParticleType, ComputeShaderMode);

		char* particleTypeName[] = {"Single Draw" , "Batch Render" , "Draw Instance" , "Compute Shader"};
		ImGui::Combo("Particle Type", &m_ParticleType, particleTypeName, 4 );

		ImGui::Checkbox("Snow", &isRun);
		ImGui::DragFloat("Birth Size", &m_Particle.SizeBegin, 0.01f, 0.00f, 1000.00f);
		ImGui::DragFloat("Life Time", &m_Particle.LifeTime, 0.01f, 0.01f, 1000.00f);
		ImGui::DragFloat3("Particle Velocity", glm::value_ptr(m_Particle.VelocityVariation), 0.01f, -10.00f, 100.00f);
		ImGui::DragInt("Emit Quantity", &singleParticleQuantity, 1, 1, 1000);

		ImGui::End();

	}

	virtual void Render(float ts) override
	{

		m_Camera.OnResize(m_Width, m_Height);

		if (m_ParticleType != m_ParticleSystem->m_ParticleType)
		{
			switch (m_ParticleType)
			{
			case SingleDrawMode:m_ParticleSystem = std::make_unique<SingleDraw>();
				break;
			case BatchRenderMode: m_ParticleSystem = std::make_unique<BatchRender>();
				break;
			case DrawInstanceMode: m_ParticleSystem = std::make_unique<DrawInstance>();
				break;
			case ComputeShaderMode: m_ParticleSystem = std::make_unique<ComputeShader>();
				break;
			default:
				break;
			}
		}

		if (isRun)
		{
			for (int i = 0; i < singleParticleQuantity; i++)
			{
				m_Particle.Position.x = Random::Float(m_ParticleSystem->xBounds.x, m_ParticleSystem->xBounds.y);
				m_Particle.Position.y = Random::Float(m_ParticleSystem->yBounds.x, m_ParticleSystem->yBounds.y);
				m_Particle.Position.z = Random::Float(m_ParticleSystem->zBounds.x, m_ParticleSystem->zBounds.y);
				m_ParticleSystem->Emit(m_Particle);
			}
		}

		m_Time.Reset();
		m_ParticleSystem->OnUpdate(ts);
		t1 = m_Time.ElapsedMillis();
		m_Time.Reset();
		m_ParticleSystem->OnRender(m_Camera);
		t2 = m_Time.ElapsedMillis();
	}

private:
	int singleParticleQuantity = 10;
	int m_Width = 10, m_Height = 10;

	Camera m_Camera;

	ParticleProps m_Particle;
	std::unique_ptr<ParticleSystem> m_ParticleSystem;
	int m_ParticleType;

	bool isRun = true;

	Timer m_Time;
	Timer m_Time1;
	float t1, t2;
};


int main()
{
	static Application* ParticleSystem = new Application("Particle System");
	std::shared_ptr<MyImGuiLayer> myimguilayer = std::make_shared<MyImGuiLayer>();

	ParticleSystem->PushImGuiLayer(myimguilayer);

	ParticleSystem->Run();

	return 0;
}