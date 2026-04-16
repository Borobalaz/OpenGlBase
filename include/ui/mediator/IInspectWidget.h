#pragma once

#include <QString>
#include <QVariant>

class IInspectWidget
{
public:
  virtual ~IInspectWidget() = default;

  virtual QString WidgetTypeId() const = 0;
  virtual void SetValue(const QVariant& value) = 0;
  virtual QVariant GetValue() const = 0;
};
