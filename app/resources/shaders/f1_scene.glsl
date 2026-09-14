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
in VS_OUT { vec3 fragPos; vec3 normal; vec2 texCoords; } fs_in;
uniform vec3 viewPos, materialColor;
uniform float emissive;
uniform int surfaceMode;
uniform sampler2D asphaltTexture;
uniform sampler2D texture_diffuse1;
uniform sampler2D streetLightsTexture;
uniform vec3 pointPosition, pointColor;
uniform float pointIntensity;
uniform vec3 spotPosition, spotDirection, spotColor;
uniform float spotCutoff;
uniform samplerCube depthMap;
uniform float farPlane;

float pointShadow(vec3 normal) {
    vec3 fromLight = fs_in.fragPos - pointPosition;
    float currentDepth = length(fromLight);
    vec3 samples[8] = vec3[](vec3(1, 1, 1), vec3(-1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1),
    vec3(1, 1, -1), vec3(-1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1));
    float shadow = 0.0;
    float bias = max(0.06 * (1.0 - dot(normal, normalize(pointPosition - fs_in.fragPos))), 0.015);
    float diskRadius = 0.12 + (currentDepth / farPlane) * 0.18;
    for (int i = 0; i < 8; ++i) {
        float closestDepth = texture(depthMap, fromLight + samples[i] * diskRadius).r * farPlane;
        shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
    }
    return shadow / 8.0;
}
vec3 asphaltAlbedo(vec2 uv) {
    // Use the road strip once across the track width; the source image's outer margins are not repeated.
    float acrossTrack = mix(0.10, 0.60, uv.y);

    // Mirror every second lengthwise tile, making the image coordinates continuous at each join.
    float alongTrack = 1.0 - abs(fract(uv.x * 3.0) * 2.0 - 1.0);
    return texture(asphaltTexture, vec2(acrossTrack, alongTrack)).rgb;
}
vec3 blinnPhong(vec3 lightDirection, vec3 lightColor, float strength, float shadow) {
    vec3 norm = normalize(fs_in.normal);
    float diffuse = max(dot(norm, lightDirection), 0.0);
    vec3 halfway = normalize(lightDirection + normalize(viewPos - fs_in.fragPos));
    float specular = pow(max(dot(norm, halfway), 0.0), 48.0);
    vec3 albedo = materialColor;
    if (surfaceMode == 1) albedo = texture(asphaltTexture, fs_in.texCoords * vec2(15.0, 3.0)).rgb;
    if (surfaceMode == 1) albedo = asphaltAlbedo(fs_in.texCoords);
    if (surfaceMode == 2) albedo = texture(texture_diffuse1, fs_in.texCoords).rgb;
    if (surfaceMode == 3) albedo = texture(streetLightsTexture, fs_in.texCoords).rgb;
    return (1.0 - shadow) * strength * (diffuse * albedo + specular * vec3(0.35)) * lightColor;
}
void main() {
    vec3 norm = normalize(fs_in.normal);
    vec3 albedo = materialColor;
    if (surfaceMode == 1) albedo = texture(asphaltTexture, fs_in.texCoords * vec2(15.0, 3.0)).rgb;
    if (surfaceMode == 1) albedo = asphaltAlbedo(fs_in.texCoords);
    if (surfaceMode == 2) albedo = texture(texture_diffuse1, fs_in.texCoords).rgb;
    if (surfaceMode == 3) albedo = texture(streetLightsTexture, fs_in.texCoords).rgb;
    vec3 result = albedo * 0.055;
    vec3 toPoint = pointPosition - fs_in.fragPos;
    float distanceToPoint = length(toPoint);
    float attenuation = 1.0 / (1.0 + 0.09 * distanceToPoint + 0.032 * distanceToPoint * distanceToPoint);
    result += blinnPhong(normalize(toPoint), pointColor, pointIntensity * attenuation, pointShadow(norm));
    vec3 toSpot = spotPosition - fs_in.fragPos;
    float spotDistance = length(toSpot);
    float cone = dot(normalize(spotDirection), normalize(fs_in.fragPos - spotPosition));
    float spot = smoothstep(spotCutoff - 0.045, spotCutoff, cone);
    float spotAttenuation = 1.0 / (1.0 + 0.11 * spotDistance + 0.035 * spotDistance * spotDistance);
    result += blinnPhong(normalize(toSpot), spotColor, 3.2 * spot * spotAttenuation, 0.0);
    result += albedo * emissive;
    FragColor = vec4(result, 1.0);
    BrightColor = vec4(max(result - vec3(0.85), vec3(0.0)), 1.0);
}