#pragma once

#include <map>
#include <stack>
#include <string>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include <tiny_obj_loader.h>

#include <stb_image.h>

#include "utils.h"
#include "matrices.h"

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Se basepath == NULL, então setamos basepath como o dirname do
        // filename, para que os arquivos MTL sejam corretamente carregados caso
        // estejam no mesmo diretório dos arquivos OBJ.
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                    filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }

        printf("OK.\n");
    }
};

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
};

/**
 * @brief Struct que representa uma axis-aligned bounding box (AABB).
 * 
 * Uma AABB (Axis-Aligned Bounding Box) é utilizada para representar uma caixa delimitadora
 * que possui seus lados paralelos aos eixos do sistema de coordenadas. Ela é definida pelos pontos
 * mínimo e máximo que a delimitam.
 */
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

struct GameObject {
    std::string name;
    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec3 rotation;
    SceneObject sceneObject;
    bool isHovered;
    AABB bbox;
};

/**
 * @brief Struct que representa uma esfera.
 * 
 * Contém as informações necessárias para representar uma esfera no espaço tridimensional.
 * Ela armazena o centro da esfera e o raio da mesma.
 */
struct Sphere {
    glm::vec3 center;
    float radius;
};


// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
extern std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
extern std::stack<glm::mat4>  g_MatrixStack;

// Largura e altura da janela de renderização.
extern float g_ScreenWidth;
extern float g_ScreenHeight;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
extern float g_ScreenRatio;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
extern float g_AngleX;
extern float g_AngleY;
extern float g_AngleZ;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
extern bool g_LeftMouseButtonPressed;
extern bool g_RightMouseButtonPressed; // Análogo para botão direito do mouse
extern bool g_MiddleMouseButtonPressed; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
extern float g_CameraTheta; // Ângulo no plano ZX em relação ao eixo Z
extern float g_CameraPhi;   // Ângulo em relação ao eixo Y
extern float g_CameraDistance; // Distância da câmera para a origem

// Variáveis que controlam rotação do antebraço
extern float g_ForearmAngleZ;
extern float g_ForearmAngleX;

// Variáveis que controlam translação do torso
extern float g_TorsoPositionX;
extern float g_TorsoPositionY;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
extern bool g_UsePerspectiveProjection;

// Variável que controla se o texto informativo será mostrado na tela.
extern bool g_ShowInfoText;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
extern GLuint g_GpuProgramID;
extern GLint g_model_uniform;
extern GLint g_view_uniform;
extern GLint g_projection_uniform;
extern GLint g_object_id_uniform;
extern GLint g_bbox_min_uniform;
extern GLint g_bbox_max_uniform;

// Número de texturas carregadas pela função LoadTextureImage()
extern GLuint g_NumLoadedTextures;

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
extern double g_LastCursorPosX, g_LastCursorPosY;

// Variáveis que mudam o dígito de cada display
extern bool andIsInput1Digit0;
extern bool andIsInput2Digit0;
extern bool wireIsInputDigit0;
extern bool notIsInputDigit0;
extern bool orIsInput1Digit0;
extern bool orIsInput2Digit0;

// Variáveis de estado para as teclas de movimentação da câmera
extern bool W_key_pressed;
extern bool A_key_pressed;
extern bool S_key_pressed;
extern bool D_key_pressed;

// Variável para modificar o tipo de câmera (look-at/free camera)
extern bool freeCamera;
extern bool lookatCamera;

extern glm::vec3 g_rayPoint;