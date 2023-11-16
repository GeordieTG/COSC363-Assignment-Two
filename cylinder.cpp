#include "cylinder.h"
#include <math.h>

/**
* Cylinder's intersection method.  The input is a ray.
*/
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
{
    // Cylinder

    float a = (dir.x * dir.x) + (dir.z * dir.z);
    float b = 2 * (dir.x * (p0.x - center.x) + dir.z * (p0.z - center.z));
    float c = ((p0.x - center.x) * (p0.x - center.x)) + ((p0.z - center.z) * (p0.z - center.z)) - (radius * radius);
    float delta = (b*b) - (4 * (a * c));

    if(delta < 0.001) return -1.0;    //includes zero and negative values

    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);

    float ht1 = p0.y + t1 * dir.y;
    float ht2 = p0.y + t2 * dir.y;

    float cylinderBottom = center.y;
    float cylinderTop = center.y + height;


    if (ht1 >= cylinderBottom && ht1 <= cylinderTop) {
        if (t1 > 0) {
            return t1;
        }
    }

    if (ht2 >= cylinderBottom && ht2 <= cylinderTop) {
        if (t2 > 0) {
            return (center.y + height - p0.y) / dir.y;
        }
    }

    return -1.0;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    // Point is on the side of the cylinder
    glm::vec3 n(p.x - center.x, 0, p.z - center.z);

    if (p.y == center.y + height) {
        return glm::vec3(0,1,0);
    }

    return glm::normalize(n);
}

