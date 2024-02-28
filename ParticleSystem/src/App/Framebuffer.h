#pragma once

#include <iostream>

#include <glad/glad.h>

class Framebuffer {
public:
	Framebuffer(uint32_t width, uint32_t heigth);
	~Framebuffer() = default;
	void CreateFramebuffer();
	void Bind();
	void Unbind();
	void Resize(uint32_t width, uint32_t height);

	unsigned int GetTextureID() { return m_TextureID; }
	uint32_t GetWidth() { return m_Width; }
	uint32_t GetHeight() { return m_Height; }

private:
	uint32_t m_Width, m_Height;

	unsigned int m_TextureID;
	unsigned int FBO;
	unsigned int RBO;
};