#ifndef HAIR_H
#define HAIR_H

#include <QOpenGLFunctions_3_3_Core>
#include <resourcemanager.h>
#include <QVector>

#include <QDebug>
#include <QVector3D>
#include <QVector2D>
#include <QOpenGLTexture>
#include <QMap>

#include "stdlib.h"
#include <cmath>

class Model;
class Hairline;
class Spring;

// *********** config **************** //
const GLint SPRING_NUM_JUST = 15;
const GLfloat SPRING_K_JUST = 30.0f;
const GLfloat SPRING_LENGTH_JUST = 0.075f;

const GLint GUIDEHAIRNUM_JUST = 200;

const GLfloat PARTICLE_MASS_JUST = 1.0f;
const GLfloat GRAVITY_JUST = 7.0f;
const GLfloat DAMPING_K_JUST = 2.0f;

const QVector3D HEAD_UP = QVector3D(0,0.25f,0);
const QVector3D HEAD_DOWN = QVector3D(0,-0.25f,0);
const GLfloat HEAD_W = 0.40f;

const GLfloat AIR2HAIRSCALE = 50.0f;

class Hair
{
public:
    Hair();
    Hair(QVector<QVector3D> _all_root_ps,QVector<QVector3D> _all_root_ns,Model *model);
    ~Hair();

    bool Init(QVector<QVector3D> _all_root_ps, QVector<QVector3D> _all_root_ns);
    void Draw(GLboolean isOpenLighting);
    void Update(QMatrix4x4 model);

    QVector<Hairline> m_guide_hairlines;
//    QVector<Spring *> * m_guide_Springs;
//    QVector<QVector3D *> * m_guide_ps;
//    int guide_num;

private:
    void _selectGuideHairline();
    void _bindBufferData();

    QOpenGLFunctions_3_3_Core *core;
    Model *head_model;

};

class Hairline
{
public:
    Hairline();
    Hairline(QVector3D root_p, QVector3D n,Model *model);
    ~Hairline();

    void update(QMatrix4x4 model);
    void updateRenderData();

    GLuint VBO;
    GLuint nVBO;
    QVector<Spring*> *m_Springs;
    QVector<QVector3D > m_ps;
    QVector<QVector3D > render_ps;
    QVector<QVector3D > n_render_ps;

    QVector3D m_root_p;
    QVector3D m_n;
    QMatrix4x4 m_matrix;

private:
     QVector<QVector3D> _bSpline(QVector<QVector3D> ps, int num);
     QVector3D _linearInterpolate(QVector3D s, QVector3D e, GLfloat t);
     void _archimedeanspiral(QVector<QVector3D> ps);

     Model *head_model;

};

class Spring
{
public:
    Spring();
    Spring(QVector3D *_last, QVector3D *_x, QVector3D *_next, QVector3D _root, Model *model);
    ~Spring();

    void update(QMatrix4x4 model);
    bool detectCollision(QVector3D p, QVector3D &closestPoint);
    GLfloat dis2Head(QVector3D p,QVector3D a, QVector3D b);

    QVector3D checkSpringLength(QVector3D p);
    QVector3D checkSpringHeadCollison(QVector3D p, QVector3D head_down, QVector3D head_up);

    QVector3D *m_last;   // last point
    QVector3D *m_x;   // this point
    QVector3D *m_next; // next point

    QVector3D m_v;
    QVector3D head_down=HEAD_DOWN;
    QVector3D head_Up = HEAD_UP;
    GLfloat g = GRAVITY_JUST;
    QVector3D m_root_p;
    GLfloat m_delt;
    GLfloat m;

    GLfloat head_k= 2.3f*SPRING_K_JUST;
    GLfloat k = SPRING_K_JUST;

    bool balance_flag = false;
    QMatrix4x4 old_model_matrix;

    Model *head_model;
};

#endif // HAIR_H
