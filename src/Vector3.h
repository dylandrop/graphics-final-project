
#include <math.h>

#if defined(__APPLE__) || defined(MACOSX)
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

// representation of a 3d vector
struct Vector3
{
  GLfloat x,y,z;
  static Vector3 Make( GLfloat x, GLfloat y, GLfloat z );
  //cross product
  static Vector3 Cross(Vector3 * q, Vector3 * p);
  //normalize
  static Vector3 Norm( Vector3 p);
  //get the length
  static GLfloat Length( Vector3 * p);

  //add two vectors together
  friend Vector3 operator+ (Vector3 p, Vector3 q)
  {
    Vector3 t;
    t.x = p.x + q.x; t.y = p.y + q.y; t.z = p.z + q.z;
    return t;
  }

  friend Vector3 operator- (Vector3 p, Vector3 q)
  {
    Vector3 t;
    t.x = p.x - q.x; t.y = p.y - q.y; t.z = p.z - q.z;
    return t;
  }
};
