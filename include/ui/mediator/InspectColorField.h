#pragma once

#include "ui/mediator/InspectVec3Field.h"

class InspectColorField : public InspectVec3Field
{
  Q_OBJECT

public:
  using InspectVec3Field::InspectVec3Field;

  QUrl editorUrl() const override { return QUrl(QStringLiteral("qrc:/qml/InspectColorField.qml")); }
};
