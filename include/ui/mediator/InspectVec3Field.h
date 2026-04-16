#pragma once

#include <functional>

#include <glm/glm.hpp>

#include "ui/mediator/InspectField.h"

class InspectVec3Field : public InspectField
{
  Q_OBJECT

public:
  using Getter = std::function<glm::vec3()>;
  using Setter = std::function<void(const glm::vec3&)>;

  InspectVec3Field(QString fieldId,
                   QString displayName,
                   QString groupName,
                   Getter getter,
                   Setter setter,
                   bool readOnly = false,
                   QObject *parent = nullptr)
    : InspectField(std::move(fieldId), std::move(displayName), std::move(groupName), readOnly, parent),
      getter(std::move(getter)),
      setter(std::move(setter))
  {
  }

  QUrl editorUrl() const override { return QUrl(QStringLiteral("qrc:/qml/InspectVec3Field.qml")); }

  QVariant value() const override
  {
    const glm::vec3 current = getter ? getter() : glm::vec3(0.0f);
    QVariantList list;
    list.push_back(current.x);
    list.push_back(current.y);
    list.push_back(current.z);
    return list;
  }

  bool trySetValue(const QVariant &input) override
  {
    if (!setter)
    {
      return false;
    }

    if (input.canConvert<QVariantList>())
    {
      const QVariantList list = input.toList();
      if (list.size() >= 3)
      {
        setter(glm::vec3(list[0].toFloat(), list[1].toFloat(), list[2].toFloat()));
        return true;
      }
    }

    const QStringList parts = input.toString().split(',', Qt::SkipEmptyParts);
    if (parts.size() == 3)
    {
      setter(glm::vec3(parts[0].toFloat(), parts[1].toFloat(), parts[2].toFloat()));
      return true;
    }

    return false;
  }

private:
  Getter getter;
  Setter setter;
};
