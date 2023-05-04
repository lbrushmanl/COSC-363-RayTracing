/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cylinder class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

/**
* Cylinder's intersection method.  The input is a ray.
*/
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
{
    glm::vec3 vdif = p0 - center;

    float a = dir.x * dir.x + dir.z * dir.z;
    float b = 2 * (dir.x * vdif.x + dir.z * vdif.z);
    float c = vdif.x * vdif.x + vdif.z * vdif.z - radius * radius;
    float delta = b * b - 4 * a * c;

    if(delta < 0.001) return -1;

    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);

    float pt1 = p0.y + t1 * dir.y;
    float pt2 = p0.y + t2 * dir.y;

   if (pt1 > center.y + height && pt2 < center.y + height) return (center.y + height - p0.y) / dir.y;

    if (pt1 > center.y + height || pt1 < center.y) t1 = -1;

    if (pt2 > center.y + height || pt2 < center.y) t2 = -1;

    return (t1 > t2) ? (t2 >= 0 ? t2 : t1) : (t1 >= 0 ? t1 : t2);
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the cylinder.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    glm::vec3 n = p - center;
    return (fabs(n.y - height) < 0.001) ? glm::vec3(0, 1, 0) : glm::vec3(n.x/radius, 0, n.z/radius);
}
