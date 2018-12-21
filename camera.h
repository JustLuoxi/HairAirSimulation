#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QKeyEvent>
#include <QOpenGLShader>
#include <QDebug>
#include <QtMath>


enum Camera_Movement {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  UP,
  DOWN
};

const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 0.25;
const GLfloat SENSITIVITY = 0.1f;
const GLfloat ZOOM = 45.0f;

class Camera {
public:
  Camera(QVector3D position = QVector3D(0.0f, 0.0f, 0.0f), QVector3D up = QVector3D(0.0f, 1.0f, 0.0f),
    GLfloat yaw = YAW, GLfloat pitch = PITCH): front(QVector3D(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED),
    mouseSensitivity(SENSITIVITY), zoom(ZOOM){

    this->position = position;
    this->worldUp = up;
    this->yaw = yaw;
    this->picth = pitch;
    this->updateCameraVectors();
  }

  QMatrix4x4 getViewMatrix(); //lookat function
  void processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = true); //mouse event
  void processMouseScroll(GLfloat yoffset);//
  void processKeyboard(Camera_Movement direction, GLfloat deltaTime);//
  //void processInput(GLfloat dt);

  QVector3D position;
  QVector3D worldUp;
  QVector3D front;

  QVector3D up; //up direction
  QVector3D right; // right direction

    //Eular Angles
  GLfloat picth;
  GLfloat yaw;

    //Camera options
  GLfloat movementSpeed;
  GLfloat mouseSensitivity;
  GLfloat zoom;

private:
  void updateCameraVectors();

};
#endif // CAMERA_H
