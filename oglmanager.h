#ifndef OGLMANAGER_H
#define OGLMANAGER_H

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QOpenGLFunctions_3_3_Core>
#include <QTime>
#include <QMouseEvent>
#include <QMainWindow>

#include "camera.h"
#include "model.h"
#include "air.h"

static const int FPS = 60;
class MainWindow;

class OGLManager : public QOpenGLWidget{
public:
    explicit OGLManager(QWidget *parent = 0);
    ~OGLManager();
    void handleKeyPressEvent(QKeyEvent *event);
    void handleKeyReleaseEvent(QKeyEvent *event);
    void changeObjModel(QString fileName);
    void setModelScale();

    GLfloat modelScaling;// obj scale
    GLboolean keys[1024];//multi-key press
    GLboolean isOpenLighting;
    GLboolean isLineMode;

    // for animation
    void AddKeyFrame(int frame);
    void RuntheAnimation();
    void SetFrame(int frame);

    std::vector<QMatrix4x4> frame_matrixs;
    std::vector<QMatrix4x4> temp_translate_matrixs;
    std::vector<QMatrix4x4> temp_rotate_matrixs;
    std::vector<QMatrix4x4> temp_scale_matrixs;
    std::vector<int> key_frame_indices;

    GLboolean run_animation_flag = false;
    GLboolean has_run_animation = false;
    int frame_num = 100;
    int current_frame = 0;
    int m_fps = FPS;
    GLfloat m_deltatime = 0.0f;

    MainWindow *window;

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();
private:
    void processInput(GLfloat dt);//handle input
    void updateGL();

    // for animation
    QVector3D _translationMat2Vec3(QMatrix4x4 m);
    QMatrix4x4 _translationVec32Mat(QVector3D v);
    QOpenGLFunctions_3_3_Core *core;

    GLboolean isFirstMouse;
    GLboolean isLeftMousePress;
    GLboolean isRightMousePress;
    GLint lastX;
    GLint lastY;

    QTime time;
    GLfloat deltaTime;
    GLfloat lastFrame;

    Camera *camera;


};

struct Quaternion
{
    float w, x, y, z;

    Quaternion()
    {
        w=0;x=0;y=0;z=0;
    }
    Quaternion(float _w, float _x, float _y, float _z)
    {
        w = _w; x=_x;y=_y;z=_z;
    }

    // This constructor use 'injects' a QVector3D into the imaginary part of the Quaternion (i.e. x, y, z values)
    Quaternion(float _w, QVector3D v)
    {
        w=_w, x=v.x(), y=v.y(), z=v.z();
    }
};



// SLERP(Spherical linear interpolation) moves a point from one position to another over time
Quaternion Slerp(Quaternion a, Quaternion b, double t);

// Returns a Matrix4D used for rotation
QMatrix4x4  RotationMatrix4D(Quaternion q);

Quaternion Matrix4D2Quaternion(QMatrix4x4 m);

float Norm(Quaternion q);

Quaternion Normalize(Quaternion q);

#endif // OGLMANAGER_H
