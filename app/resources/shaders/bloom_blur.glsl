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
uniform sampler2D image;
uniform bool horizontal;
void main() {
    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    vec2 offset = 1.0 / textureSize(image, 0);
    vec3 color = texture(image, TexCoords).rgb * weights[0];
    for (int i = 1; i < 5; ++i) {
        vec2 direction = horizontal ? vec2(offset.x * i, 0.0) : vec2(0.0, offset.y * i);
        color += texture(image, TexCoords + direction).rgb * weights[i];
        color += texture(image, TexCoords - direction).rgb * weights[i];
    }
    FragColor = vec4(color, 1.0);
}