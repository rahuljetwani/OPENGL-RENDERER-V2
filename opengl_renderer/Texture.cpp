#include"Texture.h"
#include<iostream>

// stb flips rows so that (0,0) is the bottom-left corner, matching OpenGL / Assimp UVs.
// If a model ever shows up upside-down, this is the one switch to check (see model.cpp: FLIP_UVS).
static const bool FLIP_IMAGES_ON_LOAD = true;

Texture::Texture(const char* file, const char* texType, bool srgb)
{
	type = texType;

	int w, h, ch;
	stbi_set_flip_vertically_on_load(FLIP_IMAGES_ON_LOAD);
	unsigned char* bytes = stbi_load(file, &w, &h, &ch, 0);
	if (!bytes)
	{
		std::cout << "[Texture] FAILED to load '" << file << "': " << stbi_failure_reason() << "\n";
		// 1x1 magenta so a missing texture is obvious in the viewport instead of a black mesh
		unsigned char magenta[3] = { 255, 0, 255 };
		upload(magenta, 1, 1, 3, false);
		return;
	}
	upload(bytes, w, h, ch, srgb);
	stbi_image_free(bytes);
}

Texture Texture::FromEncodedMemory(const unsigned char* bytes, int length, const char* texType, bool srgb)
{
	Texture t;
	t.type = texType;

	int w, h, ch;
	stbi_set_flip_vertically_on_load(FLIP_IMAGES_ON_LOAD);
	unsigned char* pixels = stbi_load_from_memory(bytes, length, &w, &h, &ch, 0);
	if (!pixels)
	{
		std::cout << "[Texture] FAILED to decode embedded texture: " << stbi_failure_reason() << "\n";
		unsigned char magenta[3] = { 255, 0, 255 };
		t.upload(magenta, 1, 1, 3, false);
		return t;
	}
	t.upload(pixels, w, h, ch, srgb);
	stbi_image_free(pixels);
	return t;
}

Texture Texture::FromPixels(const unsigned char* pixels, int width, int height, int channels, const char* texType, bool srgb)
{
	Texture t;
	t.type = texType;
	t.upload(pixels, width, height, channels, srgb);
	return t;
}

void Texture::upload(const unsigned char* pixels, int w, int h, int channels, bool srgb)
{
	GLenum format, internalFormat;
	switch (channels)
	{
	case 1: format = GL_RED;  internalFormat = GL_R8;   break;	// no sRGB single-channel format exists
	case 2: format = GL_RG;   internalFormat = GL_RG8;  break;
	case 3: format = GL_RGB;  internalFormat = srgb ? GL_SRGB8        : GL_RGB8;  break;
	case 4: format = GL_RGBA; internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8; break;
	default:
		throw std::invalid_argument("Texture: unsupported channel count");
	}

	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	// Rows of 1- and 3-channel images are not 4-byte aligned; without this, widths that
	// are not a multiple of 4 come out sheared.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, pixels);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// A grayscale image should read as (g,g,g,1) in the shader, not (g,0,0,1)
	if (channels == 1)
	{
		GLint swizzle[4] = { GL_RED, GL_RED, GL_RED, GL_ONE };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind(GLuint unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
	ID = 0;
}
