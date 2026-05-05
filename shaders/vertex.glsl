#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 aTexCoord;

struct CameraUniforms {
  mat4 viewMatrix;
  mat4 projectionMatrix;
  vec3 viewPosition;
  vec3 focalPoint;
  float focalSize;
};

struct GameObjectUniforms {
  mat4 modelMatrix;
};

uniform CameraUniforms camera;
uniform GameObjectUniforms gameObject;

out vec2 fragTexCoord;
out vec3 fragWorldPosition;
out vec3 fragWorldNormal;
out vec3 fragWorldTangent;
out vec3 fragWorldBitangent;

/**
 * @brief Compute an arbitrary perpendicular vector to a given vector.
 * Used to build TBN matrix when explicit tangent data is unavailable.
 */
vec3 ComputePerpendicularVector(vec3 v)
{
  // Choose the perpendicular vector that is least parallel to v
  if (abs(v.x) < 0.9)
    return cross(v, vec3(1.0, 0.0, 0.0));
  else
    return cross(v, vec3(0.0, 1.0, 0.0));
}

void main()
{
  vec4 worldPosition = gameObject.modelMatrix * vec4(aPos, 1.0);
  mat3 normalMatrix = transpose(inverse(mat3(gameObject.modelMatrix)));

  gl_Position = camera.projectionMatrix
              * camera.viewMatrix
              * worldPosition;

  fragTexCoord = aTexCoord;
  fragWorldPosition = vec3(worldPosition);
  fragWorldNormal = normalize(normalMatrix * normal);

  // Compute tangent and bitangent from normal (for TBN matrix in normal mapping)
  // Note: This is an approximation. Ideally, meshes should provide explicit tangent data.
  vec3 tangent = normalize(ComputePerpendicularVector(fragWorldNormal));
  vec3 bitangent = cross(fragWorldNormal, tangent);

  fragWorldTangent = tangent;
  fragWorldBitangent = bitangent;
}