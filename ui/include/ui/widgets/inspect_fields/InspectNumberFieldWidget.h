#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <utility>

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QObject>
#include <QSignalBlocker>
#include <QPointer>
#include <QString>
#include <QVariant>

#include "ui/widgets/inspect_fields/IInspectWidget.h"

class InspectNumberFieldWidget : public QObject, public IInspectWidget
{
  Q_OBJECT

public:
  using Getter = std::function<double()>;
  using Setter = std::function<void(double)>;

  InspectNumberFieldWidget(QString fieldId,
                           QString displayName,
                           QString groupName,
                           Getter getter,
                           Setter setter,
                           double minimumValue = 0.0,
                           double maximumValue = 1.0,
                           double stepSize = 0.01,
                           bool readOnly = false,
                           QObject *parent = nullptr)
    : QObject(parent),
      fieldIdValue(std::move(fieldId)),
      displayNameValue(std::move(displayName)),
      groupNameValue(std::move(groupName)),
      getter(std::move(getter)),
      setter(std::move(setter)),
      minimumValue(minimumValue),
      maximumValue(maximumValue),
      stepSize(std::max(stepSize, 1e-12)),
      readOnlyValue(readOnly)
  {
  }

  InspectNumberFieldWidget(QString fieldId,
                           QString displayName,
                           QString groupName,
                           double minimumValue = 0.0,
                           double maximumValue = 1.0,
                             double stepSize = 0.01,
                           bool readOnly = false,
                           QObject *parent = nullptr)
    : InspectNumberFieldWidget(std::move(fieldId),
                               std::move(displayName),
                               std::move(groupName),
                               Getter{},
                               Setter{},
                               minimumValue,
                               maximumValue,
                               stepSize,
                               readOnly,
                               parent)
  {
  }

  QString fieldId() const override { return fieldIdValue; }
  QString displayName() const override { return displayNameValue; }
  QString groupName() const override { return groupNameValue; }
  bool isReadOnly() const override { return readOnlyValue; }
  double minimum() const override { return minimumValue; }
  double maximum() const override { return maximumValue; }
  double singleStep() const { return stepSize; }

  QVariant value() const
  {
    return getter ? QVariant::fromValue(getter()) : QVariant(0.0);
  }

  QVariant GetValue() const override { return value(); }

  void SetValue(const QVariant &input) override
  {
    const double numericValue = input.toDouble();

    if (setter)
    {
      setter(numericValue);
    }

    if (editorWidget)
    {
      const QSignalBlocker blocker(editorWidget);
      editorWidget->setValue(numericValue);
    }
  }

  IInspectWidget *addToLayout(QHBoxLayout *layout) override
  {
    auto *editor = new QDoubleSpinBox;
    editor->setDecimals(DecimalsForStep(stepSize));
    editor->setSingleStep(stepSize);
    editor->setButtonSymbols(QAbstractSpinBox::NoButtons);
    editor->setMinimumWidth(0);
    editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    editor->setEnabled(!isReadOnly());

    if (minimumValue < maximumValue)
    {
      editor->setRange(minimumValue, maximumValue);
    }
    else
    {
      editor->setRange(-1e9, 1e9);
    }

    QObject::connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), editor, [this](double v)
    {
      SetValue(v);
    });

    const QSignalBlocker blocker(editor);
    editor->setValue(value().toDouble());
    layout->addWidget(editor, 1);
    editorWidget = editor;
    return this;
  }

  ~InspectNumberFieldWidget() override = default;

private:
  QString fieldIdValue;
  QString displayNameValue;
  QString groupNameValue;
  Getter getter;
  Setter setter;
  double minimumValue = 0.0;
  double maximumValue = 1.0;
  double stepSize = 0.01;
  bool readOnlyValue = false;
  QPointer<QDoubleSpinBox> editorWidget;

  static int DecimalsForStep(double step)
  {
    if (!(step > 0.0))
    {
      return 2;
    }

    const double digits = -std::log10(step);
    if (digits <= 0.0)
    {
      return 0;
    }

    return std::clamp(static_cast<int>(std::ceil(digits)), 0, 10);
  }
};
