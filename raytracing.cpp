// ****************** Ray Tracing **************************************
//----------------------------------------------------------------------------
// Constants and macros
//----------------------------------------------------------------------------
#include "raytracing.h"
//#include "model.h"

#define SQUARE(x)            ((x) * (x))

// row-major
//#define MATRC(mat, r, c)     (mat[(c) + 4 * (r)])

// column-major
#define MATRC(mat, r, c)     (mat[(r) + 4 * (c)])

#define X 0
#define Y 1
#define Z 2
#define W 3

#define R 0
#define G 1
#define B 2
#define A 3

//#define LEFT   0
//#define RIGHT  1
//#define BOTTOM 2
//#define TOP    3
//#define NEAR   4
//#define FAR    5

//#define std::min(a, b) (a)<(b)?(a):(b)
//#define std::max(a, b) (a)<(b)?(b):(a)

// delete bvh node
void delete_BvhNode(BvhNode *node) {
    if (node != NULL) {
        if (node->lchild != NULL) delete_BvhNode(node->lchild);
        if (node->rchild != NULL) delete_BvhNode(node->rchild);
        free(node);
        node=NULL;
    }
}

// delete bvh tree
void delete_BvhTree(BvhTree *tree) {
    ////LOGI("DEGIN: 1.1");
    if (tree != NULL) {
        //LOGI("1.1 a -- e");
        if (tree->firstNode != NULL) {
            //LOGI("DEGIN: 1.1 first  -- e");
            if (tree->firstNode->lchild != NULL)
            {
                delete_BvhNode(tree->firstNode->lchild);
                //LOGI("DEGIN: 1.1 first  lc -- e");
            }
            if (tree->firstNode->rchild != NULL)
            {
                delete_BvhNode(tree->firstNode->rchild);
                //LOGI("DEGIN: 1.1 first  rc -- e");
            }
            free(tree->firstNode);
            tree->firstNode = NULL;
            //LOGI("END: 1.1 first  -- e");
        }
        free(tree);
        tree = NULL;
        //LOGI("END: 1.1 a -- e");
    }
    else
    {
        //LOGI("1.1 b -- NULL");
    }

    //LOGI("$$$$ Successfully: 1.1");
}

// make new bvh tree

BvhTree *make_BvhTree(int num) {////////////////////////////////////tt
    BvhTree *bvhtree;
    bvhtree = (BvhTree *) malloc(sizeof(BvhTree));
    bvhtree->nodeNum = num;
    return bvhtree;
}
//----------------------------------------------------------------------------

// make new bvh node

BvhNode *make_BvhNode() {//////////////////////////////////////////tt
    BvhNode *bvhnode;
    bvhnode = (BvhNode *) malloc(sizeof(BvhNode));
    bvhnode->leaf = false;
    bvhnode->lchild = NULL;
    bvhnode->rchild = NULL;
    bvhnode->findex = -1;
    return bvhnode;
}

//----------------------------------------------------------------------------
// create new bvh tree
BvhTree *create_new_BvhTree(std::vector<MeshTriangle> *mesh_triangle) {
    BvhTree *bvhtree = NULL;
    bvhtree = make_BvhTree(mesh_triangle->size());

    //compute each triangle's center
    std::vector<MeshTriangle>::iterator titer = mesh_triangle->begin();
    QVector3D center_;
    for (; titer != mesh_triangle->end(); titer++) {
        center_ = QVector3D(0, 0, 0);
        center_ += *((*titer).V0);
        center_ += *((*titer).V1);
        center_ += *((*titer).V2);
        center_ /= 3;
        (*titer).center = center_;
    }
    bvhtree->firstNode = create_new_BvhNode(mesh_triangle, 0, bvhtree->nodeNum - 1, 0);
    return bvhtree;
}


//----------------------------------------------------------------------------

// create new bvh node
BvhNode *
create_new_BvhNode(std::vector<MeshTriangle> *mesh_triangle, int start, int end, int axis) {
    BvhNode *bvhnode = NULL;
    int num = end - start + 1;
    int mid;
    if (num > 0) {
        bvhnode = make_BvhNode();

        if (num == 1) {
            bvhnode->leaf = true;
            bvhnode->mesh_tri_list_index = start;
            bvhnode->findex = mesh_triangle->at(start).findex;
            create_bounding_box(bvhnode->box, mesh_triangle->at(start));
        } else {
            if (axis == 0) {
                sort(mesh_triangle->begin() + start, mesh_triangle->begin() + end + 1, comp_x);
            } else if (axis == 1) {
                sort(mesh_triangle->begin() + start, mesh_triangle->begin() + end + 1, comp_y);
            } else {
                sort(mesh_triangle->begin() + start, mesh_triangle->begin() + end + 1, comp_z);
            }
            mid = (start + end) >> 1;

            bvhnode->lchild = create_new_BvhNode(mesh_triangle, start, mid, (axis + 1) % 3);
            bvhnode->rchild = create_new_BvhNode(mesh_triangle, mid + 1, end, (axis + 1) % 3);
            if (bvhnode->lchild != NULL && bvhnode->rchild != NULL) {
                combine_bounding_box(bvhnode->lchild->box, bvhnode->rchild->box, bvhnode->box);
            } else if (bvhnode->lchild != NULL) {
                copy_bounding_box(bvhnode->box, bvhnode->lchild->box);
            } else {
                copy_bounding_box(bvhnode->box, bvhnode->rchild->box);
            }

        }
    }

    return bvhnode;

}


//----------------------------------------------------------------------------

// hit box

bool hit_box(RapidRay *ray, BoundingBox &box) {
    double tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin = (box.bounds[ray->sign[X]][X] - ray->orig[X]) * ray->invdir[X];
    tmax = (box.bounds[1 - ray->sign[X]][X] - ray->orig[X]) * ray->invdir[X];
    tymin = (box.bounds[ray->sign[Y]][Y] - ray->orig[Y]) * ray->invdir[Y];
    tymax = (box.bounds[1 - ray->sign[Y]][Y] - ray->orig[Y]) * ray->invdir[Y];

    if ((tmin > tymax) || (tymin > tmax))
        return false;
    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    tzmin = (box.bounds[ray->sign[Z]][Z] - ray->orig[Z]) * ray->invdir[Z];
    tzmax = (box.bounds[1 - ray->sign[Z]][Z] - ray->orig[Z]) * ray->invdir[Z];

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;
    /*
    if (tzmin > tmin)
    tmin = tzmin;
    if (tzmax < tmax)
    tmax = tzmax;
    */
    return true;
}

// create new bounding box

void create_bounding_box(BoundingBox &box, MeshTriangle tri) {
    for (int i = 0; i < 3; i++) {
        box.bounds[0][i] = std::min((*tri.V0)[i], (*tri.V1)[i]);
        box.bounds[0][i] = std::min(box.bounds[0][i], (*tri.V2)[i]);

        box.bounds[1][i] = std::max((*tri.V0)[i], (*tri.V1)[i]);
        box.bounds[1][i] = std::max(box.bounds[1][i], (*tri.V2)[i]);
    }
}


//----------------------------------------------------------------------------

// combine bounding boxes

void combine_bounding_box(BoundingBox &box1, BoundingBox &box2, BoundingBox &box3) {
    for (int i = 0; i < 3; i++) {
        box3.bounds[0][i] = std::min(box1.bounds[0][i], box2.bounds[0][i]);
        box3.bounds[1][i] = std::max(box1.bounds[1][i], box2.bounds[1][i]);
    }
}
// copy bounding box

void copy_bounding_box(BoundingBox &dst, BoundingBox &src) {
    dst.bounds[0] = src.bounds[0];
    dst.bounds[1] = src.bounds[1];
}

//----------------------------------------------------------------------------
// Bvh Tree: intersect ray with .obj model (a bunch of triangles with precomputed normals)
// if we hit something, set the color and return true

Intersection *
intersect_ray_BvhTree(RapidRay *ray, std::vector<MeshTriangle> *mesh_triangle, BvhTree *bvhtree,
                      int &hitFace) {
    //static GLMtriangle* triangle;
    HitRecord *hit_record = new HitRecord();
    Intersection *inter = NULL;
    //GLMmodel * model = NULL;

    inter = intersect_ray_BvhNode(ray, mesh_triangle, bvhtree->firstNode, hit_record);

    hitFace = hit_record->findex;
    delete (hit_record);
    return inter;
}

//----------------------------------------------------------------------------

// Bvh Tree: intersect ray with bvh tree node

Intersection *
intersect_ray_BvhNode(RapidRay *ray, std::vector<MeshTriangle> *mesh_triangle, BvhNode *bvhnode,
                      HitRecord *rec) {
    Intersection *nearest_inter = NULL;
    Intersection *left_inter = NULL;
    Intersection *right_inter = NULL;
    HitRecord lrec, rrec;

    if (mesh_triangle == NULL) {
        return NULL;
    }
    if (bvhnode == NULL) {
        return NULL;
    }
    if (bvhnode->leaf) {
        nearest_inter = intersect_ray_triangle(ray, *(mesh_triangle->at(
                bvhnode->mesh_tri_list_index).V0), *(mesh_triangle->at(
                bvhnode->mesh_tri_list_index).V1), *(mesh_triangle->at(
                bvhnode->mesh_tri_list_index).V2));
        rec->findex = bvhnode->findex;
    } else {
        if (hit_box(ray, bvhnode->box)) {
            if (bvhnode->lchild != NULL) {
                left_inter = intersect_ray_BvhNode(ray, mesh_triangle, bvhnode->lchild, &lrec);
            }

            if (bvhnode->rchild != NULL) {
                right_inter = intersect_ray_BvhNode(ray, mesh_triangle, bvhnode->rchild, &rrec);
            }


            update_nearest_intersection(&left_inter, &nearest_inter);
            update_nearest_intersection(&right_inter, &nearest_inter);
            if (nearest_inter == left_inter) {
                rec->findex = lrec.findex;
            } else if (nearest_inter == right_inter) {
                rec->findex = rrec.findex;
            }
        }
    }

    return nearest_inter;

}

RapidRay *make_ray() {
    RapidRay *r;

    r = (RapidRay *) malloc(sizeof(RapidRay));

    return r;
}

//----------------------------------------------------------------------------

RapidRay *make_ray(QVector3D orig, QVector3D dir) {
    RapidRay *r;

    r = make_ray();
    r->orig = orig;
    r->dir = dir;

    adjust_ray(r);

    return r;
}

//----------------------------------------------------------------------------

void adjust_ray(RapidRay *r) {
    const double max_double = 1e20;
    for (int i = 0; i < 3; i++) {
        if (fabs(r->dir[i]) < SMALL_NUM) {
            if (r->dir[i] >= 0) {
                r->invdir[i] = max_double;
            } else {
                r->invdir[i] = -max_double;
            }
        } else {
            r->invdir[i] = 1.0 / r->dir[i];
        }
        r->sign[i] = r->invdir[i] < 0;
    }
}

//----------------------------------------------------------------------------

Intersection *make_intersection() {
    Intersection *inter;

    inter = (Intersection *) malloc(sizeof(Intersection));

    return inter;
}

//----------------------------------------------------------------------------

// x, y are in pixel coordinates with (0, 0) the upper-left hand corner of the image.
// color variable is result of this function--it carries back info on how to draw the pixel
void trace_ray(RapidRay *ray, std::vector<MeshTriangle> *mesh_triangle, BvhTree *bvhtree_) {
    Intersection *nearest_inter = NULL;
    Intersection *inter = NULL;
    int i;

    // test for intersection with all .obj models
    /*
    for (i = 0; i < model_list.size(); i++) {
    inter = intersect_ray_glm_object(ray, model_list[i], model_predata_list[i]);
    update_nearest_intersection(&inter, &nearest_inter);
    }
    */
    nearest_inter = intersect_ray_BvhTree(ray, mesh_triangle, bvhtree_, i);

    // test for intersection with all spheres
    // free space
    free(nearest_inter);
}


//----------------------------------------------------------------------------
Intersection *intersect_ray_triangle(RapidRay *ray, QVector3D V0, QVector3D V1, QVector3D V2) {
    QVector3D u, v, n;        // triangle std::vectors
    QVector3D w0, w;          // ray std::vectors
    float a, b;           // params to calc ray-plane intersect
    float t;
    QVector3D I;

    // get triangle edge std::vectors and plane normal

    u = V1 - V0;
    v = V2 - V0;
    n = QVector3D::crossProduct(u, v);

    if (n.x() == 0 && n.y() == 0 &&
        n.z() == 0)            // triangle is degenerate; do not deal with this case
        return NULL;

    w0 = ray->orig - V0;
    a = QVector3D::dotProduct(-n, w0);
    b = QVector3D::dotProduct(n, ray->dir);

    if (fabs(b) < SMALL_NUM)                        // ray is parallel to triangle plane
        return NULL;

    // get intersect point of ray with triangle plane

    t = a / b;
    if (t < SMALL_NUM)                   // triangle is behind/too close to ray => no intersect
        return NULL;                 // for a segment, also test if (t > 1.0) => no intersect

    // intersect point of ray and plane

    I = ray->dir * t + ray->orig;


    // is I inside T?

    float uu, uv, vv, wu, wv, D;
    uu = QVector3D::dotProduct(u, u);
    uv = QVector3D::dotProduct(u, v);
    vv = QVector3D::dotProduct(v, v);

    w = I - V0;
    wu = QVector3D::dotProduct(w, u);
    wv = QVector3D::dotProduct(w, v);

    D = uv * uv - uu * vv;

    // get and test parametric (i.e., barycentric) coords

    float p, q;  // were s, t in original code
    p = (uv * wv - vv * wu) / D;
    if (p < 0.0 || p > 1.0)        // I is outside T
        return NULL;
    q = (uv * wu - uu * wv) / D;
    if (q < 0.0 || (p + q) > 1.0)  // I is outside T
        return NULL;

    Intersection *inter;
    inter = new Intersection;
    //inter = make_intersection();
    inter->t = t;
    inter->P.setX(I.x());
    inter->P.setY(I.y());
    inter->P.setZ(I.z());
    return inter;                      // I is in T
}

//----------------------------------------------------------------------------

// inter = current intersection (possibly NULL)
// nearest_inter = nearest intersection so far (also possibly NULL)

void update_nearest_intersection(Intersection **inter, Intersection **nearest_inter) {
    // only do something if this was a hit

    if (*inter) {

        // this is the first object hit

        if (!*nearest_inter)
            *nearest_inter = *inter;

            // this is closer than any previous hit

        else if ((*inter)->t < (*nearest_inter)->t) {
            free(*nearest_inter);
            *nearest_inter = *inter;
        }

            // something else is closer--move along

        else
            free(*inter);
    }
}

//QVector3D RayShootoMesh(QVector3D orig_, QVector3D dir_, Model *mesh_, int &hitFace) {
//    RapidRay *ray_ = make_ray(orig_, dir_);
//    Intersection *inter_ = intersect_ray_BvhTree(ray_, mesh_->mesh_triangles,
//                                                 mesh_->mesh_bvhtree,
//                                                 hitFace);
//    QVector3D inter_point = std::max_VEC3;
//    if (inter_ != NULL) {
//        inter_point = inter_->P;
//        delete inter_;
//    }
//    free(ray_);
//    return inter_point;
//}

QVector3D RayShootoMesh(QVector3D orig_, QVector3D dir_, BvhTree *mesh_bvhtree, std::vector<MeshTriangle> *mesh_triangles, int &hitFace) {
    RapidRay *ray_ = make_ray(orig_, dir_);
    Intersection *inter_ = intersect_ray_BvhTree(ray_,
                                                 mesh_triangles,
                                                 mesh_bvhtree,
                                                 hitFace);
    QVector3D inter_point  = MAX_VEC3;
    if (inter_ != NULL) {
        inter_point = inter_->P;
        delete inter_;
    }
    free(ray_);
    return inter_point;
}
//----------------------------------------------------------------------------
// compare along x-axis

bool comp_x(MeshTriangle &a, MeshTriangle &b) {
    //MeshTriangle * t1 = (MeshTriangle *)a;
    //MeshTriangle * t2 = (MeshTriangle *)b;
    //return t1->center[0] < t2->center[0];
    return a.center[0] < b.center[0];
}

//----------------------------------------------------------------------------

// compare along x-axis

bool comp_y(MeshTriangle &a, MeshTriangle &b) {
    //MeshTriangle * t1 = (MeshTriangle *)a;
    //MeshTriangle * t2 = (MeshTriangle *)b;
    //return t1->center[1] < t2->center[1];
    return a.center[1] < b.center[1];

}

//----------------------------------------------------------------------------

// compare along x-axis

bool comp_z(MeshTriangle &a, MeshTriangle &b) {
    //MeshTriangle * t1 = (MeshTriangle *)a;
    //MeshTriangle * t2 = (MeshTriangle *)b;
    //return t1->center[2] < t2->center[2];
    return a.center[2] < b.center[2];
}
