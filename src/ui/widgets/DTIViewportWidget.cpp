#include "ui/widgets/DTIViewportWidget.h"

#include <iostream>
#include <memory>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QWheelEvent>
#include <QtGlobal>

#include "core/DtiVolumeScene.h"
#include "ui/qt-adapters/InspectQtAdapter.h"
#include "ui/qt-adapters/QtInspectionMovement.h"

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

DTIViewportWidget::DTIViewportWidget(QWidget *parent)
  : QOpenGLWidget(parent)
{
  inspectAdapterObject = new InspectQtAdapter(this);

  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);

  frameTimer.setTimerType(Qt::PreciseTimer);
  frameTimer.setInterval(16);
  QObject::connect(&frameTimer, &QTimer::timeout, this, [this]()
  {
    update();
  });
  frameTimer.start();
}

DTIViewportWidget::~DTIViewportWidget() = default;

QString DTIViewportWidget::dwiPath() const
{
  return dwiPathValue;
}

QString DTIViewportWidget::bvalPath() const
{
  return bvalPathValue;
}

QString DTIViewportWidget::bvecPath() const
{
  return bvecPathValue;
}

InspectQtAdapter &DTIViewportWidget::inspectAdapter() const
{
  Q_ASSERT(inspectAdapterObject != nullptr);
  return *inspectAdapterObject;
}

void DTIViewportWidget::setDwiPath(const QString &path)
{
  if (dwiPathValue == path)
  {
    return;
  }

  dwiPathValue = path;
  emit dwiPathChanged();
}

void DTIViewportWidget::setBvalPath(const QString &path)
{
  if (bvalPathValue == path)
  {
    return;
  }

  bvalPathValue = path;
  emit bvalPathChanged();
}

void DTIViewportWidget::setBvecPath(const QString &path)
{
  if (bvecPathValue == path)
  {
    return;
  }

  bvecPathValue = path;
  emit bvecPathChanged();
}

void DTIViewportWidget::initializeGL()
{
  if (!gladLoadGLLoader(QtGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD in widgets OpenGL context.\n";
    return;
  }

  initializeScene();
}

void DTIViewportWidget::resizeGL(int width, int height)
{
  if (scene && height > 0)
  {
    scene->SetCameraAspect(static_cast<float>(width) / static_cast<float>(height));
  }
}

void DTIViewportWidget::paintGL()
{
  if (!scene)
  {
    return;
  }

  updateInspectProviders(scene.get());

  if (height() > 0)
  {
    scene->SetCameraAspect(static_cast<float>(width()) / static_cast<float>(height()));
  }

  scene->ReloadShadersIfChanged();

  const qint64 nowNs = elapsedTimer.nsecsElapsed();
  float deltaSeconds = 0.0f;
  if (lastFrameTimeNs == 0)
  {
    deltaSeconds = 1.0f / 60.0f;
  }
  else
  {
    deltaSeconds = static_cast<float>(nowNs - lastFrameTimeNs) / 1e9f;
  }
  lastFrameTimeNs = nowNs;

  scene->Update(deltaSeconds);
  scene->Render();
}

void DTIViewportWidget::keyPressEvent(QKeyEvent *event)
{
  if (movement)
  {
    movement->SetKeyPressed(event->key(), true);
  }

  event->accept();
}

void DTIViewportWidget::keyReleaseEvent(QKeyEvent *event)
{
  if (movement)
  {
    movement->SetKeyPressed(event->key(), false);
  }

  event->accept();
}

void DTIViewportWidget::mousePressEvent(QMouseEvent *event)
{
  setFocus();

  if (movement)
  {
    movement->SetMouseButtonPressed(event->button(), true);
    movement->SetMousePosition(event->position());
  }

  event->accept();
}

void DTIViewportWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if (movement)
  {
    movement->SetMouseButtonPressed(event->button(), false);
    movement->SetMousePosition(event->position());
  }

  event->accept();
}

void DTIViewportWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (movement)
  {
    movement->SetMousePosition(event->position());
  }

  event->accept();
}

void DTIViewportWidget::wheelEvent(QWheelEvent *event)
{
  if (movement)
  {
    const QPoint angleDelta = event->angleDelta();
    movement->AddScrollDelta(static_cast<float>(angleDelta.y()) / 120.0f);
  }

  event->accept();
}

void DTIViewportWidget::initializeScene()
{
  scene = std::make_unique<DtiVolumeScene>();
  scene->Init();

  if (!scene->LoadDataset(dwiPath().toStdString(), bvalPath().toStdString(), bvecPath().toStdString()))
  {
    std::cout << "DTI dataset load failed: " << scene->GetLastLoadError() << "\n";
  }

  if (auto sceneCamera = scene->GetCamera())
  {
    auto moveComponent = std::make_unique<QtInspectionMovement>(glm::vec3(0.0f));
    movement = moveComponent.get();
    sceneCamera->SetMoveComponent(std::move(moveComponent));
  }

  elapsedTimer.start();
  lastFrameTimeNs = 0;
}

void DTIViewportWidget::updateInspectProviders(Scene *sceneObject)
{
  if (!inspectAdapterObject)
  {
    return;
  }

  if (!sceneObject)
  {
    inspectAdapterObject->SetProviders({});
    return;
  }

  inspectAdapterObject->SetProviders(sceneObject->GetInspectProviders());
}
