
#version 330 core

struct Material{
  vec3 Ka;
  vec3 Kd;
  vec3 Ks;
  float shininess;
};

struct Light{
  vec3 position;
};

in vec3 Tangent;
in vec3 FragPos;

uniform vec3 viewPos;
//uniform Material material;
uniform Light light;

uniform bool isOpenLighting;

out vec4 color;

void main()
{
    // default
    vec3 dcolor = vec3(0.8f, 0.0f, 0.0f);
    Material material;
    material.Ka = vec3(0.05f,0.05f,0.05f);
    material.Kd = vec3(0.2f,0.2f,0.2f);
    material.Ks = vec3(0.7f, 0.7f, 0.7f);
    material.shininess = 15.0f;

    //Ambient
    vec3 ambient = material.Ka * dcolor;

    if(isOpenLighting){
      //Diffuse
      vec3 lightDir = normalize(light.position - FragPos);
      float dotTL = dot(Tangent, lightDir);
      float kajiyaTL = sqrt(1.0f - dotTL*dotTL);
      vec3 diffuse = material.Kd * kajiyaTL * dcolor;

      //specular
      vec3 viewDir = normalize(viewPos - FragPos);
      vec3 H = normalize(lightDir + viewDir);
      float dotTH = dot(Tangent,H);
      float kajiyaTH = sqrt(1.0f - dotTH*dotTH);
      float dirAtten = smoothstep(-1.0f,0.0f,dotTH);
      float spec = dirAtten * pow(kajiyaTH, material.shininess);
      vec3 specular = material.Ks * spec * dcolor;
      vec3 result = (ambient + diffuse + specular);
      color = vec4(result, 1.0f);
    }else{
      color = vec4(ambient, 1.0f);
    }
//    color = vec4(0.8f, 0.0f, 0.0f,1.0f);
}
