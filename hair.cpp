#include "hair.h"
#include "model.h"
#include "oglmanager.h"
#include <math.h>

extern Air *air;
bool air2hair_flag = true;

Hair::Hair()
{

}

Hair::Hair(QVector<QVector3D> _all_root_ps, QVector<QVector3D> _all_root_ns, Model *model)
{
    this->head_model = model;
    bool res = this->Init(_all_root_ps, _all_root_ns);
    if(!res)
    {
        qDebug()<<"ERROR:: fail to initialize hair  "<<"\n";
    }
}

Hair::~Hair()
{

}

bool Hair::Init(QVector<QVector3D> _all_root_ps, QVector<QVector3D> _all_root_ns)
{
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();

    // initialize hairlines
    int range_max = _all_root_ps.size();
    for(int i=0;i<GUIDEHAIRNUM_JUST;i++)
    {
        int id = std::rand();
        id %= range_max;
        QVector3D x = _all_root_ps[id];
        QVector3D n = _all_root_ns[id];

        Hairline hair(x,n,this->head_model);
        this->m_guide_hairlines.push_back(hair);

    }

    this->_bindBufferData();

    return true;
}

// draw hair in every frame
void Hair::Draw(GLboolean isOpenLighting)
{

    for(int i=0; i<this->m_guide_hairlines.size();i++)
    {
        ResourceManager::getShader("hair").use().setBool("isOpenLighting", isOpenLighting);
//        ResourceManager::getShader("hair").use().setVector3f("material.Ka", QVector3D(0,0,0));
//        ResourceManager::getShader("hair").use().setVector3f("material.Kd", QVector3D(0.58f,0.58f,0.58f));
//        ResourceManager::getShader("hair").use().setVector3f("material.Ks", QVector3D(0.1f,0.1f,0.1f));
//        ResourceManager::getShader("hair").use().setFloat("material.shininess", 20.0);

        core->glEnableVertexAttribArray(0);
        core->glBindBuffer(GL_ARRAY_BUFFER, m_guide_hairlines[i].VBO);
        core->glVertexAttribPointer(
                    0,                  // attribute
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
                    );

        core->glDrawArrays(GL_LINES,0,m_guide_hairlines[i].render_ps.size());
    }
}

int frame = 0;
void Hair::Update(QMatrix4x4 model)
{
    qDebug()<<"frame: " << frame++<<"\n";
    for(int i=0; i<this->m_guide_hairlines.size();i++)
    {
        this->m_guide_hairlines[i].update(model);
    }

    this->_bindBufferData();

    // update air velocity
    air->HairInfluenceAir();
}

void Hair::_selectGuideHairline()
{
    //    this->guide_ps.clear();
    //    int range_max = this->all_root_ps.size();
    //    for(int i=0;i<this->guide_num;i++)
    //    {
    //        int id = std::rand();
    //        id %= range_max;
    //        this->guide_ps.push_back(this->all_root_ps[id]);
    //        this->guide_ns.push_back(this->all_root_ns[id]);


    //    }
}

void Hair::_bindBufferData()
{
    for(int i =0 ; i<this->m_guide_hairlines.size();i++)
    {
        core->glDeleteBuffers(1, &(m_guide_hairlines[i].VBO));
        core->glDeleteBuffers(1, &(m_guide_hairlines[i].nVBO));
    }

    for(int i =0 ; i<this->m_guide_hairlines.size();i++)
    {
        core->glGenBuffers(1,&(m_guide_hairlines[i].VBO));
        core->glBindBuffer(GL_ARRAY_BUFFER,m_guide_hairlines[i].VBO);
        core->glBufferData(GL_ARRAY_BUFFER,m_guide_hairlines[i].render_ps.size()* sizeof(QVector3D), &m_guide_hairlines[i].render_ps[0], GL_STATIC_DRAW);

        core->glGenBuffers(1,&(m_guide_hairlines[i].nVBO));
        core->glBindBuffer(GL_ARRAY_BUFFER,m_guide_hairlines[i].nVBO);
        core->glBufferData(GL_ARRAY_BUFFER,m_guide_hairlines[i].n_render_ps.size()* sizeof(QVector3D), &m_guide_hairlines[i].n_render_ps[0], GL_STATIC_DRAW);

    }

}


Hairline::Hairline()
{

}

Hairline::Hairline(QVector3D root_p, QVector3D n, Model *model)
{
    this->m_root_p = root_p;
    this->m_n = n.normalized();
    this->m_matrix.setToIdentity();
    this->head_model = model;

    n = n.normalized();
    this->m_Springs = new QVector<Spring*>;
    QVector3D last,x,next;
    for(int i=0;i<SPRING_NUM_JUST;i++)
    {
        x = root_p + i*SPRING_LENGTH_JUST*n;
        this->m_ps.push_back(x);
    }
    this->m_ps.push_back(x + SPRING_LENGTH_JUST*n );

    Spring * spring;
    for(int i=0; i<SPRING_NUM_JUST;i++)
    {
        if(i==0)
        {
            spring = new Spring(NULL,&this->m_ps[i],&this->m_ps[i+1],root_p,model);
        }
        else
        {
            spring = new Spring(&this->m_ps[i-1],&this->m_ps[i],&this->m_ps[i+1],root_p,model);
        }
        this->m_Springs->push_back(spring);
    }
    this->m_Springs->push_back(new Spring(&this->m_ps[SPRING_NUM_JUST-1],&this->m_ps[SPRING_NUM_JUST],NULL,root_p,model));

    this->updateRenderData();
}

Hairline::~Hairline()
{

}

void Hairline::update(QMatrix4x4 model)
{
    this->m_matrix = model;
    for(int i=0;i<this->m_Springs->size();i++)
    {
        Spring *spring = this->m_Springs->at(i);
        spring->update(model);
        m_ps[i] = * spring->m_x;
    }
    this->updateRenderData();
}

void Hairline::updateRenderData()
{
    this->render_ps.clear();
    this->n_render_ps.clear();
    QVector<QVector3D> temp_vector(this->m_ps);

    // B-spline
    temp_vector = this->_bSpline(this->m_ps,15);

    // Archimedean spiral algorithm
    this->_archimedeanspiral(temp_vector);

}

QVector3D Hairline::_linearInterpolate(QVector3D s, QVector3D e, GLfloat t)
{
    if(t<0) t = 0;
    if(t>1) t = 1;
    return s+(e-s)*t;
}

void Hairline::_archimedeanspiral(QVector<QVector3D> ps)
{
    for(int i=0;i<ps.size()-2;i++)
    {
        this->render_ps.push_back(ps[i]);
        this->render_ps.push_back(ps[i+1]);
        this->n_render_ps.push_back(ps[i+1]);
        this->n_render_ps.push_back(ps[i+2]);
    }

    int loops = 10;
    GLfloat b = 0.005f;
    GLfloat a = 0.005f;
    GLfloat theta = 0;
    GLfloat step = 0.5f;
    GLfloat r=0;
    GLfloat u=0,v=0;
    QVector3D x,y;
    QVector3D offset(0,0,0);
    QVector3D normal = QVector3D(this->m_matrix * QVector4D(this->m_n,0)).normalized();
    QVector3D root = QVector3D(this->m_matrix * QVector4D(this->m_root_p,1));
    GLfloat _A,_B,_C,_D; // plane function
    _A = normal.x();
    _B = normal.y();
    _C = normal.z();
    _D = - QVector3D::dotProduct(normal,root);

    if(_A!=0)
        x = QVector3D(-_D/_A,0,0).normalized();
    else if(_B!=0)
        x = QVector3D(0,-_D/_B,0).normalized();
    else
        x = QVector3D(0,0,-_D/_C).normalized();

    y = QVector3D::crossProduct(normal,x).normalized();

    while(theta < 2 *M_PI_4*loops)
    {
        r = a+b*theta;
        u = (a+b*theta)*std::cos(theta);
        v = (a+b*theta)*std::sin(theta);
        offset = u*x + v*y;
        for(int i=0;i<ps.size()-2;i++)
        {
            this->render_ps.push_back(ps[i]+offset);
            this->render_ps.push_back(ps[i+1]+offset);
            this->n_render_ps.push_back(ps[i+1]+offset);
            this->n_render_ps.push_back(ps[i+2]+offset);
        }
        theta += step;
    }
}

// num -- there are NUM points in every gap
QVector<QVector3D> Hairline::_bSpline(QVector<QVector3D> ps, int num)
{
    if(num <=3 || ps.size()<=3)
    {
        return ps;
    }

    QVector<QVector3D> results;
    results.push_back(ps[0]);
    GLfloat t = 0.0f;
    for(int j=0;j<ps.size()-2;j+=2)
    {
        t = 0.0f;
        for(int i=0; i<num;i++)
        {
            t+=1.0f/num;
            QVector3D p1 = _linearInterpolate(ps[j],ps[j+1],t);
            QVector3D p2 = _linearInterpolate(ps[j+1],ps[j+2],t);
            QVector3D pt = _linearInterpolate(p1,p2,t);
            results.push_back(pt);
        }
    }
    return results;
}

GLfloat Spring::dis2Head(QVector3D p,QVector3D a, QVector3D b)
{
    // a--head_down, b--head_up
    // Given segment ab and point p, computes closest point d on ab.
    QVector3D ab = b - a;
    GLfloat t = QVector3D::dotProduct(p-a,ab)/ab.length();
    if(t<0.0f) t = 0.0f;
    if(t>1.0f) t = 1.0f;
    QVector3D d = a + t*ab;
    return (p-d).length();
}

QVector3D Spring::checkSpringLength(QVector3D p)
{
    GLfloat deltal = (p-*(this->m_last)).length();
    // length constrain
    //    if(deltal<=1.1f*SPRING_LENGTH_JUST && deltal>=0.9f*SPRING_LENGTH_JUST)
    //    {
    //       * this->m_x = p;
    //    }
    if(deltal<0.9f*SPRING_LENGTH_JUST) // too short
    {
        p = (p-*(this->m_last)).normalized() * 0.9f*SPRING_LENGTH_JUST + *this->m_last;
    }
    else if(deltal>1.1f*SPRING_LENGTH_JUST)// too long
    {
        p = (p-*(this->m_last)).normalized() * 1.1f*SPRING_LENGTH_JUST + *this->m_last;
    }
    return p;
}

QVector3D Spring::checkSpringHeadCollison(QVector3D p, QVector3D head_down, QVector3D head_up)
{
    // head collision constrain
    GLfloat dis_length = this->dis2Head(p, head_down, head_up);
    QVector3D force(0,0,0);
    QVector3D dire = p - (head_down + head_up)/2;
    dire.normalize();
    GLfloat scale=0;
    if(dis_length<HEAD_W) // detect collision
    {
        scale = HEAD_W - dis_length;
        scale = this->head_k*scale;
    }
    force = dire*scale;

    return force;
}


Spring::Spring()
{

}

Spring::Spring(QVector3D *_last, QVector3D *_x, QVector3D *_next,QVector3D _root, Model *model)
{
    this->m_last = _last;
    this->m_x = _x;
    this->m_next = _next;
    this->head_model = model;

    this->m_v = QVector3D(0,0,0);
    this->m_delt = 1.0f/FPS;
    this->m = PARTICLE_MASS_JUST;
    this->k = SPRING_K_JUST;
    this->m_root_p = _root;
    this->old_model_matrix.setToIdentity();
}

Spring::~Spring()
{

}

void Spring::update(QMatrix4x4 model)
{

    if(model!=this->old_model_matrix)
    {
        this->balance_flag = false;
        this->old_model_matrix = model;
    }
    if(this->balance_flag)
    {
        this->m_v = QVector3D(0,0,0);
        return;
    }

    if(this->m_last == NULL)
    {
        QVector3D p = this->m_root_p;
        p = QVector3D(model* QVector4D(p,1));
        *this->m_x = p;
        return;
    }

    QVector3D f_sl,f_sn(0,0,0), f_d,f_g;
    QVector3D dis, dire;
    GLfloat scale = 0;
    GLfloat length = 0;
    GLfloat deltal = 0;
    QVector3D force(0,0,0),a(0,0,0);
    QVector3D v,p;
    p = * this->m_x;
    v=this->m_v;

    // force of last Spring
    dis = * this->m_last - * this->m_x;
    length = dis.length();
    dire = dis.normalized();
    deltal = length - SPRING_LENGTH_JUST;
    scale = this->k * deltal;
    if(std::abs(deltal) >= 0.1f*SPRING_LENGTH_JUST)
    {
        scale *= 10;
    }
    f_sl = dire * scale;
//         force of next Spring
    if(this->m_next!=NULL)
    {
        dis = * this->m_next - * this->m_x;
        length = dis.length();
        dire = dis.normalized();
        deltal = length - SPRING_LENGTH_JUST;
        scale = this->k * deltal;
        if(std::abs(deltal) >= 0.1f*SPRING_LENGTH_JUST)
        {
            scale *= 10;
        }
        f_sn = dire * scale;
    }
    else
    {
        f_sn = QVector3D(0,0,0);
    }
    // force of damping
    f_d = -1 * this->m_v * DAMPING_K_JUST;
    if( this->m_v.length() > 15)
    {
        f_d = f_d * 10;
    }
    // force of gravity
    f_g = this->m * this->g * QVector3D(0,-1,0);

    // force of air flow
    QVector3D f_air(0,0,0);
    if(air2hair_flag)
        f_air =  AIR2HAIRSCALE * air->GetVelocity(p.x(),p.y(),p.z());

    force = f_sl + f_sn + f_d + f_g + f_air;
    a = force / this->m;

    // compute velocity
    v = this->m_v;
    v = a*this->m_delt + v;

    // update position
    p = v * this->m_delt + p;
    // check length constrain
    p = this->checkSpringLength(p);



////     check the head-hair collision -- 1
//    this->head_down = model * HEAD_DOWN;
//    this->head_Up = model * HEAD_UP;
//    QVector3D f_c  = this->checkSpringHeadCollison(p,this->head_down,this->head_Up);
//    if(f_c!=QVector3D(0,0,0)) // detect collision
//    {
//            a = f_c / this->m;
//            v = this->m_v;
//            v = a*this->m_delt + v;
////            p = * this->m_x;
//            p = v * this->m_delt + p;
//            p = this->checkSpringLength(p);
//    }

////     check the head-hair collision -- 2
    QVector3D closestPoint;
    bool collision = this->detectCollision(p,closestPoint);
    if(collision)
    {
        QVector3D head_center = this->old_model_matrix * this->head_model->center;
        QVector3D f_c;
        GLfloat delta_dis =(closestPoint-head_center).length() - (p-head_center).length();
//        GLfloat old_delta_dis = 1e6;

        f_c = (closestPoint - head_center).normalized() * this->head_k*delta_dis;
        a = f_c / this->m;
        v = this->m_v;
        v = a*this->m_delt + v;
//        p = * this->m_x;
        p = v * this->m_delt + p;
        p = this->checkSpringLength(p);

    }

    this->m_v = v;
    * this->m_x = p;
}

bool Spring::detectCollision(QVector3D p, QVector3D &closestPoint)
{
    int hitFace;
    bool doesHit;
    QVector3D head_center = this->head_model->center;
    p = this->old_model_matrix.inverted()* p;
    GLfloat hit_distance = this->head_model->RayShoot2Model(head_center,(p-head_center).normalized(),doesHit,closestPoint,hitFace);
    closestPoint = this->old_model_matrix * closestPoint;
    GLfloat p_center_distance = (p-head_center).length();
    if(hit_distance>p_center_distance) // collision
    {
        return true;
    }
    else
    {
        return false;
    }
}
