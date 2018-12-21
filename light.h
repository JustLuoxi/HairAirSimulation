#ifndef LIGHT_H
#define LIGHT_H

#include <QOpenGLFunctions_3_3_Core>

class Light{
public:
  Light();
  ~Light();
  void init();
  void drawLight();
private:
  QOpenGLFunctions_3_3_Core *core;
  GLuint lightVBO;
};

#endif // LIGHT_H
