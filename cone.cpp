#include "cone.h"
#include <cmath>

float Cone::intersect(glm::vec3 p0, glm::vec3 dir)
{
    float yd = height - p0.y + center.y;
    float rh = radius / height;
    float tan = rh * rh;

    float a = (dir.x * dir.x) + (dir.z * dir.z) - (tan*dir.y*dir.y);
    float b = 2*((p0.x - center.x) * dir.x + (p0.z - center.z) * dir.z + tan * yd * dir.y);
    float c = ((p0.x - center.x) * (p0.x - center.x)) + ((p0.z - center.z) * (p0.z - center.z)) - (tan * yd * yd);

    float delta = (b*b) - (4 * (a * c));

    if(delta < 0.001) return -1.0;    //includes zero and negative values

    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);

    float ht1 = p0.y + t1 * dir.y;
    float ht2 = p0.y + t2 * dir.y;

    if ((ht1 < center.y || ht1 > center.y + height) && (ht2 < center.y || ht2 > center.y + height))
    {
        return -1.0f;
    }

    return (ht1 >= center.y && ht1 <= center.y + height) ? t1 : t2;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Cone::normal(glm::vec3 p)
{
    // Point is on the side of the cylinder

    float alpha = atan((p.x - center.x) / (p.z - center.z));
    float theta = atan(radius / height);
    glm::vec3 n(sin(alpha)*cos(theta), sin(theta), cos(alpha)*cos(theta));
    return n;
}
