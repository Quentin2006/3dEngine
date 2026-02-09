#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 FaceNormal;

uniform sampler2D ourTexture;

// Maximum number of lights
#define MAX_LIGHTS 8

// Light structure - must match C++ LightData with std140 alignment
// In std140, vec3 has 16-byte alignment (padded)
struct Light {
  vec3 position;
  vec3 color;
};

// Uniform block for lights using std140 layout
// Binding point is set via glUniformBlockBinding() in C++ (OpenGL 3.3 compatible)
layout(std140) uniform LightBlock {
  Light lights[MAX_LIGHTS]; // Array of structs to match C++
  int count;
} lightBlock;

void main()
{
  // get the texture color
  vec4 texColor = texture(ourTexture, TexCoord);

  vec3 finalColor = vec3(0.0);

  // Iterate through all active lights
  for (int i = 0; i < lightBlock.count; i++) {
    // Ambient light
    float ambientStrength = 0.1;
    vec3 ambientColor = ambientStrength * lightBlock.lights[i].color;

    // Diffuse light
    vec3 lightDir = normalize(lightBlock.lights[i].position - FragPos);
    vec3 norm = normalize(FaceNormal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseColor = diff * lightBlock.lights[i].color;

    // Add contribution from this light
    finalColor += (ambientColor + diffuseColor) * texColor.rgb;
  }

  // If no lights, just show texture
  if (lightBlock.count == 0) {
    finalColor = texColor.rgb;
  }

  FragColor = vec4(finalColor, 1.0);
}
