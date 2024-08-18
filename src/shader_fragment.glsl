#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

in vec4 colorWire; // Cor dos blocos para shading de Gourard

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 0
#define LIGHTBULB_WIRE 1
#define NOT 2
#define AND 3
#define WIRE 4
#define DISPLAY 5
#define TABLE 6
#define AND_INPUT1_DIGIT 7
#define AND_INPUT2_DIGIT 8
#define PLANE_WIRE 9
#define LIGHTBULB_NOT 10
#define PLANE_NOT 11
#define LIGHTBULB_AND 12
#define PLANE_AND 13
#define GROUND 14
#define OR 15
#define PLANE_OR 16
#define LIGHTBULB_OR 17
#define OR_INPUT2_DIGIT 18
#define OR_INPUT1_DIGIT 19
#define WIRE_INPUT1_DIGIT 20
#define NOT_INPUT1_DIGIT 21
#define SKY 22

uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureLightbulbOFF;
uniform sampler2D TextureLightbulbON;
uniform sampler2D TextureTable;
uniform sampler2D TextureDisplay;
uniform sampler2D TextureDigit0;
uniform sampler2D TextureDigit1;
uniform sampler2D TexturePlaneWire;
uniform sampler2D TexturePlaneNot;
uniform sampler2D TextureSphere;
uniform sampler2D TexturePlaneAnd;
uniform sampler2D TextureBlocks;
uniform sampler2D TextureFloor;
uniform sampler2D TexturePlaneOr;
uniform sampler2D TextureSky;

uniform bool u_andIsInput1Digit0;
uniform bool u_andIsInput2Digit0;
uniform bool u_orIsInput1Digit0;
uniform bool u_orIsInput2Digit0;
uniform bool u_notIsInputDigit0;
uniform bool u_wireIsInputDigit0;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = v;

    // Vetor que define o sentido da reflexão especular ideal.
    vec4 r = -l + 2 * n * dot(n, l);

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

    // Variáveis usadas para os modelos de iluminação
    vec3 Kd, Ia, Ka, Ks, I;
    float lambert, q;
    vec3 lambertDiffuseTerm, ambientTerm, specularTerm;

    // Definição dos coeficientes de reflexão da superfície
    Ka  = vec3(0.05,0.05,0.05); // coeficiente de reflexão ambiente
    Ks  = vec3(0.5,0.5,0.5); // coeficiente de reflexão especular
    Ia  = vec3(0.2,0.2,0.2); // intensidade da luz ambiente
    I   = vec3(1.0,1.0,1.0); // intensidade da luz

    lambert = max(0,dot(n,l));
    q = 100.0;

    vec4 h = normalize(l+v);
    ambientTerm = Ka * Ia;
    specularTerm = Ks * I * pow(dot(n, h),q);

    if ( object_id == SPHERE ) // Blinn-Phong e Phong shading
    {
        // PREENCHA AQUI as coordenadas de textura da esfera, computadas com
        // projeção esférica EM COORDENADAS DO MODELO. Utilize como referência
        // o slides 134-150 do documento Aula_20_Mapeamento_de_Texturas.pdf.
        // A esfera que define a projeção deve estar centrada na posição
        // "bbox_center" definida abaixo.

        // Você deve utilizar:
        //   função 'length( )' : comprimento Euclidiano de um vetor
        //   função 'atan( , )' : arcotangente. Veja https://en.wikipedia.org/wiki/Atan2.
        //   função 'asin( )'   : seno inverso.
        //   constante M_PI
        //   variável position_model

        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        float radius = length(bbox_max - bbox_center);

        vec4 position_sphere = bbox_center + radius * normalize(position_model - bbox_center);

        float theta = atan(position_sphere.x, position_sphere.z);
        float phi = asin(position_sphere.y / radius);

        U = (theta + M_PI) / (2.0 * M_PI);
        V = (phi + M_PI / 2) / M_PI;

        // Obtemos a refletância difusa a partir da leitura da imagem TextureSphere
        Kd = texture(TextureSphere, vec2(U,V)).rgb;

        lambertDiffuseTerm = Kd * I * lambert;

        color.rgb = lambertDiffuseTerm + ambientTerm + specularTerm; // Blinn-Phong
    }
    else if ( object_id == SKY ) // Totalmente difusa e Phong shading
    {
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        float radius = length(bbox_max - bbox_center);

        vec4 position_sphere = bbox_center + radius * normalize(position_model - bbox_center);

        float theta = atan(position_sphere.x, position_sphere.z);
        float phi = asin(position_sphere.y / radius);

        U = (theta + M_PI) / (2.0 * M_PI);
        V = 1.0 - (phi + M_PI / 2) / M_PI;

        U = U * 5.0f;
        V = V * 5.0f;

        // Obtemos a refletância difusa a partir da leitura da imagem TextureSphere
        Kd = texture(TextureSky, vec2(U,V)).rgb;

        color.rgb = Kd;
    }
    else if ( object_id == LIGHTBULB_WIRE ) // ON=Blinn-Phong, OFF=Diffuse, Phong shading
    {
        if (!u_wireIsInputDigit0) {
            Kd = texture(TextureLightbulbON, texcoords).rgb;
            lambertDiffuseTerm = Kd * I * lambert;
            color.rgb = lambertDiffuseTerm + ambientTerm + specularTerm; // Blinn-Phong
        }
        else {
            Kd = texture(TextureLightbulbOFF, texcoords).rgb;
            lambertDiffuseTerm = Kd * I * lambert;
            color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
        }
    }
    else if ( object_id == LIGHTBULB_NOT ) // ON=Blinn-Phong e Phong shading, OFF=Diffuse e Gouraud shading
    {
        if (u_notIsInputDigit0) {
            Kd = texture(TextureLightbulbON, texcoords).rgb;
            lambertDiffuseTerm = Kd * I * lambert;
            color.rgb = lambertDiffuseTerm + ambientTerm + specularTerm; // Blinn-Phong
        }
        else {
            Kd = texture(TextureLightbulbOFF, texcoords).rgb;
            lambertDiffuseTerm = Kd * I * lambert;
            color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
        }
    }
    else if ( object_id == LIGHTBULB_AND ) // ON=Blinn-Phong e Phong shading, OFF=Diffuse e Gouraud shading
    {
        if (!u_andIsInput1Digit0 && !u_andIsInput2Digit0) {
            Kd = texture(TextureLightbulbON, texcoords).rgb;
            lambertDiffuseTerm = Kd * I * lambert;
            color.rgb = lambertDiffuseTerm + ambientTerm + specularTerm; // Blinn-Phong
        }
        else {
            Kd = texture(TextureLightbulbOFF, texcoords).rgb;
            lambertDiffuseTerm = Kd * I * lambert;
            color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
        }
    }
    else if ( object_id == LIGHTBULB_OR ) // ON=Blinn-Phong e Phong shading, OFF=Diffuse e Gouraud shading
    {
        if (!u_orIsInput1Digit0 || !u_orIsInput2Digit0) {
            Kd = texture(TextureLightbulbON, texcoords).rgb;
            lambertDiffuseTerm = Kd * I * lambert;
            color.rgb = lambertDiffuseTerm + ambientTerm + specularTerm; // Blinn-Phong
        }
        else {
            Kd = texture(TextureLightbulbOFF, texcoords).rgb;
            lambertDiffuseTerm = Kd * I * lambert;
            color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
        }
    }
    else if (object_id == TABLE) // Diffuse e Phong shading
    {
        Kd = texture(TextureTable, texcoords).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
    }
    else if (object_id == WIRE) // Blinn-Phong e Gourard shading
    {
        color = colorWire; // Cor resultante do shading de Gourard
    }
    else if (object_id == DISPLAY) // Blinn-Phong e Phong shading
    {
        Kd = texture(TextureDisplay, texcoords).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm + specularTerm; // Blinn-Phong
    }
    else if (object_id == GROUND) // Blinn-Phong e Phong shading
    {
        // Forçamos que as coord. de textura saiam do intervalo [0,1]
        // para aplicar o texture wrappng GL_REPEAT
        U = texcoords.x * 20.0f;
        V = texcoords.y * 20.0f;
        Kd = texture(TextureFloor, vec2(U,V)).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Blinn-Phong
    }
    else if (object_id == AND_INPUT1_DIGIT) // Diffuse e Phong shading
    {
        if (u_andIsInput1Digit0)
            Kd = texture(TextureDigit0, texcoords).rgb;
        else
            Kd = texture(TextureDigit1, texcoords).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
    }
    else if (object_id == AND_INPUT2_DIGIT) // Diffuse e Phong shading
    {
        if (u_andIsInput2Digit0)
            Kd = texture(TextureDigit0, texcoords).rgb;
        else
            Kd = texture(TextureDigit1, texcoords).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
    }
    else if (object_id == OR_INPUT1_DIGIT) // Diffuse e Phong shading
    {
        if (u_orIsInput1Digit0)
            Kd = texture(TextureDigit0, texcoords).rgb;
        else
            Kd = texture(TextureDigit1, texcoords).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
    }
    else if (object_id == OR_INPUT2_DIGIT) // Diffuse e Phong shading
    {
        if (u_orIsInput2Digit0)
            Kd = texture(TextureDigit0, texcoords).rgb;
        else
            Kd = texture(TextureDigit1, texcoords).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
    }
    else if (object_id == NOT_INPUT1_DIGIT) // Diffuse e Phong shading
    {
        if (u_notIsInputDigit0)
            Kd = texture(TextureDigit0, texcoords).rgb;
        else
            Kd = texture(TextureDigit1, texcoords).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
    }
    else if (object_id == WIRE_INPUT1_DIGIT) // Diffuse e Phong shading
    {
        if (u_wireIsInputDigit0)
            Kd = texture(TextureDigit0, texcoords).rgb;
        else
            Kd = texture(TextureDigit1, texcoords).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
    }
    else if (object_id == PLANE_WIRE) // Diffuse e Phong shading
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd = texture(TexturePlaneWire, vec2(U,V)).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
    }
    else if (object_id == PLANE_NOT) // Diffuse e Phong shading
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd = texture(TexturePlaneNot, vec2(U,V)).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
    }
    else if (object_id == PLANE_AND) // Diffuse e Phong shading
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd = texture(TexturePlaneAnd, vec2(U,V)).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
    }
    else if (object_id == PLANE_OR) // Diffuse e Phong shading
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd = texture(TexturePlaneOr, vec2(U,V)).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm; // Diffuse
    }
    else if (object_id == AND || object_id == OR) // Blinn-Phong e Phong shading
    {
        U = texcoords.x * 2.0f;
        V = texcoords.y * 2.0f;
        Kd = texture(TextureBlocks, vec2(U,V)).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm + specularTerm; // Blinn-Phong
    }
    else if (object_id == NOT) // Blinn-Phong e Phong shading
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd = texture(TextureBlocks, vec2(U,V)).rgb;
        lambertDiffuseTerm = Kd * I * lambert;
        color.rgb = lambertDiffuseTerm + ambientTerm + specularTerm; // Blinn-Phong
    }

    // NOTE: Se você quiser fazer o rendering de objetos transparentes, é
    // necessário:
    // 1) Habilitar a operação de "blending" de OpenGL logo antes de realizar o
    //    desenho dos objetos transparentes, com os comandos abaixo no código C++:
    //      glEnable(GL_BLEND);
    //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 2) Realizar o desenho de todos objetos transparentes *após* ter desenhado
    //    todos os objetos opacos; e
    // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
    //    suas distâncias para a câmera (desenhando primeiro objetos
    //    transparentes que estão mais longe da câmera).
    // Alpha default = 1 = 100% opaco = 0% transparente
    color.a = 1;

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
} 
