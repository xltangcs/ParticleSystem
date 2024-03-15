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

#include "App/Framebuffer.h"
#include "App/ParticleSystem.h"

Timer m_Time;
Timer m_Time1;
float t1, t2;

class MyImGuiLayer : public ImGuiLayer
{
public:
	MyImGuiLayer()
		: m_Framebuffer(100, 80), m_Camera(45.0f, 0.1f, 100.0f)
	{

		// Init here
		m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
		m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
		m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
		m_Particle.LifeTime = 1.0f;
		m_Particle.Velocity = { 0.0f, 0.0f };
		m_Particle.VelocityVariation = { 3.01f, 2.01f };
		m_Particle.Position = { 0.0f, 0.0f };
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
		ImGui::Text("Number of particles : %d", m_ParticleSystem.ParticleQuantity());


		ImGui::Separator();

		ImGui::DragFloat("Birth Size", &m_Particle.SizeBegin, 0.01f, 0.00f, 1000.00f);
		ImGui::DragFloat("Death Size", &m_Particle.SizeEnd, 0.01f, 0.00f, 1000.00f);
		ImGui::DragFloat("Life Time", &m_Particle.LifeTime, 0.01f, 0.00f, 1000.00f);
		ImGui::DragInt("Emit Quantity", &particleQuantity);

		ImGui::End();

	}

	virtual void Render(float ts) override
	{

		m_Camera.OnResize(m_Width, m_Height);

		GLFWwindow* window = Application::Get().GetGLFWwindow();

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			
			if (xpos >= 0 && ypos >= 0 && xpos < m_Width && ypos < m_Height)
			{
				float ndcX = (2.0f * xpos) / m_Width - 1.0f;
				float ndcY = 1.0f - (2.0f * ypos) / m_Height;
				glm::vec4 mouseNdc = glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
				glm::vec3 mouseWorld = m_Camera.GetProjection() * m_Camera.GetInverseView() * mouseNdc;

				m_Particle.Position = { mouseWorld.x, mouseWorld.y };

				for (int i = 0; i < particleQuantity; i++)
					m_ParticleSystem.Emit(m_Particle);
			}
		}


		m_Time.Reset();
		m_ParticleSystem.OnUpdate(ts);
		t1 = m_Time.ElapsedMillis();
		m_Time.Reset();
		m_ParticleSystem.OnRender(m_Camera);
		t2 = m_Time.ElapsedMillis();
	}

private:
	int particleQuantity = 10000;
	int m_Width = 10, m_Height = 10;

	Camera m_Camera;

	Framebuffer m_Framebuffer;

	ParticleProps m_Particle;
	ParticleSystem m_ParticleSystem;
};


int main()
{
	static Application* ParticleSystem = new Application("Particle System");
	std::shared_ptr<MyImGuiLayer> myimguilayer = std::make_shared<MyImGuiLayer>();

	ParticleSystem->PushImGuiLayer(myimguilayer);

	ParticleSystem->Run();

	return 0;
}