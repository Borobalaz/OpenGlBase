#include "ui/qt-adapters/QtSceneWindow.h"

#include <iostream>

#include <glad/glad.h>

#include <QOpenGLContext>
#include <QSurfaceFormat>

namespace
{
  void *QtGetProcAddress(const char *name)
  {
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
      return nullptr;
    }

    return reinterpret_cast<void *>(ctx->getProcAddress(name));
  }
}

QtSceneWindow::QtSceneWindow(const std::string &dwiPath,
                             const std::string &bvalPath,
                             const std::string &bvecPath,
                             QWindow *parent)
    : QOpenGLWindow(NoPartialUpdate, parent),
      scene(nullptr),
      dwiPath(dwiPath),
      bvalPath(bvalPath),
      bvecPath(bvecPath)
{
  setTitle("Connectomics-Imaging (Qt Host)");

  frameTimer.setTimerType(Qt::PreciseTimer);
  frameTimer.setInterval(16);
  QObject::connect(&frameTimer, &QTimer::timeout, this, [this]()
                   { update(); });
}

QtSceneWindow::~QtSceneWindow()
{
  makeCurrent();
  if (scene)
  {
    scene->Destroy();
    scene.reset();
  }
  doneCurrent();
}

void QtSceneWindow::initializeGL()
{
  if (!gladLoadGLLoader(QtGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD in Qt context.\n";
    return;
  }

  scene = std::make_unique<DtiVolumeScene>();
  scene->Init();

  if (!scene->LoadDataset(dwiPath, bvalPath, bvecPath))
  {
    std::cout << "DTI dataset load failed: " << scene->GetLastLoadError() << "\n";
  }

  if (std::shared_ptr<Camera> sceneCamera = scene->GetCamera())
  {
    auto moveComponent = std::make_unique<QtInspectionMovement>(glm::vec3(0.0f));
    movement = moveComponent.get();
    sceneCamera->SetMoveComponent(std::move(moveComponent));
  }

  elapsedTimer.start();
  lastFrameTimeNs = elapsedTimer.nsecsElapsed();
  frameTimer.start();
}

void QtSceneWindow::resizeGL(int width, int height)
{
  if (!scene || height <= 0)
  {
    return;
  }

  const float aspect = static_cast<float>(width) / static_cast<float>(height);
  scene->SetCameraAspect(aspect);
}

void QtSceneWindow::paintGL()
{
  if (!scene)
  {
    return;
  }

  scene->ReloadShadersIfChanged();

  const qint64 nowNs = elapsedTimer.nsecsElapsed();
  const float deltaSeconds = static_cast<float>(nowNs - lastFrameTimeNs) / 1e9f;
  lastFrameTimeNs = nowNs;

  scene->Update(deltaSeconds);
  scene->Render();
}

void QtSceneWindow::keyPressEvent(QKeyEvent *event)
{
  if (movement)
  {
    movement->SetKeyPressed(event->key(), true);
  }
  QOpenGLWindow::keyPressEvent(event);
}

void QtSceneWindow::keyReleaseEvent(QKeyEvent *event)
{
  if (movement)
  {
    movement->SetKeyPressed(event->key(), false);
  }
  QOpenGLWindow::keyReleaseEvent(event);
}

void QtSceneWindow::mousePressEvent(QMouseEvent *event)
{
  if (movement)
  {
    movement->SetMouseButtonPressed(event->button(), true);
    movement->SetMousePosition(event->position());
  }
  QOpenGLWindow::mousePressEvent(event);
}

void QtSceneWindow::mouseReleaseEvent(QMouseEvent *event)
{
  if (movement)
  {
    movement->SetMouseButtonPressed(event->button(), false);
    movement->SetMousePosition(event->position());
  }
  QOpenGLWindow::mouseReleaseEvent(event);
}

void QtSceneWindow::mouseMoveEvent(QMouseEvent *event)
{
  if (movement)
  {
    movement->SetMousePosition(event->position());
  }
  QOpenGLWindow::mouseMoveEvent(event);
}

void QtSceneWindow::wheelEvent(QWheelEvent *event)
{
  if (movement)
  {
    const QPoint angleDelta = event->angleDelta();
    movement->AddScrollDelta(static_cast<float>(angleDelta.y()) / 120.0f);
  }
  QOpenGLWindow::wheelEvent(event);
}
