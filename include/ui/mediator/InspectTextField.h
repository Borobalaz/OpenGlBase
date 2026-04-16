#pragma once

#include <functional>

#include "ui/mediator/InspectField.h"

class InspectTextField : public InspectField
{
  Q_OBJECT

public:
  using Getter = std::function<QString()>;
  using Setter = std::function<void(const QString&)>;

  InspectTextField(QString fieldId,
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

  QUrl editorUrl() const override { return QUrl(QStringLiteral("qrc:/qml/InspectTextField.qml")); }
  QVariant value() const override { return getter ? QVariant(getter()) : QVariant(QString()); }

  bool trySetValue(const QVariant &input) override
  {
    if (!setter)
    {
      return false;
    }

    setter(input.toString());
    return true;
  }

private:
  Getter getter;
  Setter setter;
};
