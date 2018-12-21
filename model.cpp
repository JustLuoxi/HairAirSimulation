#include "model.h"
#include <qmath.h>
#include <QFile>

//bool loadOBJ(const char *path, QVector<QVector3D> &out_positions, QVector<QVector2D> &out_uvs, QVector<QVector3D> &out_normals);
QOpenGLTexture *temp;

Model::Model(){
  core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
  m_matrix.setToIdentity();
  m_matrix_rotate.setToIdentity();
  m_matrix_scale.setToIdentity();
  m_matrix_translate.setToIdentity();

}

bool Model::init(const QString &path){
    // delete previous model
  for(int i = 0; i < objects.size(); ++i){
    core->glDeleteBuffers(1, &objects[i].positionVBO);
    core->glDeleteBuffers(1, &objects[i].uvVBO);
    core->glDeleteBuffers(1, &objects[i].normalVBO);
  }
  objects.clear();
  map_materials.clear();


  bool res = loadOBJ(path);
  if(res){
    this->bindBufferData();
      m_matrix.setToIdentity();
      m_matrix_rotate.setToIdentity();
      m_matrix_scale.setToIdentity();
      m_matrix_translate.setToIdentity();
  }

  return res;
}

void Model::draw(GLboolean isOpenLighting){
  for(int i = 0; i < objects.size(); ++i){
    //
    ResourceManager::getShader("model").use().setVector3f("material.Ka", map_materials[objects[i].matName].Ka);
    ResourceManager::getShader("model").use().setBool("isOpenLighting", isOpenLighting);

    if(!map_materials.empty()){
      core->glActiveTexture(GL_TEXTURE0);
      ResourceManager::getTexture(map_materials[objects[i].matName].name_map_Ka).bind();
      if(isOpenLighting){
        //light
        core->glActiveTexture(GL_TEXTURE1);
        ResourceManager::getTexture(map_materials[objects[i].matName].name_map_Kd).bind();
        ResourceManager::getShader("model").use().setVector3f("material.Kd", map_materials[objects[i].matName].Kd);
        ResourceManager::getShader("model").use().setVector3f("material.Ks", map_materials[objects[i].matName].Ks);
        ResourceManager::getShader("model").use().setFloat("material.shininess", map_materials[objects[i].matName].shininess);
      }
    }

    core->glEnableVertexAttribArray(0);
    core->glBindBuffer(GL_ARRAY_BUFFER, objects[i].positionVBO);
    core->glVertexAttribPointer(
                0,                  // attribute
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
            );

    core->glEnableVertexAttribArray(1);
    core->glBindBuffer(GL_ARRAY_BUFFER, objects[i].uvVBO);
    core->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    core->glEnableVertexAttribArray(2);
    core->glBindBuffer(GL_ARRAY_BUFFER, objects[i].normalVBO);
    core->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  // drawing
    core->glDrawArrays(GL_TRIANGLES, 0, objects[i].positions.size());
  }
}

void Model::processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch)
{
    xoffset *= this->mouseSensitivity/100.0f;
    yoffset *= this->mouseSensitivity/100.0f;

//    this->yaw += xoffset;
//    this->picth += yoffset;

//    if (constraintPitch) {
//      if (this->picth > 89.0f)
//        this->picth = 89.0f;
//      if (this->picth < -89.0f)
//        this->picth = -89.0f;
//    }

//    // just. make the rotation matrix
//    GLfloat yawR = qDegreesToRadians(this->yaw);
//    GLfloat picthR = qDegreesToRadians(this->picth);//degree to radians
//    m_matrix_translate.setToIdentity();
    m_matrix_translate.translate(QVector3D(xoffset,yoffset,0));

    this->UpdateMatrix();
}

void Model::processRotateMovement(GLfloat xoffset, GLfloat yoffset)
{
    xoffset *= this->mouseSensitivity;
    yoffset *= this->mouseSensitivity;

    QMatrix4x4 t_rotate;
    t_rotate.rotate(xoffset,QVector3D(0,1,0));
    t_rotate.rotate(yoffset,QVector3D(-1,0,0));

    m_matrix_rotate =  m_matrix_rotate*t_rotate;

    this->UpdateMatrix();
}

void Model::processMouseScroll(GLfloat yoffset)
{
    this->zoom += yoffset;
    if(this->zoom<=0)
    {
        this->zoom -= yoffset;
        this->zoom += yoffset;
        if(this->zoom<=0.001f)
            this->zoom = 0.001f;
    }

    this->m_matrix_scale.setToIdentity();

    this->m_matrix_scale.scale(zoom);
    this->UpdateMatrix();
}

void Model::UpdateMatrix()
{
    this->m_matrix = this->m_matrix_translate * this->m_matrix_rotate * this->m_matrix_scale;
//    qDebug("m_matrix matrix :\n %f,%f,%f,%f\n", m_matrix(0,0),m_matrix(0,1),m_matrix(0,2),m_matrix(0,3));
//      qDebug("%f,%f,%f,%f\n", m_matrix(1,0),m_matrix(1,1),m_matrix(1,2),m_matrix(1,3));
//      qDebug("%f,%f,%f,%f\n", m_matrix(2,0),m_matrix(2,1),m_matrix(2,2),m_matrix(2,3));
//      qDebug("%f,%f,%f,%f\n", m_matrix(3,0),m_matrix(3,1),m_matrix(3,2),m_matrix(3,3));
}

void Model::SetScale(GLfloat s)
{
    zoom = s;
    this->m_matrix_scale.setToIdentity();
    this->m_matrix_scale.scale(zoom);
    UpdateMatrix();
}

void Model::Reset()
{
    this->m_matrix_rotate.setToIdentity();
    this->m_matrix_translate.setToIdentity();
    this->UpdateMatrix();
}

QMatrix4x4 Model::getModelMatrix()
{
//    qDebug(" ----- m_matrix matrix :\n %f,%f,%f,%f\n", m_matrix(0,0),m_matrix(0,1),m_matrix(0,2),m_matrix(0,3));
//      qDebug("%f,%f,%f,%f\n", m_matrix(1,0),m_matrix(1,1),m_matrix(1,2),m_matrix(1,3));
//      qDebug("%f,%f,%f,%f\n", m_matrix(2,0),m_matrix(2,1),m_matrix(2,2),m_matrix(2,3));
//      qDebug("%f,%f,%f,%f\n", m_matrix(3,0),m_matrix(3,1),m_matrix(3,2),m_matrix(3,3));
    return this->m_matrix;
}

bool Model::loadOBJ(const QString &path){
  QFile file(path);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug()<<"OBJLOADER ERROR::FILE CAN NOT OPEN!";
    file.close();
    return false;
  }

  // set center to (0,0,0)
  center.setX(0);center.setY(0);center.setZ(0);

  QTextStream in(&file);
  QString line;

  QVector<int> positionIndices, uvIndices, normalIndices;
  QVector<QVector3D> temp_positions;
  QVector<QVector2D> temp_uvs;
  QVector<QVector3D> temp_normals;
  QString temp_matName;

  // for hair
  int h_index = 0;
  QVector<int> hair_indices;
  QVector<QVector3D> all_root_ps;
  QVector<QVector3D> all_root_ns;


  while(!in.atEnd()){
    line = in.readLine();
    QStringList list = line.split(" ", QString::SkipEmptyParts);
    if(list.empty())
      continue;
    //qDebug() << list;
    if(list[0] == "mtllib"){
      QString mtl_path = path;
      int tempIndex = path.lastIndexOf("/")+1;
      mtl_path.remove(tempIndex, path.size()-tempIndex);
//      qDebug() << mtl_path;

      /******* 1.2 load .mtl to Material *********/
      QFile mtl_file(mtl_path+list[1]);//.mtl file
      if(!mtl_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"OBJLOADER ERROR::MTL_FILE CAN NOT OPEN!";
        mtl_file.close();
        file.close();
        return false;
      }
      QTextStream mtl_in(&mtl_file);
      QString mtl_line;//

      Material material;
      QString matName;
      while(!mtl_in.atEnd()){
        mtl_line = mtl_in.readLine();
        QStringList mtl_list = mtl_line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if(mtl_list.empty())
          continue;
        if(mtl_list[0] == "newmtl"){
          matName = mtl_list[1];
          map_materials[matName] = material;
        }else if(mtl_list[0] == "Ns"){
          double shininess = mtl_list[1].toDouble();
          map_materials[matName].shininess = shininess;
        }else if(mtl_list[0] == "Ka"){
          double x = mtl_list[1].toDouble();
          double y = mtl_list[2].toDouble();
          double z = mtl_list[3].toDouble();

          QVector3D Ka(x, y, z);
          map_materials[matName].Ka = Ka;
        }else if(mtl_list[0] == "Kd"){
          double x = mtl_list[1].toDouble();
          double y = mtl_list[2].toDouble();
          double z = mtl_list[3].toDouble();

          QVector3D Kd(x, y, z);
          map_materials[matName].Kd = Kd;
        }else if(mtl_list[0] == "Ks"){
          double x = mtl_list[1].toDouble();
          double y = mtl_list[2].toDouble();
          double z = mtl_list[3].toDouble();

          QVector3D Ks(x, y, z);
          map_materials[matName].Ks = Ks;
        }else if(mtl_list[0] == "map_Ka"){
          ResourceManager::loadTexture(mtl_list[1], mtl_path+mtl_list[1]);
          map_materials[matName].name_map_Ka = mtl_list[1];
        }else if(mtl_list[0] == "map_Kd"){
          ResourceManager::loadTexture(mtl_list[1], mtl_path+mtl_list[1]);
          map_materials[matName].name_map_Kd = mtl_list[1];
        }
      }
     /*******   *********/
    }else if(list.size() > 1 && (list[1] == "object"||list[0]=='o')){//look for object
      if(!objects.empty()){
        for(int i=0; i < positionIndices.size(); i++ ){
          int posIndex = positionIndices[i];
          int uvIndex = uvIndices[i];
          int norIndex = normalIndices[i];

          //get the index
//          QVector3D pos = temp_positions[posIndex-1];
//          QVector2D uv = temp_uvs[uvIndex-1];
//          QVector3D nor = temp_normals[norIndex-1];
//qDebug() << uvIndex;
//          //save to objects
//          objects.last().positions.push_back(pos);
//          objects.last().uvs.push_back(uv);
//          objects.last().normals.push_back(nor);
          //load positions to objects
          QVector3D pos = temp_positions[posIndex-1];
          objects.last().positions.push_back(pos);
          all_positions.push_back(pos);
          center += pos;

          QVector3D nor = temp_normals[norIndex-1];
          objects.last().normals.push_back(nor);

          if(uvIndex != 0){
            QVector2D uv = temp_uvs[uvIndex-1];
            objects.last().uvs.push_back(uv);
          }

        }
        objects.last().matName = temp_matName;
        positionIndices.clear();
        uvIndices.clear();
        normalIndices.clear();
      }

      Object object;
      objects.push_back(object);//the first obj
    }else if (list[0] == "v"){
      double x = list[1].toDouble();
      double y = list[2].toDouble();
      double z = list[3].toDouble();

      QVector3D pos;
      pos.setX(x);
      pos.setY(y);
      pos.setZ(z);
      temp_positions.push_back(pos);

      // hair just.11.29
      if(list[4].toDouble()==0.0&&list[5].toDouble()==0.0&&list[6].toDouble()==0.0)
      {
          hair_indices.push_back(h_index);
      }

      ++h_index;
    }else if (list[0] == "vt"){
      double x = list[1].toDouble();
      double y = list[2].toDouble();

      QVector2D uv;
      uv.setX(x);
      uv.setY(y);
      temp_uvs.push_back(uv);
    }else if (list[0] == "vn"){
      double x = list[1].toDouble();
      double y = list[2].toDouble();
      double z = list[3].toDouble();

      QVector3D nor;
      nor.setX(x);
      nor.setY(y);
      nor.setZ(z);
      temp_normals.push_back(nor);
    }else if (list[0] == "usemtl"){
      temp_matName = list[1];
      //qDebug() << list[1];
    }else if (list[0] == "f"){
      if(list.size() > 4){
        qDebug() << "OBJLOADER ERROR::THE LOADER ONLY SUPPORT THE TRIANGLES MESH!" << endl;
        file.close();
        return false;
      }
      for(int i = 1; i < 4; ++i){// face
        QStringList slist = list[i].split("/");
        int posIndex = slist[0].toInt();
        int uvIndex = slist[1].toInt();
        int norIndex = slist[2].toInt();

        positionIndices.push_back(posIndex);
        uvIndices.push_back(uvIndex);
        normalIndices.push_back(norIndex);
        //qDebug() <<posIndex << " " << uvIndex << " " << norIndex;
      }
    }
  }

  //the last object

  for(int i=0; i < positionIndices.size(); i++ ){
    //index
    int posIndex = positionIndices[i];
    int uvIndex = uvIndices[i];
    int norIndex = normalIndices[i];

    // save positions to object
    QVector3D pos = temp_positions[posIndex-1];
    objects.last().positions.push_back(pos);
    all_positions.push_back(pos);
    center += pos;

    QVector3D nor = temp_normals[norIndex-1];
    objects.last().normals.push_back(nor);

    if(uvIndex != 0){
      QVector2D uv = temp_uvs[uvIndex-1];
      objects.last().uvs.push_back(uv);
    }
            //qDebug() <<posIndex << " " << uvIndex << " " << norIndex;
  }

  objects.last().matName = temp_matName;
  center /= positionIndices.size();

  qDebug()<<"positions: "<<temp_positions.size()<<"\n";
  qDebug()<<"normals: "<<temp_normals.size()<<"\n";
    qDebug()<<"uvs: "<<temp_uvs.size()<<"\n";

  file.close();

  //hair
  for(int i=0; i<hair_indices.size();i++)
  {
      all_root_ps.push_back(temp_positions[hair_indices[i]]);
      all_root_ns.push_back(temp_normals[hair_indices[i]]);
  }
  this->m_hair = new Hair(all_root_ps, all_root_ns, this);

  // create bvh tree
  this->createBvhTree(all_positions);

  return true;
}

void Model::bindBufferData(){
  for(int i = 0; i < objects.size(); ++i){
    core->glGenBuffers(1, &objects[i].positionVBO);
    core->glBindBuffer(GL_ARRAY_BUFFER, objects[i].positionVBO);
//    qDebug() << objects[i].positions.size();
    core->glBufferData(GL_ARRAY_BUFFER, objects[i].positions.size() * sizeof(QVector3D), &objects[i].positions[0], GL_STATIC_DRAW);

    core->glGenBuffers(1, &objects[i].uvVBO);
    core->glBindBuffer(GL_ARRAY_BUFFER, objects[i].uvVBO);
    core->glBufferData(GL_ARRAY_BUFFER, objects[i].uvs.size() * sizeof(QVector2D), &objects[i].uvs[0], GL_STATIC_DRAW);

    core->glGenBuffers(1, &objects[i].normalVBO);
    core->glBindBuffer(GL_ARRAY_BUFFER, objects[i].normalVBO);
    core->glBufferData(GL_ARRAY_BUFFER, objects[i].normals.size() * sizeof(QVector3D), &objects[i].normals[0], GL_STATIC_DRAW);
   // qDebug() << i << " : " << objects[i].positionVBO << " " << objects[i].uvVBO << " " << objects[i].normalVBO;
  }
}

void Model::createBvhTree(std::vector<QVector3D> Positions)
{
    if(Positions.size()==0)
    {
        qDebug()<<" DATA ERROR: no Positions! ";
        return;
    }

    this->mesh_triangles = new std::vector<MeshTriangle>;

    for (int i = 0; i < Positions.size(); i += 3) {
        MeshTriangle temp_triangle;
        temp_triangle.V0 = Positions[i];
        temp_triangle.V1 = Positions[i + 1];
        temp_triangle.V2 = Positions[i + 2];
        temp_triangle.findex = i / 3;

        mesh_triangles->push_back(temp_triangle);
    }

    this->mesh_bvhtree = create_new_BvhTree(this->mesh_triangles);

}

// Test a ray(orig_, dir_) whether(doesHit) hit the model(model) or not.
// If hit, which point(hitPoint), which face(hitFace) of the model.
float Model::RayShoot2Model(QVector3D orig_, QVector3D dir_, bool &doesHit, QVector3D &hitPoint, int &hitFace)
{
    hitPoint = RayShootoMesh(orig_, dir_, this, hitFace);
//    hitPoint = RayShootoMesh(orig_, dir_, this->mesh_bvhtree, this->mesh_triangles, hitFace);

    doesHit = (hitPoint != MAX_VEC3);
    float distance = MAX_NUM;
    if (doesHit) {
        distance = (orig_ - hitPoint).length();
//        int v0 = model->Indices[hitFace * 3 + 0];
//        int v1 = model->Indices[hitFace * 3 + 1];
//        int v2 = model->Indices[hitFace * 3 + 2];
//        QVector3D p1 = model->Positions[v0];
//        QVector3D p2 = model->Positions[v1];
//        QVector3D p3 = model->Positions[v2];

//        QVector3D dir_1_2 = (p2 - p1).normalized();
//        QVector3D dir_1_3 = (p3 - p1).normalized();
//        QVector3D hitface_normal = (QVector3D::crossProduct(dir_1_2, dir_1_3)).normalize();

    }

    return distance;

}

// ****************** Ray Tracing **************************************

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
        center_ += (*titer).V0;
        center_ += (*titer).V1;
        center_ += (*titer).V2;
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
        box.bounds[0][i] = std::min((tri.V0)[i], (tri.V1)[i]);
        box.bounds[0][i] = std::min(box.bounds[0][i], (tri.V2)[i]);

        box.bounds[1][i] = std::max((tri.V0)[i], (tri.V1)[i]);
        box.bounds[1][i] = std::max(box.bounds[1][i], (tri.V2)[i]);
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
        try{

        nearest_inter = intersect_ray_triangle(
                    ray,
                    mesh_triangle->at(bvhnode->mesh_tri_list_index).V0,
                    mesh_triangle->at(bvhnode->mesh_tri_list_index).V1,
                    mesh_triangle->at(bvhnode->mesh_tri_list_index).V2
                    );
        rec->findex = bvhnode->findex;
        }
        catch(std::exception &e)
        {
            qDebug()<<e.what();
            qDebug()<<bvhnode->mesh_tri_list_index;

            qDebug()<<(mesh_triangle->at(bvhnode->mesh_tri_list_index)).V0;
            qDebug()<<mesh_triangle->at(bvhnode->mesh_tri_list_index).V1;
            qDebug()<<mesh_triangle->at(bvhnode->mesh_tri_list_index).V2;

        }
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

QVector3D RayShootoMesh(QVector3D orig_, QVector3D dir_, Model *mesh_, int &hitFace) {
    RapidRay *ray_ = make_ray(orig_, dir_);
//    qDebug()<<(*(mesh_->mesh_triangles->at(0).V0)).x();
//    qDebug()<<(*(mesh_->mesh_triangles->at(0).V1)).y();
//    qDebug()<<(*(mesh_->mesh_triangles->at(0).V2)).z();
//    qDebug()<<mesh_->mesh_triangles->at(0).center.x();
    Intersection *inter_ = intersect_ray_BvhTree(ray_, mesh_->mesh_triangles,
                                                 mesh_->mesh_bvhtree,
                                                 hitFace);
    QVector3D inter_point = MAX_VEC3;
    if (inter_ != NULL) {
        inter_point = inter_->P;
        delete inter_;
    }
    free(ray_);
    return inter_point;
}

//QVector3D RayShootoMesh(QVector3D orig_, QVector3D dir_, BvhTree *mesh_bvhtree, std::vector<MeshTriangle> *mesh_triangles, int &hitFace) {
//    RapidRay *ray_ = make_ray(orig_, dir_);
//    Intersection *inter_ = intersect_ray_BvhTree(ray_,
//                                                 mesh_triangles,
//                                                 mesh_bvhtree,
//                                                 hitFace);
//    QVector3D inter_point  = MAX_VEC3;
//    if (inter_ != NULL) {
//        inter_point = inter_->P;
//        delete inter_;
//    }
//    free(ray_);
//    return inter_point;
//}
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


