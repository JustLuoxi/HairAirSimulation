#include "air.h"

extern Model *objModel;

Air::Air()
{


}

Air::~Air()
{

}

void Air::Init()
{
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    this->createGrid();
    this->UpdateRenderdata();

    // link hair
    this->hair = objModel->m_hair;
}

void Air::Simulate()
{
    VelocityStep(this->m_grid->Vx, this->m_grid->Vy,this->m_grid->Vz,this->m_grid->Vx_p,this->m_grid->Vy_p,this->m_grid->Vz_p, this->dt);
    //    DensityStep(this->m_grid->rho, this->m_grid->rho_p, this->m_grid->Vx,this->m_grid->Vy, this->m_grid->Vz, this->dt);
    this->UpdateRenderdata();
}

void Air::HairInfluenceAir()
{
    if(!hair2air_flag) return;

    QVector3D haircoord, hairv;
    Hairline hairline;
    Spring spring;
    GLfloat id_x,id_y,id_z;
    GLfloat l2,w;
    GLfloat *weights;
    weights = new GLfloat[RESOLUTION*RESOLUTION*RESOLUTION]();
    int N = RESOLUTION;
    int kernel = 1;
    int x0,x1,y0,y1,z0,z1;


    for(int i=0;i<this->hair->m_guide_hairlines.size();i++)
    {
        hairline = this->hair->m_guide_hairlines[i];
        for(int j=0;j<hairline.m_Springs->size();j++)
        {
            spring = *hairline.m_Springs->at(j);
            haircoord = * spring.m_x;
            hairv = spring.m_v;

            id_x = W2A(haircoord.x());
            id_y = W2A(haircoord.y());
            id_z = W2A(haircoord.z());

            x0 = check2ID(id_x);
            x1 = x0+1;
            y0 = check2ID(id_y);
            y1 = y0+1;
            z0 = check2ID(id_z);
            z1 = z0+1;


            for(int ii=x0-kernel+1;ii<x0+kernel;ii++){
                for(int jj=y0-kernel+1;jj<y0+kernel;jj++){
                    for(int kk=z0-kernel+1;kk<z0+kernel;kk++){
                        if(ii<2||ii>N-2||jj<2||jj>N-2||kk<2||kk>N-2) continue;
                        //                        this->m_grid->Vx[IDX(ii,jj,kk)] = hairv.x()/AIR2HAIRSCALE;
                        //                        this->m_grid->Vy[IDX(ii,jj,kk)] = hairv.y()/AIR2HAIRSCALE;
                        //                        this->m_grid->Vz[IDX(ii,jj,kk)] = hairv.z()/AIR2HAIRSCALE;
                        this->m_grid->Vx[IDX(ii,jj,kk)] = 0.0f;
                        this->m_grid->Vy[IDX(ii,jj,kk)] = 0.0f;
                        this->m_grid->Vz[IDX(ii,jj,kk)] = 0.0f;
                    }
                }
            }

            for(int ii=x0-kernel+1;ii<x0+kernel;ii++){
                for(int jj=y0-kernel+1;jj<y0+kernel;jj++){
                    for(int kk=z0-kernel+1;kk<z0+kernel;kk++){
                        if(ii<2||ii>N-2||jj<2||jj>N-2||kk<2||kk>N-2) continue;
                        l2 = (id_x - ii)*(id_x-ii) + (id_y-jj)*(id_y-jj)+(id_z-kk)*(id_z-kk);
                        w = std::exp(-l2);
                        weights[IDX(ii,jj,kk)] += w;
                        this->m_grid->Vx[IDX(ii,jj,kk)] += w*hairv.x()/AIR2HAIRSCALE;
                        this->m_grid->Vy[IDX(ii,jj,kk)] += w*hairv.y()/AIR2HAIRSCALE;
                        this->m_grid->Vz[IDX(ii,jj,kk)] += w*hairv.z()/AIR2HAIRSCALE;
                    }
                }
            }
        }
    }

    for(int i=0;i<N*N*N;i++)
    {
        if(weights[i]!=0)
        {
            this->m_grid->Vx[i]/=weights[i];
            this->m_grid->Vy[i]/=weights[i];
            this->m_grid->Vz[i]/=weights[i];
        }
    }
}

// update rendering data
void Air::UpdateRenderdata()
{
    // create render data
    GLfloat x,y,z,h;
    int N = RESOLUTION;
    h = RADIUS*2/(N-1);

    render_ps.clear();

    if(render_flag){
        QVector3D s,e;
        for(int i = 1;i<N-1;i++){
            x = (i-0.5f)*h;
            for(int j = 1;j<N-1;j++){
                y = (j-0.5f)*h;
                for(int k=1;k<N-1;k++){
                    z = (k-0.5f)*h;

                    s = QVector3D(x,y,z)-QVector3D(RADIUS,RADIUS,RADIUS);
                    e = s + QVector3D(this->m_grid->Vx[IDX(i,j,k)],
                            this->m_grid->Vy[IDX(i,j,k)],
                            this->m_grid->Vz[IDX(i,j,k)]);
                    render_ps.push_back(s);
                    render_ps.push_back(e);
                }
            }
        }
    }

    // bind data and render them
    this->bindBufferData();
}

// draw velocity
void Air::Draw()
{
    core->glEnableVertexAttribArray(0);
    core->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    core->glVertexAttribPointer(
                0,                  // attribute
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
                );

    core->glDrawArrays(GL_LINES,0,render_ps.size());
}



void Air::DensityStep(GLfloat *d, GLfloat *d_p, GLfloat *vx, GLfloat *vy, GLfloat *vz, GLfloat dt)
{
    diffuse(0, d_p, d,diff,dt);
    advect(0,d,d_p,vx,vy,vz,dt);
}

void Air::VelocityStep(GLfloat *vx, GLfloat *vy, GLfloat *vz, GLfloat *vx_p, GLfloat *vy_p, GLfloat *vz_p, GLfloat dt)
{
    diffuse(1,vx_p,vx,viscosity,dt);
    diffuse(2,vy_p,vy,viscosity,dt);
    diffuse(3,vz_p,vz,viscosity,dt);

    project(vx_p,vy_p,vz_p,vx,vy);

    advect(1,vx,vx_p,vx_p,vy_p,vz_p,dt);
    advect(2,vy,vy_p,vx_p,vy_p,vz_p,dt);
    advect(3,vz,vz_p,vx_p,vy_p,vz_p,dt);

    project(vx, vy, vz, vx_p, vy_p);
}

void Air::bindBufferData()
{
    core->glDeleteBuffers(1, &VBO);

    core->glGenBuffers(1,&VBO);
    core->glBindBuffer(GL_ARRAY_BUFFER,VBO);
    core->glBufferData(GL_ARRAY_BUFFER,render_ps.size()* sizeof(QVector3D), &render_ps[0], GL_STATIC_DRAW);

}


void Air::createGrid()
{
    // create the grid
    this->m_grid = new Grid();

    this->m_grid->rho = new GLfloat[RESOLUTION*RESOLUTION*RESOLUTION]();
    this->m_grid->Vx = new GLfloat[RESOLUTION*RESOLUTION*RESOLUTION]();
    this->m_grid->Vy = new GLfloat[RESOLUTION*RESOLUTION*RESOLUTION]();
    this->m_grid->Vz = new GLfloat[RESOLUTION*RESOLUTION*RESOLUTION]();

    this->m_grid->rho_p = new GLfloat[RESOLUTION*RESOLUTION*RESOLUTION]();
    this->m_grid->Vx_p = new GLfloat[RESOLUTION*RESOLUTION*RESOLUTION]();
    this->m_grid->Vy_p = new GLfloat[RESOLUTION*RESOLUTION*RESOLUTION]();
    this->m_grid->Vz_p = new GLfloat[RESOLUTION*RESOLUTION*RESOLUTION]();

    //    int N = RESOLUTION*RESOLUTION*RESOLUTION;
    //    GLfloat h = RADIUS*2/(RESOLUTION-1);
    //    for(int i=0;i<N;i++)
    //    {
    //        this->m_grid->Vx[i] += h/2;
    //    }
}

void Air::AddDensity(int x, int y, int z, GLfloat d)
{
    int N = RESOLUTION*RESOLUTION*RESOLUTION;
    for(int i=0;i<N;i++)
    {
        this->m_grid->rho_p[i] = 0.0f;
    }
    this->m_grid->rho_p[IDX(x,y,z)] = d;
    this->m_grid->rho[IDX(x,y,z)] += d *this->dt;

    //    for(int i=0;i<N;i++)
    //    {
    //        this->m_grid->rho[i] += this->m_grid->rho_p[i]*this->dt;
    //    }

}

void Air::AddVelocity(int x, int y, int z, GLfloat vx, GLfloat vy, GLfloat vz)
{
    int N = RESOLUTION*RESOLUTION*RESOLUTION;
    for(int i=0;i<N;i++)
    {
        this->m_grid->Vx_p[i] = 0.0f;
        this->m_grid->Vy_p[i] = 0.0f;
        this->m_grid->Vz_p[i] = 0.0f;
    }

    this->m_grid->Vx_p[IDX(x,y,z)] = vx;
    this->m_grid->Vy_p[IDX(x,y,z)] = vy;
    this->m_grid->Vz_p[IDX(x,y,z)] = vz;
    this->m_grid->Vx[IDX(x,y,z)] += vx*this->dt;
    this->m_grid->Vy[IDX(x,y,z)] += vy*this->dt;
    this->m_grid->Vz[IDX(x,y,z)] += vz*this->dt;

    //    for(int i=0;i<N;i++)
    //    {
    //        this->m_grid->Vx[i] += this->m_grid->Vx_p[i]*this->dt;
    //        this->m_grid->Vy[i] += this->m_grid->Vy_p[i]*this->dt;
    //        this->m_grid->Vz[i] += this->m_grid->Vz_p[i]*this->dt;
    //    }

}

void Air::Reset()
{
    int N = RESOLUTION*RESOLUTION*RESOLUTION;
    for(int i=0;i<N;i++)
    {
        this->m_grid->Vx[i] = 0.0f;
        this->m_grid->Vy[i] = 0.0f;
        this->m_grid->Vz[i] = 0.0f;
        this->m_grid->Vx_p[i] = 0.0f;
        this->m_grid->Vy_p[i] = 0.0f;
        this->m_grid->Vz_p[i] = 0.0f;
    }
}

QVector3D Air::GetVelocity(GLfloat x, GLfloat y, GLfloat z)
{
    //    int N = RESOLUTION;
    //    float h = RADIUS*2/(N-1);
    //    x = (x + RADIUS)/h;
    //    y = (y + RADIUS)/h;
    //    z = (z + RADIUS)/h;
    x = W2A(x);
    y = W2A(y);
    z = W2A(z);
    GLfloat xx,yy,zz;
    xx = trilinearInterpolation(x,y,z,this->m_grid->Vx);
    yy = trilinearInterpolation(x,y,z,this->m_grid->Vy);
    zz = trilinearInterpolation(x,y,z,this->m_grid->Vz);

    return QVector3D(xx, yy, zz);
}

void Air::setBound(int b, GLfloat *v)
{
    int N = RESOLUTION;

    // setting faces
    for(int j = 1; j < N - 1; j++) {
        for(int i = 1; i < N - 1; i++) {
            v[IDX(i, j, 0  )] = b == 3 ? -v[IDX(i, j, 1  )] : v[IDX(i, j, 1  )];
            v[IDX(i, j, N-1)] = b == 3 ? -v[IDX(i, j, N-2)] : v[IDX(i, j, N-2)];
        }
    }
    for(int k = 1; k < N - 1; k++) {
        for(int i = 1; i < N - 1; i++) {
            v[IDX(i, 0  , k)] = b == 2 ? -v[IDX(i, 1  , k)] : v[IDX(i, 1  , k)];
            v[IDX(i, N-1, k)] = b == 2 ? -v[IDX(i, N-2, k)] : v[IDX(i, N-2, k)];
        }
    }
    for(int k = 1; k < N - 1; k++) {
        for(int j = 1; j < N - 1; j++) {
            v[IDX(0  , j, k)] = b == 1 ? -v[IDX(1  , j, k)] : v[IDX(1  , j, k)];
            v[IDX(N-1, j, k)] = b == 1 ? -v[IDX(N-2, j, k)] : v[IDX(N-2, j, k)];
        }
    }

    //Setting edges
    for (int i=1; i<N-1; i++) {
        v[IDX(i,  0,  0)] = 0.5f*(v[IDX(i,1,  0)]+  v[IDX(i,  0,  1)]);
        v[IDX(i,N-1,  0)] = 0.5f*(v[IDX(i,N-2,  0)]+v[IDX(i,N-1,  1)]);
        v[IDX(i,  0,N-1)] = 0.5f*(v[IDX(i,0,  N-2)]+v[IDX(i,  1,N-1)]);
        v[IDX(i,N-1,N-1)] = 0.5f*(v[IDX(i,N-2,N-1)]+v[IDX(i,N-1,  N-2)]);
    }

    for (int i=1; i<N-1; i++) {
        v[IDX(0,  i,  0)] = 0.5f*(v[IDX(1,i,  0)]+  v[IDX(0,  i,  1)]);
        v[IDX(N-1,i,  0)] = 0.5f*(v[IDX(N-2,i,  0)]+v[IDX(N-1,i,  1)]);
        v[IDX(0,  i,N-1)] = 0.5f*(v[IDX(0,i,  N-2)]+v[IDX(1,  i,N-1)]);
        v[IDX(N-1,i,N-1)] = 0.5f*(v[IDX(N-2,i,N-1)]+v[IDX(N-1,i,  N-2)]);
    }

    for (int i=1; i<N-1; i++) {
        v[IDX(0,  0,  i)] = 0.5f*(v[IDX(0,  1,i)]+  v[IDX(1,  0,  i)]);
        v[IDX(0,  N-1,i)] = 0.5f*(v[IDX(0,  N-2,i)]+v[IDX(1,  N-1,i)]);
        v[IDX(N-1,0,  i)] = 0.5f*(v[IDX(N-2,  0,i)]+v[IDX(N-1,1,  i)]);
        v[IDX(N-1,N-1,i)] = 0.5f*(v[IDX(N-1,N-2,i)]+v[IDX(N-2, N-1,i)]);
    }

    // setting corners
    v[IDX(0, 0, 0)]       = 0.33f * (v[IDX(1, 0, 0)]
            +  v[IDX(0, 1, 0)]
            +  v[IDX(0, 0, 1)]);
    v[IDX(0, N-1, 0)]     = 0.33f * (v[IDX(1, N-1, 0)]
            +  v[IDX(0, N-2, 0)]
            +  v[IDX(0, N-1, 1)]);
    v[IDX(0, 0, N-1)]     = 0.33f * (v[IDX(1, 0, N-1)]
            +  v[IDX(0, 1, N-1)]
            +  v[IDX(0, 0, N)]);
    v[IDX(0, N-1, N-1)]   = 0.33f * (v[IDX(1, N-1, N-1)]
            +  v[IDX(0, N-2, N-1)]
            +  v[IDX(0, N-1, N-2)]);
    v[IDX(N-1, 0, 0)]     = 0.33f * (v[IDX(N-2, 0, 0)]
            +  v[IDX(N-1, 1, 0)]
            +  v[IDX(N-1, 0, 1)]);
    v[IDX(N-1, N-1, 0)]   = 0.33f * (v[IDX(N-2, N-1, 0)]
            +  v[IDX(N-1, N-2, 0)]
            +  v[IDX(N-1, N-1, 1)]);
    v[IDX(N-1, 0, N-1)]   = 0.33f * (v[IDX(N-2, 0, N-1)]
            +  v[IDX(N-1, 1, N-1)]
            +  v[IDX(N-1, 0, N-2)]);
    v[IDX(N-1, N-1, N-1)] = 0.33f * (v[IDX(N-2, N-1, N-1)]
            +  v[IDX(N-1, N-2, N-1)]
            +  v[IDX(N-1, N-1, N-2)]);
}

void Air::mySolver(int b, GLfloat *v, GLfloat *v_p, GLfloat a, GLfloat c)
{
    int N = RESOLUTION;
    for(int l = 0;l<LINEARSOLVERITER;l++){
        for(int i=1;i<N-1;i++){
            for(int j=1;j<N-1;j++){
                for(int k = 1;k<N-1;k++)
                {
                    v[IDX(i,j,k)] = ( v_p[IDX(i,j,k)]
                            + a*(v[IDX(i+1,j,k)]
                            +v[IDX(i-1,j,k)]
                            +v[IDX(i,j+1,k)]
                            +v[IDX(i,j-1,k)]
                            +v[IDX(i,j,k+1)]
                            +v[IDX(i,j,k-1)]))/c ;
                }
            }
        }
        setBound(b,v);
    }
}

void Air::diffuse(int b, GLfloat *v, GLfloat *v_p, GLfloat diff, GLfloat dt)
{
    GLfloat a = dt * diff * (RESOLUTION-2) * (RESOLUTION - 2);
    this->mySolver(b,v,v_p,a,1+6*a);
}

void Air::advect(int b, GLfloat *x_c, GLfloat *x_p, GLfloat *vx, GLfloat *vy, GLfloat *vz, GLfloat dt)
{
    int N = RESOLUTION;
    int i0,j0,k0,i1,j1,k1;
    GLfloat s0,t0,s1,t1,u0,u1;
    GLfloat x,y,z,dtx,dty,dtz;
    dtx=dty=dtz=dt*(N-2);

    for(int i=1;i<N-1;i++){
        for(int j=1;j<N-1;j++){
            for(int k=1;k<N-1;k++){
                x = i - dtx*vx[IDX(i,j,k)];
                y = j - dty*vy[IDX(i,j,k)];
                z = k - dtz*vz[IDX(i,j,k)];

                x_c[IDX(i,j,k)] = trilinearInterpolation(x,y,z,x_p);

            }
        }
    }

    setBound(b,x_c);
}

void Air::project(GLfloat *vx, GLfloat *vy, GLfloat *vz, GLfloat *p, GLfloat *div)
{
    int N = RESOLUTION;
    for(int i=1;i<N-1;i++){
        for(int j=1;j<N-1;j++){
            for(int k=1;k<N-1;k++){
                div[IDX(i,j,k)] = -0.5f*(
                            vx[IDX(i+1,j,k)]
                        -vx[IDX(i-1,j,k)]
                        +vy[IDX(i,j+1,k)]
                        -vy[IDX(i,j-1,k)]
                        +vz[IDX(i,j,k+1)]
                        -vz[IDX(i,j,k-1)]
                        )/N;
                p[IDX(i,k,k)] = 0;
            }
        }
    }
    setBound(0,div);
    setBound(0,p);
    mySolver(0,p,div,1,6);

    for(int i=1;i<N-1;i++){
        for(int j=1;j<N-1;j++){
            for(int k=1;k<N-1;k++){
                vx[IDX(i,j,k)] -= 0.5f*(p[IDX(i+1,j,k)]-p[IDX(i-1,j,k)])*N;
                vy[IDX(i,j,k)] -= 0.5f*(p[IDX(i,j+1,k)]-p[IDX(i,j-1,k)])*N;
                vz[IDX(i,j,k)] -= 0.5f*(p[IDX(i,j,k+1)]-p[IDX(i,j,k-1)])*N;
            }
        }
    }

    setBound(1,vx);
    setBound(2,vy);
    setBound(3,vz);
}

// x,y,z: index
// source: original data to be interpolated
GLfloat Air::trilinearInterpolation(GLfloat x, GLfloat y, GLfloat z, GLfloat *source)
{
    int N = RESOLUTION;
    int i0,j0,k0,i1,j1,k1;
    GLfloat s0,t0,s1,t1,u0,u1;
    GLfloat result;

    i0 = check2ID(x);
    i1 = i0 + 1;

    j0 = check2ID(y);
    j1 = j0 + 1;

    k0 = check2ID(z);
    k1 = k0 + 1;

    s1 = x - i0;
    s0 = 1.0f - s1;
    t1 = y - j0;
    t0 = 1.0f - t1;
    u1 = z - k0;
    u0 = 1.0f - u1;

    result = s0*(t0*u0*source[IDX(i0,j0,k0)]
            +t1*u0*source[IDX(i0,j1,k0)]
            +t0*u1*source[IDX(i0,j0,k1)]
            +t1*u1*source[IDX(i0,j1,k1)]) +
            s1*(t0*u0* source[IDX(i1,j0,k0)]
            +t1*u0*source[IDX(i1,j1,k0)]
            +t0*u1*source[IDX(i1,j0,k1)]
            +t1*u1*source[IDX(i1,j1,k1)]);
    return result;
}

int Air::check2ID(GLfloat &x)
{
    int N = RESOLUTION;

    if(x<0.5f) x = 0.5f;
    if(x>N + 0.5f) x = N + 0.5f;
    return floorf(x);
}


