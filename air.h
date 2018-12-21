#ifndef AIR_H
#define AIR_H

#include <QOpenGLFunctions_3_3_Core>
#include <resourcemanager.h>
#include <QVector>

#include <QDebug>
#include <QVector3D>
#include <QVector2D>
#include <QOpenGLTexture>
#include <QMap>

#include "oglmanager.h"

//----------------------------------------------------------------------------
// Type and struct definitionsw
//----------------------------------------------------------------------------
const GLfloat RADIUS = 2.0f;
const int RESOLUTION = 20;
const int LINEARSOLVERITER = 50;
const GLfloat GRIDSTEP = (RADIUS*2)/(RESOLUTION-1);

const GLfloat DELTAT = 1.0f/10;
const GLfloat DIFFUSE_JUST = 0.0f;
const GLfloat VISCOSITY_JUST = 0.0f;

#define IDX(i,j,k) ((i)+RESOLUTION*(j) + RESOLUTION*RESOLUTION*(k))
#define SWAP(x,y) {GLfloat * temp = x; x = y; x = temp;}
#define W2A(x) (x+RADIUS)/GRIDSTEP // world space to air index

typedef struct grid_struct{
    GLfloat dt = DELTAT;
    GLfloat diff = DIFFUSE_JUST;
    GLfloat viscosity = VISCOSITY_JUST;

    GLfloat *rho;

    GLfloat *Vx;
    GLfloat *Vy;
    GLfloat *Vz;

    // previous ones
    GLfloat *rho_p;

    GLfloat *Vx_p;
    GLfloat *Vy_p;
    GLfloat *Vz_p;
} Grid;


class Air
{
public:
    Air();
    ~Air();

    void Init();
    void UpdateRenderdata();
    void Draw();

    // for simulation
    void Simulate();
    // for coupling
    void HairInfluenceAir();

    void DensityStep(GLfloat *x, GLfloat *x0, GLfloat *vx, GLfloat *vy, GLfloat *vz, GLfloat dt);
    void VelocityStep(GLfloat *vx, GLfloat *vy, GLfloat *vz, GLfloat *vx_p, GLfloat *vy_p, GLfloat *vz_p, GLfloat dt);
    void AddDensity(int x, int y, int z, GLfloat d);
    void AddVelocity(int x, int y, int z, GLfloat vx, GLfloat vy, GLfloat vz);
    void Reset();

    // for interfaces
    QVector3D GetVelocity(GLfloat x, GLfloat y, GLfloat z);

    // for air-hair coupling
    Hair *hair;

    // render flag
    bool render_flag = true;
    // hair -> air flag
    bool hair2air_flag = true;

private:
    // for rendering
    void bindBufferData();

    void createGrid();

    void setBound(int b, GLfloat *v); // ??
    void mySolver(int b, GLfloat *v, GLfloat *v_p, GLfloat a, GLfloat c);
    void diffuse(int b, GLfloat *v, GLfloat *v_p, GLfloat diff, GLfloat dt);
    void advect(int b, GLfloat *d, GLfloat *d_p, GLfloat *vx,GLfloat *vy, GLfloat *vz,GLfloat dt);
    void project(GLfloat *vx, GLfloat *vy, GLfloat *vz, GLfloat *p, GLfloat *div);
    GLfloat trilinearInterpolation(GLfloat x, GLfloat y, GLfloat z, GLfloat *source);

    int check2ID(GLfloat &x);

    // for rendering
    GLuint VBO;
    QOpenGLFunctions_3_3_Core *core;
    QVector<QVector3D > render_ps;

    Grid *m_grid;
    GLfloat dt = DELTAT;
    GLfloat diff = DIFFUSE_JUST;
    GLfloat viscosity = VISCOSITY_JUST;
};

#endif // AIR_H
