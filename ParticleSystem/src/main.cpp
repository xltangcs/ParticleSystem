#include <memory>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Renderer.h"
#include "Framebuffer.h"
#include "Application.h"
#include "OrthographicCamera.h"
#include "ParticleSystem.h"

class MyImGuiLayer : public ImGuiLayer
{
public:
	MyImGuiLayer()
		: m_Framebuffer(100, 80), m_Camera(45.0f, 0.1f, 100.0f), m_OrthographicCamera(16.0f / 9.0f, 1.0f)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Init here
		m_Particle.ColorBegin = {254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
		m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
		m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
		m_Particle.LifeTime = 1.0f;
		m_Particle.Velocity = { 0.0f, 0.0f };
		m_Particle.VelocityVariation = { 3.0f, 1.0f };
		m_Particle.Position = { 0.0f, 0.0f };
	}

	virtual void ShowUI(float ts) override
	{

		ImGui::Begin("Viewport");

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		ImGui::Image((ImTextureID)m_Framebuffer.GetTextureID(), viewportPanelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		ImGui::End();


		m_Framebuffer.Bind();//Render here

		ParticleSystem2D(ts);

		m_Framebuffer.Unbind();

		SettingUI();

	}

	void ParticleSystem2D(float ts)
	{
		GLFWwindow* window = Application::Get().GetGLFWwindow();

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		glm::vec2 mousePos((float)xpos, (float)ypos);


		ImGui::Begin("glfw pos");

		ImGui::DragFloat2("mouse pos", glm::value_ptr(mousePos));

		ImGui::End();

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			auto [mx, my] = ImGui::GetMousePos();

			mx -= m_ViewportBounds[0].x;
			my -= m_ViewportBounds[0].y;
			glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
			//my = viewportSize.y - my;
			int mouseX = (int)mx;
			int mouseY = (int)my;

			if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
			{
				float x = (mousePos.x / m_ViewportSize.x) * 2.0f * m_OrthographicCamera.GetAspectRatio() * m_OrthographicCamera.GetZoomLevel()
							- m_OrthographicCamera.GetAspectRatio() * m_OrthographicCamera.GetZoomLevel();
				float y = m_OrthographicCamera.GetZoomLevel() - (mousePos.y / m_ViewportSize.y) * m_OrthographicCamera.GetZoomLevel() * 2.0f;
				auto pos = m_OrthographicCamera.GetPosition();
				m_Particle.Position = { x + pos.x, y + pos.y };

				//float ndcX = (2.0f * mousePos.x) / m_ViewportSize.x - 1.0f;
				//float ndcY = 1.0f - (2.0f * mousePos.y) / m_ViewportSize.y;
				//glm::vec4 mouseNdc = glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
				//glm::vec3 mouseWorld = m_Camera.GetProjection() * m_Camera.GetInverseView() * mouseNdc;
				//m_Particle.Position = { mouseWorld.x, mouseWorld.y };
				
				for (int i = 0; i < 5; i++)
					m_ParticleSystem.Emit(m_Particle);
			}

		}

		m_ParticleSystem.OnUpdate(ts);
		m_ParticleSystem.OnRender(m_OrthographicCamera);

	}

	void SettingUI()
	{
		ImGui::Begin("Settings");

		ImGui::ColorEdit4("Birth Color", glm::value_ptr(m_Particle.ColorBegin));
		ImGui::ColorEdit4("Death Color", glm::value_ptr(m_Particle.ColorEnd));
		ImGui::DragFloat("Birth Size", &m_Particle.SizeBegin, 0.01f, 0.00f, 1000.00f);
		ImGui::DragFloat("Death Size", &m_Particle.SizeEnd, 0.01f, 0.00f, 1000.00f);
		ImGui::DragFloat("Life Time", &m_Particle.LifeTime, 0.01f, 0.00f, 1000.00f);

		ImGui::DragFloat("Zoom Level", &m_OrthographicCamera.GetZoomLevel(), 0.01f, 0.01f, 1000.00f);

		ImGui::End();
	}

	virtual void OnUpdate(float ts) override
	{
		glViewport(0, 0, (GLsizei)m_ViewportSize.x, (GLsizei)m_ViewportSize.y);

		m_OrthographicCamera.OnUpdate(ts);
		m_OrthographicCamera.OnResize(m_ViewportSize.x, m_ViewportSize.y);

		m_Framebuffer.Resize(m_ViewportSize.x, m_ViewportSize.y);
	}

private:
	glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
	glm::vec2 m_ViewportBounds[2];

	Camera m_Camera;
	OrthographicCamera m_OrthographicCamera;

    Framebuffer m_Framebuffer;
	Renderer m_Renderer;

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