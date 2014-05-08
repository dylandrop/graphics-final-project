#include "Vector3.h"

Vector3 Vector3::Make(GLfloat x, GLfloat y, GLfloat z)
{
  Vector3 t;
  t.x = x; t.y = y; t.z = z;
  return t;
}
GLfloat Vector3::Length(Vector3 * v)
{
  return (GLfloat)(sqrt(v->x*v->x+v->y*v->y+v->z*v->z));  
}
Vector3 Vector3::Cross(Vector3 * u, Vector3 * v)
{
  Vector3 t;
  t.x = u->y*v->z - u->z*v->y; t.y = u->z*v->x - u->x*v->z; t.z = u->x*v->y - u->y*v->x;
  return t;
}
Vector3 Vector3::Norm(Vector3 v)
{
  float l = Length(&v);
  if (l == 0.0f) return Make(0.0f,0.0f,0.0f);
  Vector3 t;
  t.x = v.x / l; t.y = v.y / l; t.z = v.z / l;
  return t;
}
