#include <string>

class Shader {
public:
  Shader();
  ~Shader();
  void loadShaders();
  unsigned int getShaderProgram() { return currentShaderProgram; }
  int addUBO(const std::string &name);

private:
  std::string readFile(const char *path);
  unsigned int createShaderProgram(const char *vertexSource,
                                   const char *fragmentSource);
  unsigned int compileShader(unsigned int type, const char *source);

  std::string vertexSource;
  std::string fragmentSource;
  unsigned int currentShaderProgram;
  // lastVertexModTime, lastFragmentModTime;
};
