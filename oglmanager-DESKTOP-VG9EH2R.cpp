#include "oglmanager.h"
#include <QPalette>
#include "model.h"
#include "light.h"
#include "resourcemanager.h"
#include <QDebug>
#include <mainwindow.h>
//#include "quaternion.h"



const QVector3D CAMERA_POSITION(0.0f, 0.0f, 3.0f);
const QVector3D LIGHT_POSITION(0.0f, 3.0f, 3.0f);

const int OGLMANAGER_WIDTH = 600;
const int OGLMANAGER_HEIGHT = 600;

Light *light;
Model *objModel;

OGLManager::OGLManager(QWidget *parent) : QOpenGLWidget(parent){
    this->setGeometry(10, 20, OGLMANAGER_WIDTH, OGLMANAGER_HEIGHT);

    this->frame_matrixs.resize(frame_num);
    this->temp_rotate_matrixs.resize(frame_num);
    this->temp_translate_matrixs.resize(frame_num);
    this->temp_scale_matrixs.resize(frame_num);
}

OGLManager::~OGLManager(){
    if(camera)
        delete camera;
}

void OGLManager::handleKeyPressEvent(QKeyEvent *event){
    GLuint key = event->key();
    if(key >= 0 && key <= 1024)
        this->keys[key] = GL_TRUE;

}

void OGLManager::handleKeyReleaseEvent(QKeyEvent *event){
    GLuint key = event->key();
    if(key >= 0 && key <= 1024)
        this->keys[key] = GL_FALSE;
    if(event->key()==Qt::Key_Q)
        objModel->Reset();
}

void OGLManager::changeObjModel(QString fileName){
    objModel->init(fileName);
    key_frame_indices.clear();
    temp_translate_matrixs.clear();
    temp_rotate_matrixs.clear();
    temp_scale_matrixs.clear();
    this->frame_matrixs.resize(frame_num);
    this->temp_rotate_matrixs.resize(frame_num);
    this->temp_translate_matrixs.resize(frame_num);
    this->temp_scale_matrixs.resize(frame_num);
    AddKeyFrame(0);
    this->window->setdoubleSpinBoxValue(ZOOM_JUST);
}

void OGLManager::setModelScale()
{
    QMatrix4x4 scaling;
    scaling.scale(modelScaling);
    objModel->SetScale(modelScaling);
    ResourceManager::getShader("model").use().setMatrix4f("model", objModel->getModelMatrix());
}

void OGLManager::AddKeyFrame(int frame)
{
    temp_translate_matrixs[frame] = objModel->m_matrix_translate;
    temp_rotate_matrixs[frame] = objModel->m_matrix_rotate;
    key_frame_indices.push_back(frame);
    qDebug("add keyfram : %d \n",frame);
    qDebug("temp_translate_matrixs size : %d \n",temp_translate_matrixs.size());
}

void OGLManager::RuntheAnimation()
{
    run_animation_flag = true;
    has_run_animation = false;
    current_frame = 0;
    // sort indices and erase the duplicate nodes
    std::sort(key_frame_indices.begin(),key_frame_indices.end());
    key_frame_indices.erase(std::unique(key_frame_indices.begin(),key_frame_indices.end()),key_frame_indices.end());

    // just. TODO: interpolation
    int a_frame,b_frame;
    for(int i=0;i<key_frame_indices.size()-1;i++)
    {
        qDebug("keyfram : %d \n",key_frame_indices[i]);
        // find the gap
        a_frame = key_frame_indices[i];
        b_frame = key_frame_indices[i+1];

        int t = b_frame+1-a_frame;
        QVector3D a_translate = _translationMat2Vec3(temp_translate_matrixs[a_frame]);
        QVector3D b_translate = _translationMat2Vec3(temp_translate_matrixs[b_frame]);;
        QVector3D t_vec3 = (b_translate - a_translate)/t;
        Quaternion a_quat,b_quat;
        a_quat = Matrix4D2Quaternion(temp_rotate_matrixs[a_frame]);
        b_quat = Matrix4D2Quaternion(temp_rotate_matrixs[b_frame]);
        QMatrix4x4 interp_mat;
        for(int j=a_frame;j<=b_frame;j++)
        {
            // 1. translation interpolation -- linear
            temp_translate_matrixs[j] = _translationVec32Mat(a_translate+(j-a_frame)*t_vec3);

            // 2. rotation interpolation-- linear
            Quaternion inter_quat = Slerp(a_quat,b_quat,1.0*(j-a_frame)/t);
            temp_rotate_matrixs[j] = RotationMatrix4D(inter_quat);
        }

    }

    qDebug("keyfram : %d \n",b_frame);
    // for imcomplete interpolation
    if(b_frame!=frame_num-1){
        a_frame = b_frame;
        b_frame = frame_num-1;
        for(int i=a_frame;i<=b_frame;i++)
        {
            temp_translate_matrixs[i]=temp_translate_matrixs[a_frame];
            temp_rotate_matrixs[i]=temp_rotate_matrixs[a_frame];
        }

    }

    // temp => frame_matrix
    for(int i=0;i<frame_num;i++)
    {
        frame_matrixs[i] = temp_translate_matrixs[i]*temp_rotate_matrixs[i];
    }


}

void OGLManager::SetFrame(int frame)
{
    if(!has_run_animation) return;
    current_frame = frame;
    objModel->getModelMatrix() = frame_matrixs[frame];
    //      qDebug("set frame matrix :\n %f,%f,%f,%f\n", frame_matrixs[frame](0,0),frame_matrixs[frame](0,1),frame_matrixs[frame](0,2),frame_matrixs[frame](0,3));
    //      qDebug("%f,%f,%f,%f\n", frame_matrixs[frame](1,0),frame_matrixs[frame](1,1),frame_matrixs[frame](1,2),frame_matrixs[frame](1,3));
    //      qDebug("%f,%f,%f,%f\n", frame_matrixs[frame](2,0),frame_matrixs[frame](2,1),frame_matrixs[frame](2,2),frame_matrixs[frame](2,3));
    //      qDebug("%f,%f,%f,%f\n", frame_matrixs[frame](3,0),frame_matrixs[frame](3,1),frame_matrixs[frame](3,2),frame_matrixs[frame](3,3));
}

void OGLManager::initializeGL(){
    /*********** OGL ***********/
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    core->glEnable(GL_DEPTH_TEST);

    /*********** initialization  *************/
    isOpenLighting = GL_FALSE;
    isLineMode = GL_FALSE;
    modelScaling = 0.001f;

    /*********** interaction  *************/
    for(GLuint i = 0; i != 1024; ++i)
        keys[i] = GL_FALSE;

    deltaTime = 0.0f;
    lastFrame = 0.0f;

    isFirstMouse = GL_TRUE;
    isLeftMousePress = GL_FALSE;
    isRightMousePress = GL_FALSE;
    lastX = width() / 2.0f;
    lastY = height() / 2.0f;

    time.start();

    /************ camera ***********/
    camera = new Camera(CAMERA_POSITION);

    /*********** light  *************/
    light = new Light();
    light->init();

    /************ obj ***********/
    objModel = new Model();
//        objModel->init(":/models/res/models/biwutai/biwutai.obj");
    objModel->init(":/models/res/models/head/Pasha_guard_head_just.obj");
    AddKeyFrame(0);
    /************ shader ***********/
    ResourceManager::loadShader("model", ":/shaders/res/shaders/model.vert", ":/shaders/res/shaders/model.frag");
    ResourceManager::loadShader("light", ":/shaders/res/shaders/light.vert", ":/shaders/res/shaders/light.frag");

    ResourceManager::getShader("model").use().setInteger("material.ambientMap", 0);
    ResourceManager::getShader("model").use().setInteger("material.diffuseMap", 1);
    ResourceManager::getShader("model").use().setVector3f("light.position", LIGHT_POSITION);

    QMatrix4x4 model_;
    model_.scale(0.01f);
    ResourceManager::getShader("model").use().setMatrix4f("model", model_);
    QMatrix4x4 model;
    model.translate(LIGHT_POSITION);
    model.scale(0.1f);
    ResourceManager::getShader("light").use().setMatrix4f("model", model);

    /************ bg ***********/
    core->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    core->glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
}

void OGLManager::resizeGL(int w, int h){
    core->glViewport(0, 0, w, h);
}

// update in every frame
void OGLManager::paintGL(){
    /*********** time  ***************/
    GLfloat currentFrame = (GLfloat)time.elapsed()/1000;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    //  qDebug("fps: %f",1/deltaTime);

    this->processInput(deltaTime);
    this->updateGL();

    /*********  light ************/
    ResourceManager::getShader("light").use();
    light->drawLight();

    /*********  draw model ************/
    ResourceManager::getShader("model").use();
    objModel->draw(this->isOpenLighting);

}

void OGLManager::processInput(GLfloat dt){
    if (keys[Qt::Key_W])
        camera->processKeyboard(FORWARD, dt);
    if (keys[Qt::Key_S])
        camera->processKeyboard(BACKWARD, dt);
    if (keys[Qt::Key_A])
        camera->processKeyboard(LEFT, dt);
    if (keys[Qt::Key_D])
        camera->processKeyboard(RIGHT, dt);
    if (keys[Qt::Key_E])
        camera->processKeyboard(UP, dt);
    if (keys[Qt::Key_Q])
        camera->processKeyboard(DOWN, dt);

}

void OGLManager::updateGL(){
    if(this->isLineMode)
        core->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        core->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    QMatrix4x4 projection, view, model;
    projection.perspective(camera->zoom, (GLfloat)width()/(GLfloat)height(), 0.1f, 200.f);
    view = camera->getViewMatrix();
    model = objModel->getModelMatrix();

    if(!run_animation_flag){ // NORMAL STATUS

        if(has_run_animation)
            model = frame_matrixs[current_frame];
        ResourceManager::getShader("light").use().setMatrix4f("projection", projection);
        ResourceManager::getShader("light").use().setMatrix4f("view", view);

        ResourceManager::getShader("model").use().setMatrix4f("projection", projection);
        ResourceManager::getShader("model").use().setMatrix4f("view", view);
        ResourceManager::getShader("model").use().setMatrix4f("model", model);
        ResourceManager::getShader("model").use().setVector3f("viewPos", camera->position);
    }
    else //
    {
        ResourceManager::getShader("light").use().setMatrix4f("projection", projection);
        ResourceManager::getShader("light").use().setMatrix4f("view", view);

        ResourceManager::getShader("model").use().setMatrix4f("projection", projection);
        ResourceManager::getShader("model").use().setMatrix4f("view", view);
        ResourceManager::getShader("model").use().setMatrix4f("model", this->frame_matrixs[current_frame]);
        ResourceManager::getShader("model").use().setVector3f("viewPos", camera->position);

        m_deltatime += deltaTime;
        if(m_deltatime>1.0f/m_fps)
        {
            this->window->setSliderValue(current_frame);
            m_deltatime = 0.0f;
            current_frame ++;
            if(current_frame>=frame_num)
            {
                run_animation_flag = false;
                has_run_animation = true;
                current_frame = frame_num-1;
                objModel->getModelMatrix() = frame_matrixs[current_frame];
            }
        }

    }

    //  qDebug("model matrix :\n %f,%f,%f,%f\n", model(0,0),model(0,1),model(0,2),model(0,3));
    //  qDebug("%f,%f,%f,%f\n", model(1,0),model(1,1),model(1,2),model(1,3));
    //  qDebug("%f,%f,%f,%f\n", model(2,0),model(2,1),model(2,2),model(2,3));
    //  qDebug("%f,%f,%f,%f\n", model(3,0),model(3,1),model(3,2),model(3,3));

    //  QMatrix4x4 scaling;
    //  scaling.scale(modelScaling);
    //  ResourceManager::getShader("model").use().setMatrix4f("model", scaling);
    //  ResourceManager::getShader("model").use().setMatrix4f("model", model);
}

QVector3D OGLManager::_translationMat2Vec3(QMatrix4x4 m)
{
    QVector3D result = QVector3D(m(0,3),m(1,3),m(2,3));
    return  result;
}

QMatrix4x4 OGLManager::_translationVec32Mat(QVector3D v)
{
    QMatrix4x4 m;
    m.setToIdentity();
    m.translate(v);
    return  m;
}

void OGLManager::mouseMoveEvent(QMouseEvent *event){
    GLint xpos = event->pos().x();
    GLint ypos = event->pos().y();
    if(isLeftMousePress){
        if (isFirstMouse){
            lastX = xpos;
            lastY = ypos;
            isFirstMouse = GL_FALSE;
        }

        GLint xoffset = xpos - lastX;
        GLint yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;
        //    camera->processMouseMovement(xoffset, yoffset);
        objModel->processMouseMovement(xoffset,yoffset);
    }
    if(isRightMousePress)
    {
        if (isFirstMouse){
            lastX = xpos;
            lastY = ypos;
            isFirstMouse = GL_FALSE;
        }

        GLint xoffset = xpos - lastX;
        GLint yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        objModel->processRotateMovement(xoffset,yoffset);
    }
}

void OGLManager::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton)
        isLeftMousePress = GL_TRUE;
    if(event->button()==Qt::RightButton)
        isRightMousePress = GL_TRUE;
}

void OGLManager::mouseReleaseEvent(QMouseEvent *event){
    isFirstMouse = GL_TRUE;
    if(event->button() == Qt::LeftButton){
        isLeftMousePress = GL_FALSE;
    }
    if(event->button()==Qt::RightButton)
        isRightMousePress = GL_FALSE;

}

void OGLManager::wheelEvent(QWheelEvent *event){
    QPoint offset = event->angleDelta();
    GLfloat t = event->delta();
    t/=12000.0f;

    //    qDebug("angleDelta: x- %d , y - %d",offset.x(),offset.y());
    //  camera->processMouseScroll(offset.y()/20.0f);
    objModel->processMouseScroll(t);
    qDebug("offset: %f",t);
    this->window->setdoubleSpinBoxValue(objModel->zoom);
}

// The slerp moves a point in space from one position to another spherically using
// the general formula p' = p1 + t(p2 - p1) where p' is the current position,
// p1 is the original position, p2 is the final position, and time is represented by t
// TODO: normalize
Quaternion Slerp(Quaternion a, Quaternion b, double t)
{
    Quaternion q = Quaternion();

    // Calculate angle between them
    double cosHalfTheta = (a.w * b.w) + (a.x * b.x) + (a.y * b.y) + (a.z * b.z);

    if (abs(cosHalfTheta) >= 1.0)
    {
        q.w = a.w;
        q.x = a.x;
        q.y = a.y;
        q.z = a.z;

        return q;
    }

    // Calculate temporary values
    double halfTheta = acos(cosHalfTheta);
    double sinHalfTheta = sqrt(1.0f - cosHalfTheta * cosHalfTheta);

    // if theta = 180 degrees then result is not fully defined
    // we could rotate around any axis normal to a or b
    if (fabs(sinHalfTheta) < 0.001)
    {
        q.w = (a.w * 0.5 + b.w * 0.5);
        q.x = (a.x * 0.5 + b.x * 0.5);
        q.y = (a.y * 0.5 + b.y * 0.5);
        q.z = (a.z * 0.5 + b.z * 0.5);

        return q;
    }

    double ratioA = sin((1 - t) * halfTheta) / sinHalfTheta;
    double ratioB = sin(t * halfTheta) / sinHalfTheta;

    // Calculate Quaternion
    q.w = (a.w * ratioA + b.w * ratioB);
    q.x = (a.x * ratioA + b.x * ratioB);
    q.y = (a.y * ratioA + b.y * ratioB);
    q.z = (a.z * ratioA + b.z * ratioB);

    return q;
}

QMatrix4x4 RotationMatrix4D(Quaternion q)
{
    float n00 = 1 - (2 * q.y*q.y) - (2 * q.z*q.z);
    float n01 = 2 * ((q.x * q.y) + (q.w * q.z));
    float n02 = 2 * ((q.x * q.z) - (q.w * q.y));
    float n10 = 2 * ((q.x * q.y) - (q.w * q.z));
    float n11 = 1 - (2 * q.x*q.x) - (2 * q.z*q.z);
    float n12 = 2 * ((q.y * q.z) + (q.w * q.x));
    float n20 = 2 * ((q.x * q.z) + (q.w * q.y));
    float n21 = 2 * ((q.y * q.z) - (q.w * q.x));
    float n22 = 1 - (2 * q.x*q.x) - (2 * q.y*q.y);

    QMatrix4x4 result;
    result(0,0)=n00; result(0,1)=n01; result(0,2)=n02;
    result(1,0)=n10; result(1,1)=n11; result(1,2)=n12;
    result(2,0)=n20; result(2,1)=n21; result(2,2)=n22;
    result(3,0)=0.0f;result(3,1)=0.0f;result(3,2)=0.0f;result(3,3)=1.0f;
    return QMatrix4x4(result);
}


Quaternion Matrix4D2Quaternion(QMatrix4x4 m)
{
    Quaternion q;
    q.w = qSqrt( qMax( 0.0f, 1 + m(0,0) + m(1,1) + m(2,2) ) ) / 2;
//    q.x = Sqrt( qMax( 0, 1 + m(0,0) - m(1,1) - m(2,2) ) ) / 2;
//    q.y = Sqrt( qMax( 0, 1 - m(0,0) + m(1,1) - m(2,2) ) ) / 2;
//    q.z = Sqrt( qMax( 0, 1 - m(0,0) - m(1,1) + m(2,2) ) ) / 2;
//    q.x *= Sign( q.x * ( m(2,1) - m(1,2) ) );
//    q.y *= Sign( q.y * ( m(0,2) - m(2,0) ) );
//    q.z *= Sign( q.z * ( m(1,0) - m(0,1) ) );
    q.x = (m(2,1)-m(1,2))/(4*q.w);
    q.y = (m(0,2)-m(2,0))/(4*q.w);
    q.z = (m(1,0)-m(0,1))/(4*q.w);
    return q;
}



