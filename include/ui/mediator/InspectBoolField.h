#pragma once

#include <functional>

#include "ui/mediator/InspectField.h"

class InspectBoolField : public InspectField
{
  Q_OBJECT

public:
  using Getter = std::function<bool()>;
  using Setter = std::function<void(bool)>;

  InspectBoolField(QString fieldId,
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

  QUrl editorUrl() const override { return QUrl(QStringLiteral("qrc:/qml/InspectCheckboxField.qml")); }
  QVariant value() const override { return getter ? QVariant::fromValue(getter()) : QVariant(false); }

  bool trySetValue(const QVariant &input) override
  {
    if (!setter)
    {
      return false;
    }

    setter(input.toBool());
    return true;
  }

private:
  Getter getter;
  Setter setter;
};
