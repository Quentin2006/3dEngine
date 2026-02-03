#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 FaceNormal;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform sampler2D ourTexture;

uniform sampler2D texture1;

void main()
{
  // get the texture color
  vec4 texColor = texture(texture1, TexCoord);

  // clacuate the ambient light
  float ambientStrength = 0.2;
  vec3 ambientColor = ambientStrength * lightColor;

  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(FaceNormal, lightDir), 0.0);
  vec3 diffuseColor = diff * lightColor;

  // --- 3. Combine ---
  // Add them together BEFORE multiplying by the object's color
  vec3 result = (ambientColor + diffuseColor) * texColor.rgb;

  FragColor = vec4(result, 1.0);
}
