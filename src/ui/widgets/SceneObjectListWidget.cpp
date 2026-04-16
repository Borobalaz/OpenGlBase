#include "ui/widgets/SceneObjectListWidget.h"

#include <QLabel>
#include <QListWidget>
#include <QSignalBlocker>
#include <QVBoxLayout>

SceneObjectListWidget::SceneObjectListWidget(QWidget *parent)
  : QFrame(parent)
{
  setObjectName("objectsPanel");
  setMinimumWidth(280);
  setMaximumWidth(280);

  auto *objectsLayout = new QVBoxLayout(this);
  objectsLayout->setContentsMargins(12, 12, 12, 12);
  objectsLayout->setSpacing(8);

  auto *objectsTitle = new QLabel("Objects", this);
  objectsTitle->setObjectName("panelTitle");
  objectsLayout->addWidget(objectsTitle);

  objectList = new QListWidget(this);
  objectList->setObjectName("objectList");
  objectsLayout->addWidget(objectList, 1);

  QObject::connect(objectList, &QListWidget::currentRowChanged, this, [this](int row)
  {
    emit currentRowChanged(row);
  });
}

void SceneObjectListWidget::setObjectNames(const QStringList &names)
{
  const QSignalBlocker blocker(objectList);
  objectList->clear();
  objectList->addItems(names);
}

void SceneObjectListWidget::setCurrentIndex(int index, bool emitSignal)
{
  if (emitSignal)
  {
    objectList->setCurrentRow(index);
    return;
  }

  const QSignalBlocker blocker(objectList);
  objectList->setCurrentRow(index);
}
