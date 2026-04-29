#version 330 core

#define MAX_LIGHTS 16

in vec3 fragWorldPosition;
in vec3 fragWorldNormal;
in vec3 fragTangent;
in vec3 fragColor;

struct MaterialUniforms {
  vec3 specularColor;
  float shininess;
};

struct CameraUniforms {
  mat4 viewMatrix;
  mat4 projectionMatrix;
  vec3 viewPosition;
  vec3 focalPoint;
  float focalSize;
};

struct LightUniforms {
  int type;
  vec3 position;
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float constant;
  float linear;
  float quadratic;
};

uniform MaterialUniforms material;
uniform CameraUniforms camera;
uniform LightUniforms lights[MAX_LIGHTS];
uniform int lightCount;

out vec4 FragColor;

vec3 ComputeLight(LightUniforms light,
                  vec3 normal,
                  vec3 fragmentPosition,
                  vec3 viewDirection,
                  vec3 diffuseAlbedo,
                  vec3 specularAlbedo)
{
  vec3 lightDirection = normalize(-light.direction);
  float attenuation = 1.0;

  if (light.type == 0)
  {
    vec3 lightToFragment = light.position - fragmentPosition;
    float distanceToLight = length(lightToFragment);
    lightDirection = normalize(lightToFragment);
    attenuation = 1.0 / (light.constant
      + light.linear * distanceToLight
      + light.quadratic * distanceToLight * distanceToLight);
  }

  float diffuseStrength = max(dot(normal, lightDirection), 0.0);
  vec3 reflectDirection = reflect(-lightDirection, normal);
  float specularStrength = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);

  vec3 ambientTerm = light.ambient * diffuseAlbedo;
  vec3 diffuseTerm = light.diffuse * diffuseStrength * diffuseAlbedo;
  vec3 specularTerm = light.specular * specularStrength * specularAlbedo;
  return (ambientTerm + diffuseTerm + specularTerm) * attenuation;
}

void main()
{
    // Surface normal (radial direction) for proper cylindrical shading
    vec3 N = normalize(fragWorldNormal);
    vec3 V = normalize(camera.viewPosition - fragWorldPosition);

    // Base color is tangent-aligned (same for entire radial segment)
    vec3 baseColor = fragColor;
    vec3 specularAlbedo = material.specularColor;

    vec3 color = vec3(0.0);
    for (int i = 0; i < lightCount; ++i)
    {
      color += ComputeLight(lights[i], N, fragWorldPosition, V, baseColor, specularAlbedo);
    }

    FragColor = vec4(color, 1.0);
}