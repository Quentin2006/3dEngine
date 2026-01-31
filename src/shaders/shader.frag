#version 330 core
out vec4 FragColor;

in vec3 vPos;       // Position from vertex shader
uniform float time; // Don't forget to send this from C++!

void main()
{
    // 1. Define two nice colors
    vec3 pink = vec3(1.0, 0.0, 0.8);
    vec3 cyan = vec3(0.0, 1.0, 1.0);

    // 2. Calculate a "Mix Factor" (0.0 to 1.0)
    // We take the Y position (height), add time to make it scroll, 
    // and wrap it in sin() to make it wave back and forth.
    float factor = sin(vPos.y * 2.0 + time) * 0.5 + 0.5;

    // 3. Mix them!
    vec3 finalColor = mix(pink, cyan, factor);

    FragColor = vec4(finalColor, 1.0);
}
