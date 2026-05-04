#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ui/widgets/inspect_fields/InspectProvider.h"
#include "ui/widgets/inspect_fields/InspectVec3FieldWidget.h"

class Transform : public InspectProvider
{
public:
  Transform() = default;

  Transform(const glm::vec3& positionValue,
            const glm::vec3& orientationValue,
            const glm::vec3& scaleValue)
    : position(positionValue),
      orientation(orientationValue),
      scale(scaleValue)
  {
  }

  const glm::vec3& GetPosition() const { return position; }
  const glm::vec3& GetOrientation() const { return orientation; }
  const glm::vec3& GetScale() const { return scale; }

  const glm::vec3& GetRotation() const { return orientation; }

  void SetPosition(const glm::vec3& value) { position = value; }
  void SetOrientation(const glm::vec3& value) { orientation = value; }
  void SetScale(const glm::vec3& value) { scale = value; }

  void SetRotation(const glm::vec3& value) { orientation = value; }

  glm::mat4 GetModelMatrix() const
  {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, orientation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, orientation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);
    return model;
  }

  std::string GetInspectDisplayName() const override { return "Transform"; }

  std::vector<std::shared_ptr<IInspectWidget>> GetInspectFields() override
  {
    auto positionField = std::make_shared<InspectVec3FieldWidget>("position", "Position", "Transform");
    positionField->SetValue(QVariantList{position.x, position.y, position.z});
    positionField->valueChangedCallback = [this](const QVariant& value)
    {
      const QVariantList list = value.toList();
      if (list.size() >= 3)
      {
        position = glm::vec3(static_cast<float>(list[0].toDouble()),
                             static_cast<float>(list[1].toDouble()),
                             static_cast<float>(list[2].toDouble()));
      }
    };

    auto orientationField = std::make_shared<InspectVec3FieldWidget>("orientation", "Orientation", "Transform");
    orientationField->SetValue(QVariantList{orientation.x, orientation.y, orientation.z});
    orientationField->valueChangedCallback = [this](const QVariant& value)
    {
      const QVariantList list = value.toList();
      if (list.size() >= 3)
      {
        orientation = glm::vec3(static_cast<float>(list[0].toDouble()),
                                static_cast<float>(list[1].toDouble()),
                                static_cast<float>(list[2].toDouble()));
      }
    };

    auto scaleField = std::make_shared<InspectVec3FieldWidget>("scale", "Scale", "Transform");
    scaleField->SetValue(QVariantList{scale.x, scale.y, scale.z});
    scaleField->valueChangedCallback = [this](const QVariant& value)
    {
      const QVariantList list = value.toList();
      if (list.size() >= 3)
      {
        scale = glm::vec3(static_cast<float>(list[0].toDouble()),
                          static_cast<float>(list[1].toDouble()),
                          static_cast<float>(list[2].toDouble()));
      }
    };

    return {positionField, orientationField, scaleField};
  }

  bool HasVisibility() const override { return false; }
  bool IsVisible() const override { return true; }

  std::optional<float> CastRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const override
  {
    (void)rayOrigin;
    (void)rayDirection;
    return std::nullopt;
  }

private:
  glm::vec3 position{0.0f, 0.0f, 0.0f};
  glm::vec3 orientation{0.0f, 0.0f, 0.0f};
  glm::vec3 scale{1.0f, 1.0f, 1.0f};
};
