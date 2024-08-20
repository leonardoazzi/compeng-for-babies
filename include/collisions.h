#ifndef _COLLISIONS_H
#define _COLLISIONS_H

#include "globals.h"

bool AABBIntersectsAABB(AABB a, AABB b);
bool RayIntersectsAABB(glm::vec4 rayOrigin, glm::vec3 rayDirection, AABB cube);
bool SphereIntersectsAABB(Sphere sphere, AABB aabb);
glm::vec4 SphereCollisionResolution(glm::vec3 point, Sphere sphere);
glm::vec3 AABBAndSphereResolution(AABB aabb, Sphere sphere);
bool PointIntersectsSphere(glm::vec3 point, Sphere sphere);
AABB FindGroupBbox(std::vector<AABB> objects);
AABB GetWorldAABB(SceneObject obj, glm::mat4 model);
glm::vec3 MouseRayCasting(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);

#endif // _COLLISIONS_H