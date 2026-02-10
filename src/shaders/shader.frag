#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 FaceNormal;

uniform sampler2D ourTexture;

// we need this to be avalable compile time as we need to
// set the lize of the lights array in LightBlock
#define MAX_LIGHTS 128

struct Light {
  vec3 position;
  vec3 color;
  float intensity;
};

layout(std140) uniform LightBlock {
  Light lights[MAX_LIGHTS];
  int count;
} lightBlock;

void main()
{
  // get the texture color
  vec4 texColor = texture(ourTexture, TexCoord);

  vec3 finalColor = vec3(0.0);

  // Iterate through all active lights
  for (int i = 0; i < lightBlock.count; i++) {
    // Reduced ambient to balance with brighter lights
    float ambientStrength = 0.05;

    // Calculate light direction and distance once, reuse
    vec3 toLight = lightBlock.lights[i].position - FragPos;
    float dist = length(toLight);
    vec3 lightDir = normalize(toLight);

    // Inverse square law with bias to prevent division by zero
    float attenuation = 1.0 / (dist * dist + 0.01);

    // Apply attenuation to ambient light for physical accuracy
    vec3 ambientColor = ambientStrength * lightBlock.lights[i].color * attenuation;

    // Diffuse light with attenuation
    vec3 norm = normalize(FaceNormal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseColor = diff * lightBlock.lights[i].color * attenuation;

    // Add contribution from this light with global intensity multiplier
    finalColor += (ambientColor + diffuseColor) * lightBlock.lights[i].intensity * texColor.rgb;
  }

  // If no lights, just show texture
  if (lightBlock.count == 0) {
    finalColor = texColor.rgb;
  }

  // Clamp to prevent blowout from multiple bright lights
  finalColor = min(finalColor, vec3(1.0));

  FragColor = vec4(finalColor, 1.0);
}
