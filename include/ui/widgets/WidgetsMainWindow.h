#pragma once

#include <QMainWindow>

class DTIViewportWidget;
class InspectorWidget;
class InspectQtAdapter;
class SceneObjectListWidget;

class WidgetsMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit WidgetsMainWindow(QWidget *parent = nullptr);

private:
  void setupLayout();
  void wireAdapterSignals();

  void refreshObjectList();
  void syncObjectSelection();

  DTIViewportWidget *viewportWidget = nullptr;
  InspectorWidget *inspectorWidget = nullptr;

  SceneObjectListWidget *sceneObjectListWidget = nullptr;
};
