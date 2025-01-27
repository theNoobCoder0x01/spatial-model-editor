//
// Created by acaramizaru on 7/25/23.
//

#include "catch_wrapper.hpp"
#include "qt_test_utils.hpp"

#include "qopenglmousetracker.hpp"
#include "rendering/rendering.hpp"
#include "sme/logger.hpp"

using namespace sme::test;

static const char *tags{"[gui/widgets/QOpenGLMouseTracker][gui][opengl]"};

TEST_CASE("QOpenGLMouseTracker: OpenGL", tags) {

  QOpenGLMouseTracker test = QOpenGLMouseTracker();

  QColor redColor = QColor(255, 0, 0);
  QColor blueColor = QColor(0, 0, 255);
  QColor blackColor = QColor(0, 0, 0);

  test.show();

  wait(100);

  // camera position
  test.SetCameraPosition(0, 0, -10);

  // loading meshes
  QFile::copy(":/test/rendering/Objects/sphere.ply", "tmp_sphere.ply");
  REQUIRE(QFile::exists("tmp_sphere.ply"));
  rendering::SMesh sphereMesh = rendering::ObjectLoader::LoadMesh(
      QDir::current().filePath("tmp_sphere.ply").toStdString());
  test.addMesh(sphereMesh, redColor);

  wait(100);

  QFile::copy(":/test/rendering/Objects/teapot.ply", "tmp_teapot.ply");
  QFileInfo info("tmp_teapot.ply");
  REQUIRE(QFile::exists("tmp_teapot.ply"));

  wait(100);

  rendering::SMesh teapotMesh = rendering::ObjectLoader::LoadMesh(
      QDir::current().filePath("tmp_teapot.ply").toStdString());
  test.addMesh(teapotMesh, blueColor);

  auto QcolorSelection = QColor(test.getColour());

  // forced windows resize and forced repainting
  test.resize(500, 500);
  // test.repaint();

  // the corner initial color should be black.
  REQUIRE(blackColor == QcolorSelection);

  // zoom
  sendMouseWheel(&test, 1);

  // move mouse over image
  sendMouseMove(&test, {10, 44});

  // click on image
  sendMouseClick(&test, {40, 40});
  // test.repaint();

  QcolorSelection = QColor(test.getColour());

  REQUIRE(blueColor != QcolorSelection);

  wait(100);

  sendMouseClick(&test, {0, 0});

  QcolorSelection = QColor(test.getColour());

  REQUIRE(blueColor == QcolorSelection);

  wait(100);

  sendMouseClick(&test, {376, 366});

  QcolorSelection = QColor(test.getColour());

  REQUIRE(redColor == QcolorSelection);

  wait(100);

  // reset
  sendMouseClick(&test, {412, 445});

  QcolorSelection = QColor(test.getColour());

  REQUIRE(blackColor == QcolorSelection);

  wait(100);

  // reset
  sendMouseClick(&test, {412, 445});

  QcolorSelection = QColor(test.getColour());

  REQUIRE(blackColor == QcolorSelection);

  wait(100);

  sendMouseClick(&test, {0, 0});

  QcolorSelection = QColor(test.getColour());

  REQUIRE(blueColor == QcolorSelection);

  wait(100);

  sendMouseClick(&test, {376, 366});

  QcolorSelection = QColor(test.getColour());

  REQUIRE(redColor == QcolorSelection);

  wait(100);

  // reset
  sendMouseClick(&test, {412, 445});

  QcolorSelection = QColor(test.getColour());

  REQUIRE(blackColor == QcolorSelection);
}
