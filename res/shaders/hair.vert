
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNext;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Tangent;
out vec3 FragPos;

void main(){
  gl_Position = projection * view * model * vec4(aPos, 1.0f);
  Tangent = normalize(aPos-aNext);
  FragPos = aPos;
}
