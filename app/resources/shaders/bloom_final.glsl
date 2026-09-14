//#shader vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;
void main() { TexCoords = aTexCoords; gl_Position = vec4(aPos, 0.0, 1.0); }

//#shader fragment
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform float exposure;
void main() {
    vec3 hdr = texture(scene, TexCoords).rgb;
    if (bloom) hdr += texture(bloomBlur, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdr * exposure);
    mapped = pow(mapped, vec3(1.0 / 2.2));
    FragColor = vec4(mapped, 1.0);
}