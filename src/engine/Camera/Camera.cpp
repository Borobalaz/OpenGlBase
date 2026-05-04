#include "Camera/Camera.h"

#include <algorithm>
#include <string>

#include "Shader.h"
#include "ui/widgets/inspect_fields/InspectNumberFieldWidget.h"
#include "ui/widgets/inspect_fields/InspectTextFieldWidget.h"
#include "ui/widgets/inspect_fields/InspectVec3FieldWidget.h"

/**
 * @brief Construct a new Camera:: Camera object
 * 
 */
Camera::Camera()
  : transform(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
    front(0.0f, 0.0f, -1.0f),
    up(0.0f, 1.0f, 0.0f),
    aspect(4.0f / 3.0f),
    focalDistance(1.0f),
    focalSize(1.0f)
{
}

/**
 * @brief Destroy the Camera:: Camera object
 * 
 */
Camera::~Camera()
{
}

/**
 * @brief Apply the camera's view and projection matrices to the shader
 * 
 * @param shader 
 */
void Camera::Apply(Shader& shader) const
{
  const std::string viewUniformName =
    ComposeUniformName("camera", "viewMatrix");
  const std::string projectionUniformName =
    ComposeUniformName("camera", "projectionMatrix");
  const std::string viewPositionUniformName =
    ComposeUniformName("camera", "viewPosition");
  const std::string focalPointUniformName =
    ComposeUniformName("camera", "focalPoint");
  const std::string focalSizeUniformName =
    ComposeUniformName("camera", "focalSize");

  if (shader.HasUniform(viewUniformName))
  {
    shader.SetMat4(viewUniformName, GetViewMatrix());
  }

  if (shader.HasUniform(projectionUniformName))
  {
    shader.SetMat4(projectionUniformName, GetProjectionMatrix());
  }

  const glm::vec3 forward = GetForwardVector();

  if (shader.HasUniform(viewPositionUniformName))
  {
    shader.SetVec3(viewPositionUniformName, GetPosition());
  }

  if (shader.HasUniform(focalPointUniformName))
  {
    shader.SetVec3(focalPointUniformName, GetPosition() + forward * focalDistance);
  }

  if (shader.HasUniform(focalSizeUniformName))
  {
    shader.SetFloat(focalSizeUniformName, focalSize);
  }
}

std::string Camera::GetInspectDisplayName() const
{
  return "camera";
}

std::vector<std::shared_ptr<IInspectWidget>> Camera::GetInspectFields()
{
  std::vector<std::shared_ptr<IInspectWidget>> fields = transform.GetInspectFields();

  auto aspectField = std::make_shared<InspectNumberFieldWidget>(
    "aspect",
    "Aspect",
    "Projection",
    [this]() -> double { return static_cast<double>(aspect); },
    [this](double value)
    {
      SetAspect(static_cast<float>(value));
    },
    0.01,
    100.0,
    0.01
  );

  auto focalDistanceField = std::make_shared<InspectNumberFieldWidget>(
    "focalDistance",
    "Focal Distance",
    "Focus",
    [this]() -> double { return static_cast<double>(focalDistance); },
    [this](double value)
    {
      SetFocalDistance(static_cast<float>(value));
    },
    0.01,
    1000.0,
    0.1
  );

  auto focalSizeField = std::make_shared<InspectNumberFieldWidget>(
    "focalSize",
    "Focal Size",
    "Focus",
    [this]() -> double { return static_cast<double>(focalSize); },
    [this](double value)
    {
      SetFocalSize(static_cast<float>(value));
    },
    0.01,
    1000.0,
    0.1
  );

  auto movementComponentField = std::make_shared<InspectTextFieldWidget>(
    "moveComponent",
    "Move Component",
    "Runtime",
    true
  );
  movementComponentField->SetValue(moveComponent ? "Attached" : "None");

  fields.push_back(aspectField);
  fields.push_back(focalDistanceField);
  fields.push_back(focalSizeField);
  fields.push_back(movementComponentField);
  return fields;
}

/**
 * @brief Set the camera's position
 * 
 * @param pos 
 */
void Camera::SetPosition(const glm::vec3& pos)
{
  transform.SetPosition(pos);
  NotifyMovementCameraStateChanged();
}

/**
 * @brief Get the camera's current position
 * 
 * @return glm::vec3 
 */
glm::vec3 Camera::GetPosition() const
{
  return transform.GetPosition();
}

void Camera::SetAspect(float newAspect)
{
  if (newAspect > 0.0f)
  {
    aspect = newAspect;
  }
}

float Camera::GetAspect() const
{
  return aspect;
}

void Camera::SetFocalDistance(float newFocalDistance)
{
  focalDistance = std::max(0.01f, newFocalDistance);
}

float Camera::GetFocalDistance() const
{
  return focalDistance;
}

void Camera::SetFocalSize(float newFocalSize)
{
  focalSize = std::max(0.01f, newFocalSize);
}

float Camera::GetFocalSize() const
{
  return focalSize;
}

/**
 * @brief Set the camera movement component
 * 
 * @param component 
 */
void Camera::SetMoveComponent(std::unique_ptr<BaseMovement> component)
{
  moveComponent = std::move(component);
}

/**
 * @brief Apply the camera movement component
 * 
 * @param deltaTime 
 */
void Camera::Move(float deltaTime)
{
  if (moveComponent)
  {
    front = GetForwardVector();
    glm::vec3 position = transform.GetPosition();
    moveComponent->Update(deltaTime, position, front, up, focalDistance, focalSize);
    transform.SetPosition(position);
    transform.SetOrientation(front);
  }
}

void Camera::NotifyMovementCameraStateChanged()
{
  if (moveComponent)
  {
    moveComponent->OnCameraStateChanged(transform.GetPosition(), front, up);
  }
}

glm::vec3 Camera::GetForwardVector() const
{
  const glm::vec3 orientation = transform.GetOrientation();
  if (glm::length(orientation) > 1e-5f)
  {
    return glm::normalize(orientation);
  }

  return front;
}