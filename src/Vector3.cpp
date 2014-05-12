#include "Vector3.h"

Vector3 Vector3::Make(GLfloat x, GLfloat y, GLfloat z)
{
  Vector3 p;
  p.x = x; p.y = y; p.z = z;
  return p;
}
GLfloat Vector3::Length(Vector3 * p)
{
  //normal length of a vector 
  return (GLfloat)(sqrt(p->x*p->x + p->y*p->y + p->z*p->z));  
}

//Cross product of p and q
Vector3 Vector3::Cross(Vector3 * p, Vector3 * q)
{
  Vector3 t;
  t.x = p->y*q->z - p->z*q->y; t.y = p->z*q->x - p->x*q->z; t.z = p->x*q->y - p->y*q->x;
  return t;
}
//The normalized vector should be this vector
//normalized by its length
Vector3 Vector3::Norm(Vector3 p)
{
  float l = Length(&p);
  if (l == 0.0f) return Make(0.0f,0.0f,0.0f);
  Vector3 t;
  t.x = p.x / l; t.y = p.y / l; t.z = p.z / l;
  return t;
}
