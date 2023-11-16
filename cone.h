#ifndef CONE_H
#define CONE_H
#include <glm/glm.hpp>
#include "SceneObject.h"

class Cone : public SceneObject
{
private:
    glm::vec3 center = glm::vec3(0);
    float radius = 1.0f;
    float height = 1.0f;
public:
    Cone() {}; //Default constructor creates a unit sphere

    Cone(glm::vec3 c, float r, float h) : center(c), radius(r), height(h) {}

    float intersect(glm::vec3 p0, glm::vec3 dir);

    glm::vec3 normal(glm::vec3 p);
};
#endif // CONE_H
