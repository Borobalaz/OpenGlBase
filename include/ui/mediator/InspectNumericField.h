#pragma once

#include <functional>

#include "ui/mediator/InspectField.h"

class InspectNumericField : public InspectField
{
  Q_OBJECT

public:
  using Getter = std::function<double()>;
  using Setter = std::function<void(double)>;

  InspectNumericField(QString fieldId,
                      QString displayName,
                      QString groupName,
                      Getter getter,
                      Setter setter,
                      double minimumValue = 0.0,
                      double maximumValue = 1.0,
                      bool readOnly = false,
                      QObject *parent = nullptr)
    : InspectField(std::move(fieldId), std::move(displayName), std::move(groupName), readOnly, parent),
      getter(std::move(getter)),
      setter(std::move(setter)),
      minimumValue(minimumValue),
      maximumValue(maximumValue)
  {
  }

  QUrl editorUrl() const override { return QUrl(QStringLiteral("qrc:/qml/InspectNumberField.qml")); }
  QVariant value() const override { return getter ? QVariant::fromValue(getter()) : QVariant(0.0); }
  double minimum() const override { return minimumValue; }
  double maximum() const override { return maximumValue; }

  bool trySetValue(const QVariant &input) override
  {
    if (!setter)
    {
      return false;
    }

    setter(input.toDouble());
    return true;
  }

private:
  Getter getter;
  Setter setter;
  double minimumValue = 0.0;
  double maximumValue = 1.0;
};
