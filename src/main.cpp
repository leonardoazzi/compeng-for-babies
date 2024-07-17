//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   LABORATÓRIO 5
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
// #include <cstdio>
// #include <cstdlib>

// Headers abaixo são específicos de C++
#include <map>
#include <vector>
#include <limits>
#include <stdexcept>
#include <algorithm>


// Headers locais, definidos na pasta "include/"
#include "stackMatrix.h"
#include "textrendering.h"
// #include "callback.h"
#include "window.h"

#define M_PI 3.14159265358979323846

int main(int argc, char* argv[])
{
    initializeGLFW();

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    configureGLFW();

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "INF01047 - Seu Cartao - Seu Nome", NULL, NULL);
    createWindow(window);

    setCallbacks(window);

    printGPUinfo();

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    // Carregamos as texturas
    LoadTextureImage("../../data/tc-earth_daymap_surface.jpg");      // TextureImage0
    LoadTextureImage("../../data/lampada/textures/lightbulbOFF.png"); // TextureLightbulbOFF
    LoadTextureImage("../../data/lampada/textures/lightbulbON.png"); // TextureLightbulbON
    LoadTextureImage("../../data/table/chinese_console_table_diff_4k.jpg"); // TextureTable
    LoadTextureImage("../../data/cylinder/Metal009_4K-JPG_Color.jpg"); // TextureWire
    LoadTextureImage("../../data/display/textures/metal_plate_diff_4k.jpg"); // TextureDisplay
    LoadTextureImage("../../data/display/textures/digit0.jpg"); // TextureDigit0
    LoadTextureImage("../../data/display/textures/digit1.jpg"); // TextureDigit1
    LoadTextureImage("../../data/circuits/wire.jpg"); // TexturePlaneWire
    LoadTextureImage("../../data/circuits/not.jpg"); // TexturePlaneNot
    LoadTextureImage("../../data/circuits/metal_grate_rusty_diff_4k.jpg"); // TextureBlocks
    LoadTextureImage("../../data/circuits/leather_red_03_coll1_4k.png"); // TextureSphere
    LoadTextureImage("../../data/circuits/and.jpg"); // TexturePlaneAnd

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    buildModel("../../data/sphere.obj");
    buildModel("../../data/lampada/lightbulb_01_4k.obj");
    buildModel("../../data/and/and.obj");
    buildModel("../../data/cylinder/cylinder.obj");
    buildModel("../../data/display/cube.obj");
    buildModel("../../data/plane.obj");
    buildModel("../../data/table/chinese_console_table_4k.obj");
    buildModel("../../data/not/not.obj");
    buildModel("../../data/and/and.obj");

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
        //
        //           R     G     B     A
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(g_GpuProgramID);

        // Computamos a posição da câmera utilizando coordenadas esféricas.  As
        // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
        // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
        // e ScrollCallback().
        float r = g_CameraDistance;
        float y = r*sin(g_CameraPhi);
        float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

        // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
        // Veja slides 195-227 e 229-234 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::vec4 camera_position_c  = glm::vec4(x,y,z,1.0f); // Ponto "c", centro da câmera
        glm::vec4 camera_lookat_l    = glm::vec4(0.0f,0.0f,0.0f,1.0f); // Ponto "l", para onde a câmera (look-at) estará sempre olhando
        glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c; // Vetor "view", sentido para onde a câmera está virada
        glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); // Vetor "up" fixado para apontar para o "céu" (eito Y global)

        // Computamos a matriz "View" utilizando os parâmetros da câmera para
        // definir o sistema de coordenadas da câmera.  Veja slides 2-14, 184-190 e 236-242 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

        // Agora computamos a matriz de Projeção.
        glm::mat4 projection;

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearplane = -0.1f;  // Posição do "near plane"
        float farplane  = -10.0f; // Posição do "far plane"

        if (g_UsePerspectiveProjection)
        {
            // Projeção Perspectiva.
            // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
            float field_of_view = 3.141592 / 3.0f;
            projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        }
        else
        {
            // Projeção Ortográfica.
            // Para definição dos valores l, r, b, t ("left", "right", "bottom", "top"),
            // PARA PROJEÇÃO ORTOGRÁFICA veja slides 219-224 do documento Aula_09_Projecoes.pdf.
            // Para simular um "zoom" ortográfico, computamos o valor de "t"
            // utilizando a variável g_CameraDistance.
            float t = 1.5f*g_CameraDistance/2.5f;
            float b = -t;
            float r = t*g_ScreenRatio;
            float l = -r;
            projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
        }

        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        #define SPHERE 0
        #define LIGHTBULB_WIRE 1
        #define NOT  2
        #define AND  3
        #define WIRE  4
        #define DISPLAY 5
        #define TABLE 6
        #define INPUT1_DIGIT 7
        #define INPUT2_DIGIT 8
        #define PLANE_WIRE 9
        #define LIGHTBULB_NOT 10
        #define PLANE_NOT 11
        #define LIGHTBULB_AND 12
        #define PLANE_AND 13

        #define PLANE_WIDTH 0.2f
        #define PLANE_HEIGHT 0.145f
        #define DISPLAY_WIDTH (PLANE_WIDTH / 6.0f)
        #define DISPLAY_HEIGHT (PLANE_WIDTH / 4.0f)
        #define NUM_CIRCUITS 4
        #define CIRCUIT_WIDTH (0.75 * PLANE_WIDTH)

   
        // Guardamos matriz model atual na pilha
        PushMatrix(model);
            // Desenhamos o modelo da mesa
            model *= Matrix_Rotate_Z(M_PI/2.0f)
                * Matrix_Rotate_X(M_PI/2.0f)
                * Matrix_Rotate_Y(M_PI/2.0f);
                // * Matrix_Scale(3.2f,3.2f,3.2f);
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, TABLE);
            DrawVirtualObject("table");
        PopMatrix(model);

        // Variáveis usadas para calcular as alturas dos objetos e colocá-los em cima da mesa
        glm::vec3 bbox_min;
        glm::vec3 bbox_max;

        // Cálculo da altura da mesa
        bbox_min = g_VirtualScene["table"].bbox_min;
        bbox_max = g_VirtualScene["table"].bbox_max;
        glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
        glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);
        float tableHeight = bbox_max.z - bbox_min.z;
        float tableWidth = bbox_max.x - bbox_min.x;
        float tableDepth = bbox_max.y - bbox_min.y;

        // Cálculo da altura da lâmpada
        bbox_min = g_VirtualScene["lightbulb_01"].bbox_min;
        bbox_max = g_VirtualScene["lightbulb_01"].bbox_max;
        glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
        glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);
        float lightBulbHeight = bbox_max[1] - bbox_min[1];

        PushMatrix(model);
            // Posição da altura dos circuitos. 0.025 é por conta da altura da mesa corresponder à medida entre a base e a borda da mesa, que é mais alta que a parte onde os objetos estarão posicionados
            model = Matrix_Translate(0.0f, tableHeight - 0.025f, 0.0f);

            // 1 - WIRE

            // Desenhamos o circuito WIRE
            PushMatrix(model);

                model *= Matrix_Translate(- tableWidth / NUM_CIRCUITS - 0.15f, 0.0f, 0.0f);

                // Plano com o circuito WIRE
                PushMatrix(model);
                    model *= Matrix_Scale(PLANE_WIDTH, 0.0f, PLANE_HEIGHT);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, PLANE_WIRE);
                    DrawVirtualObject("the_plane");
                    
                PopMatrix(model);

                PushMatrix(model);
                    // desenhamos a lâmpada
                    model *= Matrix_Translate(CIRCUIT_WIDTH, 0.01f, 0.0f);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, LIGHTBULB_WIRE);
                    DrawVirtualObject("lightbulb_01");
                PopMatrix(model);

                PushMatrix(model);
                    // Desenhamos o modelo do fio
                    model *= Matrix_Translate(0.0f,0.01f,0.0f)
                        * Matrix_Rotate_X(M_PI/2.0f)
                        * Matrix_Rotate_Z(M_PI/2.0f)
                        * Matrix_Scale(0.01f, CIRCUIT_WIDTH, 0.01f);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, WIRE);
                    DrawVirtualObject("Cylinder");
                PopMatrix(model);

                PushMatrix(model);
                    // posição do display
                    model *= Matrix_Translate(-CIRCUIT_WIDTH, 0.0f, 0.0f);

                        PushMatrix(model);
                            // Desenhamos o modelo do cubo do display
                            model *= Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                            glUniform1i(g_object_id_uniform, DISPLAY);
                            DrawVirtualObject("Cube");
                        PopMatrix(model);

                        PushMatrix(model);
                            // // Desenhamos o modelo do display do dígito
                            model *= Matrix_Translate(0.0f, DISPLAY_WIDTH + 0.0005, 0.0f) 
                                * Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                            glUniform1i(g_object_id_uniform, INPUT1_DIGIT);
                            DrawVirtualObject("the_plane");
                        PopMatrix(model);

                PopMatrix(model);

            PopMatrix(model);

            // 2 - NOT

            // Desenhamos o circuito NOT
            PushMatrix(model);

                model *= Matrix_Translate(- (3 / NUM_CIRCUITS) * tableWidth - 0.15f, 0.0f, 0.0f);

                // Plano com o circuito NOT
                PushMatrix(model);
                    model *= Matrix_Scale(PLANE_WIDTH, 0.0f, PLANE_HEIGHT);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, PLANE_NOT);
                    DrawVirtualObject("the_plane");
                PopMatrix(model);

                PushMatrix(model);
                    // desenhamos a lâmpada
                    model *= Matrix_Translate(CIRCUIT_WIDTH, 0.01f, 0.0f);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, LIGHTBULB_NOT);
                    DrawVirtualObject("lightbulb_01");
                PopMatrix(model);

                PushMatrix(model);
                    // Desenhamos a primeira metade do modelo do fio
                    model *= Matrix_Translate(-(CIRCUIT_WIDTH - 0.05f), 0.01f,0.0f)
                        * Matrix_Rotate_X(M_PI/2.0f)
                        * Matrix_Rotate_Z(M_PI/2.0f)
                        * Matrix_Scale(0.01f, CIRCUIT_WIDTH / 4.0, 0.01f);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, WIRE);
                    DrawVirtualObject("Cylinder");
                PopMatrix(model);

                PushMatrix(model);
                    // Desenhamos a segunda metade do modelo do fio
                    model *= Matrix_Translate(CIRCUIT_WIDTH - 0.05f,0.01f,0.0f)
                        * Matrix_Rotate_X(M_PI/2.0f)
                        * Matrix_Rotate_Z(M_PI/2.0f)
                        * Matrix_Scale(0.01f, CIRCUIT_WIDTH / 4.0f, 0.01f);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, WIRE);
                    DrawVirtualObject("Cylinder");
                PopMatrix(model);

                PushMatrix(model);
                    // posição do display
                    model *= Matrix_Translate(-CIRCUIT_WIDTH, 0.0f, 0.0f);

                    PushMatrix(model);
                        // Desenhamos o modelo do cubo do display
                        model *= Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, DISPLAY);
                        DrawVirtualObject("Cube");
                    PopMatrix(model);

                    PushMatrix(model);
                        // // Desenhamos o modelo do display do dígito
                        model *= Matrix_Translate(0.0f, DISPLAY_WIDTH + 0.0005, 0.0f) 
                            * Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, INPUT1_DIGIT);
                        DrawVirtualObject("the_plane");
                    PopMatrix(model);

                    PushMatrix(model);
                        // Desenhamos o modelo do bloco NOT
                        model *= Matrix_Translate(CIRCUIT_WIDTH / 2.0f + 0.01f, 0.0f, 0.0f)
                            * Matrix_Scale(CIRCUIT_WIDTH / 4.0f + 0.025f, CIRCUIT_WIDTH / 4.0f, CIRCUIT_WIDTH / 4.0f)
                            * Matrix_Rotate_Y(-M_PI/4.0f);
                        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, NOT);
                        DrawVirtualObject("Not");
                    PopMatrix(model);

                    PushMatrix(model);
                        // Desenhamos o modelo da esfera do bloco NOT
                        model *= Matrix_Translate(CIRCUIT_WIDTH / 2.0f + 0.01f + CIRCUIT_WIDTH / 4.0f + 0.025f + CIRCUIT_WIDTH / 6.0f + 0.02f, 0.025f, 0.0f)
                            * Matrix_Scale(CIRCUIT_WIDTH / 6.0f, CIRCUIT_WIDTH / 6.0f,CIRCUIT_WIDTH / 6.0f)
                            * Matrix_Rotate_Y(-M_PI/4.0f);
                        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, SPHERE);
                        DrawVirtualObject("the_sphere");
                    PopMatrix(model);

                PopMatrix(model);
            PopMatrix(model);

            // 3 - AND

            // Desenhamos o circuito AND
            PushMatrix(model);

                model *= Matrix_Translate( tableWidth / NUM_CIRCUITS - 0.15f, 0.0f, 0.0f);

                // Plano com o circuito AND
                PushMatrix(model);
                    model *= Matrix_Scale(PLANE_WIDTH, 50.0f, PLANE_HEIGHT);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, PLANE_AND);
                    DrawVirtualObject("the_plane");
                PopMatrix(model);

                PushMatrix(model);
                    // desenhamos a lâmpada
                    model *= Matrix_Translate(CIRCUIT_WIDTH, 0.01f, 0.0f);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, LIGHTBULB_AND);
                    DrawVirtualObject("lightbulb_01");
                PopMatrix(model);

                PushMatrix(model);
                    // Desenhamos a primeira metade do modelo do fio
                    model *= Matrix_Translate(-(CIRCUIT_WIDTH - 0.05f), 0.01f,0.0f)
                        * Matrix_Rotate_X(M_PI/2.0f)
                        * Matrix_Rotate_Z(M_PI/2.0f)
                        * Matrix_Scale(0.01f, CIRCUIT_WIDTH / 4.0 + 0.005f, 0.01f);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, WIRE);
                    DrawVirtualObject("Cylinder");
                PopMatrix(model);

                PushMatrix(model);
                    // Desenhamos a segunda metade do modelo do fio
                    model *= Matrix_Translate(CIRCUIT_WIDTH - 0.05f,0.01f,0.0f)
                        * Matrix_Rotate_X(M_PI/2.0f)
                        * Matrix_Rotate_Z(M_PI/2.0f)
                        * Matrix_Scale(0.01f, CIRCUIT_WIDTH / 4.0f, 0.01f);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, WIRE);
                    DrawVirtualObject("Cylinder");
                PopMatrix(model);

                PushMatrix(model);
                    // Desenhamos a o fio vertical que liga os dos displays ao circuito
                    model *= Matrix_Translate(-CIRCUIT_WIDTH, 0.01f, 0.0f)
                        * Matrix_Rotate_Y(M_PI/2.0f)
                        * Matrix_Rotate_Z(M_PI/2.0f)
                        * Matrix_Scale(0.01f, PLANE_HEIGHT - (2 * DISPLAY_HEIGHT), 0.01f);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, WIRE);
                    DrawVirtualObject("Cylinder");
                PopMatrix(model);

                PushMatrix(model);
                    // posição do display
                    model *= Matrix_Translate(-CIRCUIT_WIDTH, 0.0f, 0.0f);

                    PushMatrix(model);
                        // Posição do display 1
                        model *= Matrix_Translate(0.0f, 0.0f, - (PLANE_HEIGHT / 2.0f));

                        PushMatrix(model);
                            // Desenhamos o modelo do cubo do display
                            model *= Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                            glUniform1i(g_object_id_uniform, DISPLAY);
                            DrawVirtualObject("Cube");
                        PopMatrix(model);

                        PushMatrix(model);
                            // // Desenhamos o modelo do display do dígito
                            model *= Matrix_Translate(0.0f, DISPLAY_WIDTH + 0.0005, 0.0f) 
                                * Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                            glUniform1i(g_object_id_uniform, INPUT1_DIGIT);
                            DrawVirtualObject("the_plane");
                        PopMatrix(model);

                        PushMatrix(model);
                            // Desenhamos o modelo do cubo do display
                            model *= Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                            glUniform1i(g_object_id_uniform, DISPLAY);
                            DrawVirtualObject("Cube");
                        PopMatrix(model);

                        PushMatrix(model);
                            // // Desenhamos o modelo do display do dígito
                            model *= Matrix_Translate(0.0f, DISPLAY_WIDTH + 0.0005, 0.0f) 
                                * Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                            glUniform1i(g_object_id_uniform, INPUT1_DIGIT);
                            DrawVirtualObject("the_plane");
                        PopMatrix(model);
                        
                    PopMatrix(model);

                    PushMatrix(model);
                        // Posição do display 2
                        model *= Matrix_Translate(0.0f, 0.0f, PLANE_HEIGHT / 2.0f);

                        PushMatrix(model);
                            // Desenhamos o modelo do cubo do display
                            model *= Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                            glUniform1i(g_object_id_uniform, DISPLAY);
                            DrawVirtualObject("Cube");
                        PopMatrix(model);

                        PushMatrix(model);
                            // // Desenhamos o modelo do display do dígito
                            model *= Matrix_Translate(0.0f, DISPLAY_WIDTH + 0.0005, 0.0f) 
                                * Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                            glUniform1i(g_object_id_uniform, INPUT2_DIGIT);
                            DrawVirtualObject("the_plane");
                        PopMatrix(model);

                        PushMatrix(model);
                            // Desenhamos o modelo do cubo do display
                            model *= Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                            glUniform1i(g_object_id_uniform, DISPLAY);
                            DrawVirtualObject("Cube");
                        PopMatrix(model);

                        PushMatrix(model);
                            // // Desenhamos o modelo do display do dígito
                            model *= Matrix_Translate(0.0f, DISPLAY_WIDTH + 0.0005, 0.0f) 
                                * Matrix_Scale(DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                            glUniform1i(g_object_id_uniform, INPUT1_DIGIT);
                            DrawVirtualObject("the_plane");
                        PopMatrix(model);
                        
                    PopMatrix(model);                
                
                PopMatrix(model);

                PushMatrix(model);
                    // Desenhamos o modelo do bloco AND
                    model *= Matrix_Translate(0.0f, 0.0f, 0.0f)
                        * Matrix_Scale(CIRCUIT_WIDTH / 4.0f + 0.01f, CIRCUIT_WIDTH / 4.0f + 0.01f, CIRCUIT_WIDTH / 4.0f + 0.01f)
                        * Matrix_Rotate_Y(-M_PI/2.0f);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, AND);
                    DrawVirtualObject("and");
                PopMatrix(model);

            PopMatrix(model);

        PopMatrix(model);


        glUniformMatrix4fv(g_view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

        // Desenhamos o plano do chão
        // model = Matrix_Translate(0.0f,-1.1f,0.0f) * Matrix_Scale(100.0f,100.0f,100.0f);
        // glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        // glUniform1i(g_object_id_uniform, PLANE);
        // DrawVirtualObject("the_plane");

        // Imprimimos na tela os ângulos de Euler que controlam a rotação do
        // terceiro cubo.
        TextRendering_ShowEulerAngles(window);

        // Imprimimos na informação sobre a matriz de projeção sendo utilizada.
        TextRendering_ShowProjection(window);

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}


// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :
