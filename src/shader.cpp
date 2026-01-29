#include "shader.h"
#include "../include/glad/glad.h"
#include <GL/glext.h>
#include <fstream>
#include <iostream>
#include <sstream>

Shader::Shader() {}
Shader::~Shader() { glDeleteProgram(currentShaderProgram); }

void Shader::loadShaders() {
  vertexSource = readFile("src/shaders/vertex.glsl");
  fragmentSource = readFile("src/shaders/fragment.glsl");

  currentShaderProgram =
      createShaderProgram(vertexSource.c_str(), fragmentSource.c_str());
}

std::string Shader::readFile(const char *path) {
  std::ifstream file(path);
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

unsigned int Shader::createShaderProgram(const char *vertexSource,
                                         const char *fragmentSource) {
  unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
  unsigned int fragmentShader =
      compileShader(GL_FRAGMENT_SHADER, fragmentSource);

  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  int success;
  char infoLog[512];
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    std::cout << "Shader program linking failed: " << infoLog << std::endl;
    return 0;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

unsigned int Shader::compileShader(unsigned int type, const char *source) {
  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "Shader compilation failed: " << infoLog << std::endl;
    return 0;
  }
  return shader;
}
