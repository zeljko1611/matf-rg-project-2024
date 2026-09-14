//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
void main() { gl_Position = model * vec4(aPos, 1.0); }

//#shader geometry
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;
uniform mat4 shadowMatrices[6];
out vec4 fragPos;
void main() {
    for (int face = 0; face < 6; ++face) {
        gl_Layer = face;
        for (int vertex = 0; vertex < 3; ++vertex) {
            fragPos = gl_in[vertex].gl_Position;
            gl_Position = shadowMatrices[face] * fragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}

//#shader fragment
#version 330 core
in vec4 fragPos;
uniform vec3 lightPos;
uniform float farPlane;
void main() { gl_FragDepth = length(fragPos.xyz - lightPos) / farPlane; }
