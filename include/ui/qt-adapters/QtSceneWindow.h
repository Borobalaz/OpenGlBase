#pragma once

#include <memory>
#include <string>

#include "Scene/DtiVolumeScene.h"

#include <QElapsedTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLWindow>
#include <QTimer>
#include <QWheelEvent>

#include "ui/qt-adapters/QtInspectionMovement.h"

class QtSceneWindow : public QOpenGLWindow
{
public:
  QtSceneWindow(const std::string &dwiPath,
                const std::string &bvalPath,
                const std::string &bvecPath,
                QWindow *parent = nullptr);
  ~QtSceneWindow() override;

protected:
  void initializeGL() override;
  void resizeGL(int width, int height) override;
  void paintGL() override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  std::unique_ptr<DtiVolumeScene> scene;
  QtInspectionMovement *movement = nullptr;

  std::string dwiPath;
  std::string bvalPath;
  std::string bvecPath;

  QElapsedTimer elapsedTimer;
  qint64 lastFrameTimeNs = 0;
  QTimer frameTimer;
};
