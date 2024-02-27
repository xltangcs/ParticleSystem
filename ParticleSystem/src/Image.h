#pragma once

#include <string>

#include <glad/glad.h>

class Image
{
public:
	Image() {};
	Image(uint32_t width, uint32_t height);
	Image(const std::string& path);
	~Image();

	uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }
	uint32_t GetTextureID() const { return m_TextureID; }

	void SetData(void* data, uint32_t size);

private:
	uint32_t m_Width = 0, m_Height = 0;
	uint32_t m_TextureID = 0;
	GLenum m_DataFormat = 0;
};
