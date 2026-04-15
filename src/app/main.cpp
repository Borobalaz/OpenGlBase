#include <QApplication>
#include <QSurfaceFormat>

#include "ui/qt-adapters/QtSceneWindow.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  QtSceneWindow window(
      "assets/volumes/dwi/HARDI150_hdbet_masked4d.nii.gz",
      "assets/volumes/dwi/HARDI150.bval",
      "assets/volumes/dwi/HARDI150.bvec");
  window.resize(1280, 720);
  window.show();
  window.requestActivate();

  return app.exec();
}
