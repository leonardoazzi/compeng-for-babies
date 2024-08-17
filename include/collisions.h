#ifndef _COLLISIONS_H
#define _COLLISIONS_H

#include "globals.h"

bool AABBIntersectsAABB(AABB a, AABB b);
bool RayIntersectsAABB(glm::vec4 rayOrigin, glm::vec3 rayDirection, AABB cube);
bool SphereIntersectsAABB(Sphere sphere, AABB aabb);
AABB FindGroupBbox(std::vector<AABB> objects);
AABB GetWorldAABB(SceneObject obj, glm::mat4 model);
glm::vec3 MouseRayCasting(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);

#endif // _COLLISIONS_H