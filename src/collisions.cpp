#include "collisions.h"
#include <iostream>
#include <vector>

/**
 * @brief Verifica se dois cubos representados por AABBs se intersectam nos eixos x, y e z.
 * FONTE (adaptado) https://gamedev.stackexchange.com/questions/96060/collision-detection-in-opengl
 *
 * @param a O primeiro cubo AABB.
 * @param b O segundo cubo AABB.
 * @return True se os cubos AABB se intersectam, false caso contrário.
 */
bool AABBIntersectsAABB(AABB a, AABB b){
    int intersectedAxes = 0;

    if (a.min.x <= b.max.x && a.max.x >= b.min.x) intersectedAxes++;
    if (a.min.y <= b.max.y && a.max.y >= b.min.y) intersectedAxes++;
    if (a.min.z <= b.max.z && a.max.z >= b.min.z) intersectedAxes++;

    return intersectedAxes == 3;
}

/**
 * Verifica se um raio intersecta com um cubo.
 *
 * @param rayOrigin O ponto de origem do raio.
 * @param rayDirection A direção do raio.
 * @param cube O eixo-alinhado bounding box (AABB) que representa o cubo.
 * @return True se o raio intersectar com o cubo, false caso contrário.
 * 
 * FONTE (adaptado): https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html
 * Referência:
 * Shirley, P., Wald, I., Marrs, A. (2021). Ray Axis-Aligned Bounding Box Intersection. 
 * In: Marrs, A., Shirley, P., Wald, I. (eds) Ray Tracing Gems II. Apress, Berkeley, CA. 
 * https://doi.org/10.1007/978-1-4842-7185-8_2
 * 
 */
bool RayIntersectsAABB(glm::vec4 rayOrigin, glm::vec3 rayDirection, AABB cube){
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
 * Verifica se uma esfera intersecta uma AABB (Axis-Aligned Bounding Box).
 * FONTE (adaptado) https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-detection
 * 
 * @param sphere A esfera a ser verificada, definida por seu centro e seu raio.
 * @param aabb O AABB a ser testada.
 * @return true se houver interseção, false caso contrário.
 */
bool SphereIntersectsAABB(Sphere sphere, AABB aabb) {
    float distance = 0.0f;

    // Calcula o ponto mais próximo da AABB ao centro da esfera
    for (int i = 0; i < 3; ++i) {
        float min = aabb.min[i];
        float max = aabb.max[i];
        float center = sphere.center[i];

        if (center < min) {
            distance += (min - center) * (min - center);
        } else if (center > max) {
            distance += (center - max) * (center - max);
        }
    }

    // Verifica se a distância ao quadrado é menor ou igual ao raio da esfera ao quadrado
    return distance <= (sphere.radius * sphere.radius);
}

/**
 * Verifica se um ponto intersecta uma esfera. Baseado na função SphereIntersectsAABB().
 * 
 * @param point O ponto a ser verificado.
 * @param sphere A esfera a ser verificada.
 * @return true se o ponto intersecta a esfera, false caso contrário.
 */
bool PointIntersectsSphere(glm::vec3 point, Sphere sphere) {
    float euclideanDistance = 0.0f;

    // Reduz o raio da esfera para evitar falsos positivos
    sphere.radius -= 0.5f;

    for (int i = 0; i < 3; ++i) {
        euclideanDistance += (point[i] - sphere.center[i]) * (point[i] - sphere.center[i]);
    }

    return euclideanDistance >= (sphere.radius * sphere.radius);
}

/**
 * Calcula a resolução de colisão entre um ponto e uma esfera.
 * 
 * @param point O ponto a ser testado.
 * @param sphere A esfera com a qual o ponto está sendo testado.
 * @return Um vetor de deslocamento que reposiciona o ponto para fora da esfera, caso haja colisão. Caso contrário, retorna um vetor nulo.
 */
glm::vec4 SphereCollisionResolution(glm::vec3 point, Sphere sphere) {
    // Inicializa o vetor de offset a ser utilizado para reposicionar o ponto
    glm::vec4 offset = {0.0f, 0.0f, 0.0f, 0.0f};

    // Vetor entre o ponto e o centro da esfera
    glm::vec4 directionVec = {point - sphere.center, 0.0f};

    // Define o vetor direção no plano xz apenas
    directionVec.y = 0.0f;
    // Ângulo do vetor direção no plano xz
    float angle = glm::atan(directionVec.z, directionVec.x);
    // Vetor que define o raio da esfera
    glm::vec4 radiusVec = {(sphere.radius - 0.5f - sphere.center.x), 0.0f, 0.0f, 0.0f};

    // Rotaciona o radiusVec no mesmo sentido do directionVec
    float cosAngle = glm::cos(angle);
    float sinAngle = glm::sin(angle);
    float newX = radiusVec.x * cosAngle - radiusVec.z * sinAngle;
    float newZ = radiusVec.x * sinAngle + radiusVec.z * cosAngle;
    radiusVec.x = newX;
    radiusVec.z = newZ;

    // Calcula o módulo dos vetores para compará-los
    float tam_direction = glm::length(directionVec);
    float tam_raio = glm::length(radiusVec);

    // Se o módulo do vetor direção for maior que o módulo do raio, o ponto está fora da esfera
    if (tam_direction > tam_raio){
        offset = directionVec - radiusVec;
    }

    return offset;

}

/**
 * @brief Calcula a resolução de colisão entre um cubo AABB e uma esfera.
 * Utilizou-se o Github Copilot para auxiliar na implementação.
 * 
 * @param aabb O cubo AABB.
 * @param sphere A esfera.
 * @return Um vetor de deslocamento que reposiciona o cubo para fora da esfera, caso haja colisão. Caso contrário, retorna um vetor nulo.
 */
glm::vec3 AABBAndSphereResolution(AABB aabb, Sphere sphere) {
    // Calcula o ponto mais próximo da AABB ao centro da esfera
    glm::vec3 closestPoint = glm::clamp(sphere.center, aabb.min, aabb.max);
    // Calcula o vetor entre o ponto mais próximo e o centro da esfera
    glm::vec3 direction = closestPoint - sphere.center;
    // Calcula a distância entre o ponto mais próximo e o centro da esfera
    float distance = glm::length(direction);
    // Verifica se há colisão
    if (distance < sphere.radius && distance > 0.1f) {
        // Calcula o vetor de deslocamento para reposicionar o cubo para fora da esfera
        glm::vec3 displacement = glm::normalize(direction) * (sphere.radius - distance);
        return displacement;
    }
    // Retorna um vetor nulo se não houver colisão
    return glm::vec3(0.0f);
}
/**
 * @brief Calcula a bounding box de um grupo de objetos da cena.
 * 
 * @param objects Vetor de objetos da cena.
 * @return A bounding box do grupo de objetos.
 */
AABB FindGroupBbox(std::vector<AABB> objects)
{

    float FLT_MAX = 3.402823466e+38F;

    // Inicializa a bounding box do grupo com valores extremos
    glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    // Para cada objeto na cena, atualiza a bounding box do grupo
    for (AABB obj : objects) {
        min = glm::min(min, obj.min);
        max = glm::max(max, obj.max);
    }

    return AABB{min, max};
}

/**
 * @brief Calcula o axis-aligned bounding box (AABB) para um objeto da cena em coordenadas de mundo.
 * 
 * @param obj O objeto da cena, que contém em sua struct uma AABB em coordenadas de modelo.
 * @param model A matriz modelo que representa as transformações geométricas necessárias para o objeto estar em coordenadas de mundo.
 * @return A AABB do objeto em coordenadas de mundo.
 */
AABB GetWorldAABB(SceneObject obj, glm::mat4 model)
{

    // Dada uma matriz model, transforma as coordenadas locais da AABB do objeto para coordenadas de mundo.
    glm::vec4 min = model * glm::vec4(obj.bbox_min.x, obj.bbox_min.y, obj.bbox_min.z, 1.0f);
    glm::vec4 max = model * glm::vec4(obj.bbox_max.x, obj.bbox_max.y, obj.bbox_max.z, 1.0f);

    // Quando há rotação, os valores mínimos e máximos dos pontos da bounding box podem ser corrigidos.
    for (int i = 0; i < 3; i++) {
        if (min[i] > max[i]) std::swap(min[i], max[i]);
    }
    
    // Retorna a bounding box em coordenadas de  mundo
    return AABB{min, max}; 
}

/**
 * @brief Projeta um ray casting a partir das coordenadas do mouse.
 * FONTE (adaptado): https://antongerdelan.net/opengl/raycasting.html
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

    return ray_world;

}