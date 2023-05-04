/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cone class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cone.h"
#include <math.h>

/**
* Cone's intersection method.  The input is a ray.
*/
float Cone::intersect(glm::vec3 p0, glm::vec3 dir)
{
    glm::vec3 vdif = p0 - center;
    float coneHeight = height + center.y;
    float r = (radius / height) * (radius / height);
    vdif.y = coneHeight - p0.y;

    float a = dir.x * dir.x + dir.z * dir.z - r * dir.y * dir.y;
    float b = 2 * (dir.x * vdif.x + dir.z * vdif.z +  r * dir.y * vdif.y);
    float c = vdif.x * vdif.x + vdif.z * vdif.z - vdif.y * vdif.y * r;
    float delta = b * b - 4 * a * c;

    if(delta < 0.001) return -1;

    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);

    float cv = (t1 < 0) ? ((t2 > 0) ? t2 : -1) : t1;

    float pty = p0.y + cv * dir.y;

    return (pty > coneHeight) ? -1 : cv;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the cone.
*/
glm::vec3 Cone::normal(glm::vec3 p)
{
    glm::vec3 n = p - center;
    float theta = atan(radius / height);
    float alpha = atan(n.x / n.z);

    return glm::vec3(sin(alpha) * cos(theta), sin(theta), cos(alpha) * cos(theta));
}
