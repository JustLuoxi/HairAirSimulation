#ifndef MODEL_H
#define MODEL_H

#include <QOpenGLFunctions_3_3_Core>
#include <resourcemanager.h>
#include <QVector>

#include <QDebug>
#include <QVector3D>
#include <QVector2D>
#include <QOpenGLTexture>
#include <QMap>

#include <vector>
#include "hair.h"
//#include "raytracing.h"

class Object;
class Material;

const GLfloat YAW_JUST = -90.0f;
const GLfloat PITCH_JUST = 0.0f;
const GLfloat SPEED_JUST = 0.25;
const GLfloat SENSITIVITY_JUST = 1.0f;
const GLfloat ZOOM_JUST = 1.0f;

//----------------------------------------------------------------------------
// Constants and macros
//----------------------------------------------------------------------------
#define MAX_VEC3 QVector3D(1e6, 1e6,1e6)
#define  SMALL_NUM 1e-6
#define MAX_NUM 1e10
#define SQUARE(x)            ((x) * (x))

// row-major
//#define MATRC(mat, r, c)     (mat[(c) + 4 * (r)])

// column-major
#define MATRC(mat, r, c)     (mat[(r) + 4 * (c)])

//----------------------------------------------------------------------------
// Type and struct definitions
//----------------------------------------------------------------------------
//--------------------------------------
typedef struct raystruct {
    QVector3D dir;             // direction
    QVector3D invdir;
    QVector3D orig;            // origin
    int sign[3];

} RapidRay;

typedef struct intersectionstruct {
    double t;               // parameter of ray at point of intersection
    QVector3D P;                 // location of hit-point
    QVector3D N;                 // normal QVector3D at hit-point

} Intersection;

typedef struct mesh_triangle_struct {
    QVector3D V0, V1, V2;
    QVector3D center;
    int findex;
} MeshTriangle;

typedef struct bounding_box_struct {
    QVector3D bounds[2];
} BoundingBox;

typedef struct hit_record_struct {
    int findex;
} HitRecord;

typedef struct bvh_node_struct {
    bool leaf;
    bvh_node_struct *lchild = NULL;
    bvh_node_struct *rchild = NULL;
    int mesh_tri_list_index;
    int findex;      /* triangle index, only leaf has triangle */

    BoundingBox box;
} BvhNode;

typedef struct bvh_tree_struct {
    int nodeNum;
    BvhNode *firstNode = NULL;
} BvhTree;


class Model
{
public:
  Model();
  bool init(const QString& path);
  void draw(GLboolean isOpenLighting = GL_FALSE);
  // for interaction
  void processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = true); //mouse event
  void processRotateMovement(GLfloat xoffset, GLfloat yoffset);
  void processMouseScroll(GLfloat yoffset);
  void UpdateMatrix();
  void SetScale(GLfloat s);
  void Reset();
  QMatrix4x4 getModelMatrix();

  // for ray tracing
  void createBvhTree(std::vector<QVector3D> positions);
  float RayShoot2Model(QVector3D orig_, QVector3D dir_, bool &doesHit, QVector3D &hitPoint,int &hitFace);


  // translation, rotation, scale
  QMatrix4x4 m_matrix;
  QMatrix4x4 m_matrix_translate;
  QMatrix4x4 m_matrix_rotate;
  QMatrix4x4 m_matrix_scale;

  GLfloat zoom = ZOOM_JUST;

  // hair just.11.29
  Hair *m_hair;

  QVector3D center;
  BvhTree *mesh_bvhtree = NULL;
  std::vector<MeshTriangle> *mesh_triangles = NULL;
  std::vector<QVector3D> all_positions;

private:
  bool loadOBJ(const QString& path);
  void bindBufferData();

  QOpenGLFunctions_3_3_Core *core;
  QVector<Object> objects;
  QMap<QString, Material> map_materials;

  //Eular Angles
  GLfloat picth = PITCH_JUST;
  GLfloat yaw = YAW_JUST;

  //Camera options
  GLfloat movementSpeed = SPEED_JUST;
  GLfloat mouseSensitivity = SENSITIVITY_JUST;


};

class Object{
public:
  GLuint positionVBO;
  GLuint uvVBO;
  GLuint normalVBO;

  QVector<QVector3D> positions;
  QVector<QVector2D> uvs;
  QVector<QVector3D> normals;

  QString matName;//tex name
};

class Material{
public:
  QVector3D Ka;//ambient
  QVector3D Kd;//diffuse
  QVector3D Ks;//specular
//  QOpenGLTexture* map_Ka;//
//  QOpenGLTexture* map_Kd;//
  double shininess;
  QString name_map_Ka;
  QString name_map_Kd;

};



//************************** Ray Tracing ***************************************
// bvh tree operations
BvhTree *create_new_BvhTree(std::vector<MeshTriangle> *);

BvhNode *create_new_BvhNode(std::vector<MeshTriangle> *, int, int, int);

BvhTree *make_BvhTree(int num);

BvhNode *make_BvhNode();

void create_bounding_box(BoundingBox &, MeshTriangle);

void combine_bounding_box(BoundingBox &, BoundingBox &, BoundingBox &);

void copy_bounding_box(BoundingBox &, BoundingBox &);

bool hit_box(RapidRay *, BoundingBox &);

Intersection *
intersect_ray_BvhTree(RapidRay *, std::vector<MeshTriangle> *, BvhTree *, int &hitFace);

Intersection *
intersect_ray_BvhNode(RapidRay *, std::vector<MeshTriangle> *, BvhNode *, HitRecord *);

bool comp_x(MeshTriangle &a, MeshTriangle &b); // compare along x-axis
bool comp_y(MeshTriangle &a, MeshTriangle &b); // compare along y-axis
bool comp_z(MeshTriangle &a, MeshTriangle &b); // compare along z-axis
//----------------------------------------------------------------------------

//ray
RapidRay *make_ray();

RapidRay *make_ray(QVector3D orig_, QVector3D dir_);

void adjust_ray(RapidRay *);

// ray tracing details
void trace_ray(RapidRay *ray, std::vector<MeshTriangle> *, BvhTree *);

void update_nearest_intersection(Intersection **, Intersection **);

Intersection *make_intersection();

Intersection *intersect_ray_triangle(RapidRay *, QVector3D, QVector3D, QVector3D);

//---------------------------------------------------------------------------
//ray shoot
QVector3D RayShootoMesh(QVector3D orig_, QVector3D dir_, Model *mesh_, int &hitFace);
//QVector3D RayShootoMesh(QVector3D orig_, QVector3D dir_, BvhTree *mesh_bvhtree, std::vector<MeshTriangle> *mesh_triangles, int &hitFace);


#endif // MODEL_H
