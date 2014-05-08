#if defined(__APPLE__) || defined(MACOSX)
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

struct Vertex3
{
    GLfloat x,y,z;
    GLfloat nx,ny,nz;  //normal vector
    GLfloat UpSpeed;
    GLfloat newY;
    bool bIsExciter;
    //only in use, if bIsExciter is true:
    float ExciterAmplitude;  
    float ExciterFrequency;

    static Vertex3 Make( GLfloat x, GLfloat y, GLfloat z, 
      GLfloat nx, GLfloat ny, GLfloat nz, 
      GLfloat UpSpeed, bool bIsExciter) {
      Vertex3 t;
      t.x = x; t.y = y; t.z = z;
      t.nx = nx; t.ny = ny; t.nz = nz;
      t.UpSpeed = UpSpeed; t.bIsExciter = bIsExciter;
      return t;
    }
};
