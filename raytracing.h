#ifndef RAYTRACING_H
#define RAYTRACING_H

#include <vector>
#include <QVector>
#include <QVector3D>

#define MAX_VEC3 QVector3D(1e6, 1e6,1e6)
#define  SMALL_NUM 1e-6
#define MAX_NUM 1e10

//class Model;
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
    QVector3D *V0, *V1, *V2;
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

RapidRay *make_ray(QVector3D, QVector3D);

void adjust_ray(RapidRay *);

// ray tracing details
void trace_ray(RapidRay *ray, std::vector<MeshTriangle> *, BvhTree *);

void update_nearest_intersection(Intersection **, Intersection **);

Intersection *make_intersection();

Intersection *intersect_ray_triangle(RapidRay *, QVector3D, QVector3D, QVector3D);

//---------------------------------------------------------------------------
//ray shoot
//QVector3D RayShootoMesh(QVector3D orig_, QVector3D dir_, Model *mesh_, int &hitFace);
QVector3D RayShootoMesh(QVector3D orig_, QVector3D dir_, BvhTree *mesh_bvhtree, std::vector<MeshTriangle> *mesh_triangles, int &hitFace);



#endif // RAYTRACING_H
