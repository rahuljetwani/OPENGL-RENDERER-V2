#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include<glad/glad.h>
#include<stb/stb_image.h>
#include<string>

#include"shaderClass.h"

// A 2D texture living on the GPU.
//
// The texture does NOT remember which texture unit it belongs to. The mesh decides
// that at draw time (Bind(unit)), so the same Texture can be shared by many meshes.
//
// sRGB rule of thumb:  colour data (diffuse / base colour)  -> srgb = true
//                      data maps (specular, roughness, normals) -> srgb = false
class Texture
{
public:
	GLuint ID = 0;
	std::string type;		// "diffuse", "specular", ...

	// Load from an image file on disk (png, jpg, tga, bmp...)
	Texture(const char* file, const char* texType, bool srgb);

	// Decode an image that is still encoded in memory (e.g. png bytes embedded in a .glb)
	static Texture FromEncodedMemory(const unsigned char* bytes, int length, const char* texType, bool srgb);

	// Upload raw 8-bit pixels (channels = 1, 2, 3 or 4)
	static Texture FromPixels(const unsigned char* pixels, int width, int height, int channels, const char* texType, bool srgb);

	void Bind(GLuint unit) const;
	void Unbind() const;
	void Delete();

private:
	Texture() = default;
	void upload(const unsigned char* pixels, int width, int height, int channels, bool srgb);
};
#endif
