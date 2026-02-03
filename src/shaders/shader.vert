#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FaceNormal;
out vec3 FragPos;
out vec2 TexCoord;

vec3 lightPos = vec3(0.0, 400.0, 1.0);

void main()
{
  // world space of the object
  FragPos = vec3(model * vec4(aPos, 1.0));

  // Transform normal to world space
  FaceNormal = mat3(transpose(inverse(model))) * aNormal;
  TexCoord = aTexCoord;

  gl_Position = projection * view * vec4(FragPos, 1.0);
}
