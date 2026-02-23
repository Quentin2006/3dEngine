#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 FaceNormal;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform float shininess = 32.0;
uniform vec3 cameraPos;

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
  vec4 diffuseColor = texture(diffuseTexture, TexCoord);
  vec4 specColor = texture(specularTexture, TexCoord);
  
  vec3 norm = normalize(FaceNormal);
  vec3 viewDir = normalize(cameraPos - FragPos);
  
  vec3 ambient = diffuseColor.rgb * 0.15;
  vec3 totalLight = vec3(0.0);
  
  for (int i = 0; i < lightBlock.count; i++) {
    vec3 lightDir = normalize(lightBlock.lights[i].position - FragPos);
    float dist = length(lightBlock.lights[i].position - FragPos);
    float atten = 1.0 / (1.0 + 0.01 * dist * dist);
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightBlock.lights[i].color * diffuseColor.rgb;
    
    // Blinn-Phong specular
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), shininess);
    vec3 specular = spec * specColor.rgb * lightBlock.lights[i].color;
    
    totalLight += (diffuse + specular) * atten * lightBlock.lights[i].intensity;
  }
  
  vec3 finalColor = ambient + totalLight;
  
  if (lightBlock.count == 0) {
    finalColor = diffuseColor.rgb;
  }
  
  FragColor = vec4(min(finalColor, vec3(1.0)), 1.0);
}
