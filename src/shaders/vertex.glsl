#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 model; // The object's position (World Space)
uniform mat4 view; // The camera's position (View Space)
uniform mat4 projection; // The perspective lens (Clip Space)

void main()
{
  // The Order Matters: Projection * View * Model * Position
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
