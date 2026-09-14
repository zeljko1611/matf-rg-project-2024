//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
out VS_OUT { vec3 fragPos; vec3 normal; vec2 texCoords; } vs_out;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
void main() {
    vs_out.fragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.texCoords = aTexCoords;
    gl_Position = projection * view * vec4(vs_out.fragPos, 1.0);
}

//#shader fragment
#version 330 core

out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
} fs_in;

uniform vec3 viewPos, materialColor;
uniform float emissive;
uniform int surfaceMode;

uniform sampler2D asphaltTexture;
uniform sampler2D texture_diffuse1;
uniform sampler2D streetLightsTexture;

vec3 asphaltAlbedo(vec2 uv) {
    // Use the road strip once across the track width; the source image's outer margins are not repeated.
    float acrossTrack = mix(0.10, 0.60, uv.y);

    // Mirror every second lengthwise tile, making the image coordinates continuous at each join.
    float alongTrack = 1.0 - abs(fract(uv.x * 3.0) * 2.0 - 1.0);
    return texture(asphaltTexture, vec2(acrossTrack, alongTrack)).rgb;
}

void main() {
    vec3 norm = normalize(fs_in.normal);
    vec3 albedo = materialColor;

    if (surfaceMode == 1)
    albedo = texture(asphaltTexture, fs_in.texCoords * vec2(15.0, 3.0)).rgb;

    if (surfaceMode == 1)
    albedo = asphaltAlbedo(fs_in.texCoords);

    if (surfaceMode == 2)
    albedo = texture(texture_diffuse1, fs_in.texCoords).rgb;

    if (surfaceMode == 3)
    albedo = texture(streetLightsTexture, fs_in.texCoords).rgb;
}