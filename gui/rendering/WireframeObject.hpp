//
// Created by acaramizaru on 6/30/23.
//

#pragma once

#include "ObjectInfo.hpp"
#include "ObjectLoader.hpp"
#include "ShaderProgram.hpp"
#include "Utils.hpp"
#include <vector>

#include <QOpenGLWidget>
#include <QtOpenGL>

namespace rendering {

class WireframeObject : protected QOpenGLFunctions {

public:
  WireframeObject(const rendering::ObjectInfo &info, const QColor &color,
                  const rendering::SMesh &mesh, const QOpenGLWidget *Widget,
                  const QVector3D &position = QVector3D(0.0f, 0.0f, 0.0f),
                  const QVector3D &rotation = QVector3D(0.0f, 0.0f, 0.0f),
                  const QVector3D &scale = QVector3D(1.0f, 1.0f, 1.0f));
  WireframeObject(const WireframeObject &cpy) = delete;
  ~WireframeObject();

  void Render(std::unique_ptr<rendering::ShaderProgram> &program,
              float lineWidth = 1);

  void SetRotation(GLfloat rotationX, GLfloat rotationY, GLfloat rotationZ);
  void SetRotation(const QVector3D &rotation);
  QVector3D GetRotation() const;

  void SetPosition(GLfloat positionX, GLfloat positionY, GLfloat positionZ);
  void SetPosition(const QVector3D &position);
  QVector3D GetPosition() const;

  void SetScale(GLfloat scaleX, GLfloat scaleY, GLfloat scaleZ);
  void SetScale(const QVector3D &scale);
  QVector3D GetScale() const;

  void SetColor(const QColor &color);
  rendering::SMesh GetMesh() const;

private:
  std::vector<QVector4D> m_vertices;
  std::vector<GLuint> m_indices;
  QColor m_color;

  QOpenGLContext *m_openGLContext;

  std::vector<GLfloat> m_verticesBuffer;
  std::vector<uint8_t> m_colorBuffer;

  rendering::SMesh m_mesh;

  std::unique_ptr<QOpenGLVertexArrayObject> m_vao;
  GLuint m_vbo;
  GLuint m_colorBufferId;
  GLuint m_elementBufferId;

  QVector3D m_rotation;
  QVector3D m_position;
  QVector3D m_scale;

  void CreateVBO();
  void DestroyVBO();

  void UpdateVBOColor();
};

} // namespace rendering
