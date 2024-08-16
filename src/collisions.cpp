#include "collisions.h"
#include <iostream>

/**
 * @brief Verifica se dois cubos representados por AABBs se intersectam nos eixos x, y e z.
 * Adaptado de https://gamedev.stackexchange.com/questions/96060/collision-detection-in-opengl
 *
 * @param a O primeiro cubo AABB.
 * @param b O segundo cubo AABB.
 * @return True se os cubos AABB se intersectam, false caso contrário.
 */
bool CubeIntersectsCube(AABB a, AABB b){
    int intersectedAxes = 0;

    if (a.min.x <= b.max.x && a.max.x >= b.min.x) intersectedAxes++;
    if (a.min.y <= b.max.y && a.max.y >= b.min.y) intersectedAxes++;
    if (a.min.z <= b.max.z && a.max.z >= b.min.z) intersectedAxes++;

    std::cout << "Eixos com intersecção: em " << intersectedAxes << std::endl;

    return intersectedAxes == 3;
}

/**
 * Shirley, P., Wald, I., Marrs, A. (2021). Ray Axis-Aligned Bounding Box Intersection. 
 * In: Marrs, A., Shirley, P., Wald, I. (eds) Ray Tracing Gems II. Apress, Berkeley, CA. 
 * https://doi.org/10.1007/978-1-4842-7185-8_2
 */

// Adaptado de: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html
bool RayIntersectsCube(glm::vec4 rayOrigin, glm::vec3 rayDirection, AABB cube){
    float tmin_x = (cube.min.x - rayOrigin.x) / rayDirection.x;
    float tmax_x = (cube.max.x - rayOrigin.x) / rayDirection.x;

    if (tmin_x > tmax_x) std::swap(tmin_x, tmax_x);

    float tmin = tmin_x;
    float tmax = tmax_x;

    float tmin_y = (cube.min.y - rayOrigin.y) / rayDirection.y;
    float tmax_y = (cube.max.y - rayOrigin.y) / rayDirection.y;

    if (tmin_y > tmax_y) std::swap(tmin_y, tmax_y);

    // Verifica se o raio não intersecta o cubo
    if (tmin > tmax_y || tmin_y > tmax) return false;

    // Seleciona os valores de t, onde tmin é o maior valor e tmax é o menor valor
    if (tmin_y > tmin) tmin = tmin_y;
    if (tmax_y < tmax) tmax = tmax_y;

    float tmin_z = (cube.min.z - rayOrigin.z) / rayDirection.z;
    float tmax_z = (cube.max.z - rayOrigin.z) / rayDirection.z;

    if (tmin_z > tmax_z) std::swap(tmin_z, tmax_z);

    // Verifica se o raio não intersecta o cubo
    if (tmin > tmax_z || tmin_z > tmax) return false;

    if (tmin_z > tmin) tmin = tmin_z;
    if (tmax_z < tmax) tmax = tmax_z;

    return true;
}

/**
 * @brief Calcula o axis-aligned bounding box (AABB) para um objeto da cena em coordenadas de mundo.
 * 
 * @param obj O objeto da cena, que contém em sua struct uma AABB em coordenadas de modelo.
 * @param model A matriz modelo que representa as transformações geométricas necessárias para o objeto estar em coordenadas de mundo.
 * @return A AABB do objeto em coordenadas de mundo.
 */
AABB GetWorldAABB(SceneObject obj, glm::mat4 model){
    // Dada uma matriz model, transforma as coordenadas locais da AABB do objeto para coordenadas de mundo.
    glm::vec4 min = model * glm::vec4(obj.bbox_min.x, obj.bbox_min.y, obj.bbox_min.z, 1.0f);
    glm::vec4 max = model * glm::vec4(obj.bbox_max.x, obj.bbox_max.y, obj.bbox_max.z, 1.0f);

    // @DEBUG
    // std::cout << "BBox Local Min: " << obj.bbox_min.x << " " << obj.bbox_min.y << " " << obj.bbox_min.z << std::endl;
    // std::cout << "BBox Local Max: " << obj.bbox_max.x << " " << obj.bbox_max.y << " " << obj.bbox_max.z << std::endl;
    //std::cout << "BBox World Min: " << min.x << " " << min.y << " " << min.z << std::endl;
    //std::cout << "BBox World Max: " << max.x << " " << max.y << " " << max.z << std::endl;

    // Retorna a bounding box em coordenadas de  mundo
    return AABB{min, max}; 
}

/**
 * @brief Projeta um ray casting a partir das coordenadas do mouse.
 * Adaptado de: https://antongerdelan.net/opengl/raycasting.html
 * 
 * As coordenadas do mouse são convertidas para o sistema de coordenadas normalizado (NDC),
 * em seguida para o sistema de coordenadas do clip, para o sistema de coordenadas
 * da câmera e, por fim, para o sistema de coordenadas do mundo.. O vetor resultante é 
 * normalizado.
 */
glm::vec3 MouseRayCasting(glm::mat4 projectionMatrix, glm::mat4 viewMatrix){
    // Coordenadas do mouse em NDC
    float x = (2.0f * g_LastCursorPosX) / g_ScreenWidth - 1.0f; 
    float y = 1.0f - (2.0f * g_LastCursorPosY) / g_ScreenHeight;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);

    // Coordenadas do mouse em clip
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

    // Coordenadas do mouse no sistema da câmera
    glm::vec4 ray_camera = glm::inverse(projectionMatrix) * ray_clip;
    ray_camera = glm::vec4(ray_camera.x, ray_camera.y, -1.0, 0.0); // Desconsidera os componentes z,w

    // Normaliza o vetor de coordenadas do mouse
    glm::vec3 ray_world = glm::vec3(glm::inverse(viewMatrix) * ray_camera);
    ray_world = glm::normalize(ray_world);

    // @DEBUG
    // std::cout << "Ray World: " << ray_world.x << " " << ray_world.y << " " << ray_world.z << std::endl;

    return ray_world;

}