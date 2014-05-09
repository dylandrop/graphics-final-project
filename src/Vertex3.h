#if defined(__APPLE__) || defined(MACOSX)
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

struct Vertex3
{
    GLfloat x, y, z;
    GLfloat nX, nY, nZ;
    GLfloat vel, newY;
    bool jump;
    GLfloat jumpAmt, jumpFreq;

    static Vertex3 Make( GLfloat x, GLfloat y, GLfloat z, 
      GLfloat nX, GLfloat nY, GLfloat nZ, 
      GLfloat vel, bool jump) {
      Vertex3 t;
      t.x = x; t.y = y; t.z = z;
      t.nX = nX; t.nY = nY; t.nZ = nZ;
      t.vel = vel; t.jump = jump;
      return t;
    }
};
