#pragma once

#include <functional>

#include <QStringList>

#include "ui/mediator/InspectField.h"

class InspectDropdownField : public InspectField
{
  Q_OBJECT

public:
  using Getter = std::function<QString()>;
  using Setter = std::function<void(const QString&)>;

  InspectDropdownField(QString fieldId,
                       QString displayName,
                       QString groupName,
                       QStringList options,
                       Getter getter,
                       Setter setter,
                       bool readOnly = false,
                       QObject *parent = nullptr)
    : InspectField(std::move(fieldId), std::move(displayName), std::move(groupName), readOnly, parent),
      options(std::move(options)),
      getter(std::move(getter)),
      setter(std::move(setter))
  {
  }

  QUrl editorUrl() const override { return QUrl(QStringLiteral("qrc:/qml/InspectDropdownField.qml")); }
  QVariant value() const override { return getter ? QVariant(getter()) : QVariant(QString()); }
  QStringList enumOptions() const override { return options; }

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
  QStringList options;
  Getter getter;
  Setter setter;
};
