#pragma once

#include <QStringList>

#include <QFrame>

class QListWidget;

class SceneObjectListWidget : public QFrame
{
  Q_OBJECT

public:
  explicit SceneObjectListWidget(QWidget *parent = nullptr);

  void setObjectNames(const QStringList &names);
  void setCurrentIndex(int index, bool emitSignal = true);

signals:
  void currentRowChanged(int row);

private:
  QListWidget *objectList = nullptr;
};
