#version 330 core

in vec3 fragWorldPosition;
in vec3 fragWorldDirection;

struct MaterialUniforms {
  sampler2D diffuseTexture;
  sampler2D specularTexture;
  vec3 ambientColor;
  vec3 diffuseColor;
  vec3 specularColor;
  float shininess;
  bool hasDiffuseTexture;
  bool hasSpecularTexture;
};

struct CameraUniforms {
  mat4 viewMatrix;
  mat4 projectionMatrix;
  vec3 viewPosition;
  vec3 focalPoint;
  float focalSize;
};

uniform MaterialUniforms material;
uniform CameraUniforms camera;

out vec4 FragColor;

void main()
{
  vec3 axisColor = abs(normalize(fragWorldDirection));
  vec3 orientedColor = axisColor * material.diffuseColor;

  float cameraDistance = length(camera.viewPosition - fragWorldPosition);
  float distanceDarkening = clamp(1.0 - 0.18 * cameraDistance, 0.35, 1.0);

  float focalDistance = length(camera.focalPoint - fragWorldPosition);
  float focalRange = max(camera.focalSize, 0.01);
  float focalAlphaWeight = clamp(1.0 / (1.0 + (focalDistance / focalRange)), 0.12, 1.0);

  FragColor = vec4(orientedColor * distanceDarkening, focalAlphaWeight);
}
