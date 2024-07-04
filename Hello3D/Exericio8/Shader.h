#pragma once

#include <string>

#include <glad/glad.h>

class Shader {
private:
	GLuint id;

public:
	Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath);

	GLuint getId() const { return id; }
};