#include "terrain.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <math.h>
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"


void generateTerrainMesh(aiMesh* mesh);
void generateTerrain(std::vector<float>& vertices, std::vector<unsigned int>& indices);
void generateTerrainArray(float* vertices, unsigned int* indices, unsigned int maxVertices, unsigned int maxIndices);
extern float stb_perlin_noise3(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap);
extern float stb_perlin_noise3_seed(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap, int seed);
extern float stb_perlin_ridge_noise3(float x, float y, float z, float lacunarity, float gain, float offset, int octaves);
extern float stb_perlin_fbm_noise3(float x, float y, float z, float lacunarity, float gain, int octaves);
extern float stb_perlin_turbulence_noise3(float x, float y, float z, float lacunarity, float gain, int octaves);
extern float stb_perlin_noise3_wrap_nonpow2(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap, unsigned char seed);





// Rozmiar siatki
const unsigned int gridSize = 100;
const float gridSpacing = 1.0f;


//Core::Shader_Loader shaderLoader;


Terrain::Terrain()
{
    generateTerrainMesh(&this->mesh);
    this->context.initFromAssimpMesh(&this->mesh);
    //glGenVertexArrays(1, &this->context.vertexArray);
    //glGenBuffers(1, &this->context.vertexBuffer);
    //glGenBuffers(1, &this->context.vertexIndexBuffer);
    //
    //this->vertices = new float[gridSize * gridSize * 3];
    //this->verticesNumber = gridSize * gridSize;
    //this->indices = new unsigned int[gridSize * gridSize * 6];    //(2*((gridSize - 1) * gridSize) + (gridSize * gridSize)) * 2

    //this->context.size = gridSize * gridSize * 6;

    ////this->vertices = float[30000];

    ///*std::vector<float> vertices;
    //std::vector<unsigned int> ind;
    //generateTerrain(vertices, ind);*/
    //generateTerrainArray(this->vertices, this->indices, gridSize * gridSize * 3, gridSize * gridSize * 6);

    //glBindVertexArray(this->context.vertexArray);
    //// 2. copy our vertices array in a vertex buffer for OpenGL to use
    //glBindBuffer(GL_ARRAY_BUFFER, this->context.vertexBuffer);
    //glBufferData(GL_ARRAY_BUFFER, this->verticesNumber * 3 * sizeof(float), this->vertices, GL_STATIC_DRAW);
    //// 3. copy our index array in a element buffer for OpenGL to use
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->context.vertexIndexBuffer);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->verticesNumber * 6 * sizeof(unsigned int), this->indices, GL_STATIC_DRAW);
    //// 4. then set the vertex attributes pointers
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(0);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0);

    //this->program = shaderLoader.CreateProgram("shaders/terrain.vert", "shaders/terrain.frag");
}
//void Terrain::render(glm::mat4 cameraMatrix, glm::mat4 modelMatrix, glm::mat4 perspectiveMatrix)
//{
//    glUniformMatrix4fv(glGetUniformLocation(this->program, "view"), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
//    glUniformMatrix4fv(glGetUniformLocation(this->program, "projection"), 1, GL_FALSE, glm::value_ptr(perspectiveMatrix));
//    glUniformMatrix4fv(glGetUniformLocation(this->program, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
//    glUseProgram(this->program);
//    glBindVertexArray(this->context.vertexArray);
//    glDrawElements(GL_TRIANGLES, this->verticesNumber * 6, GL_UNSIGNED_INT, 0);
//    glBindVertexArray(0);
//}

// Funkcja generuj�ca dane terenu
void generateTerrainMesh(aiMesh* mesh) 
{
    unsigned int vertexIndex = 0;
    unsigned int indexIndex = 0;

    mesh->mNumVertices = gridSize * gridSize;
    mesh->mVertices = new aiVector3D[mesh->mNumVertices];
    mesh->mNormals = new aiVector3D[mesh->mNumVertices];
    mesh->mTangents = new aiVector3D[mesh->mNumVertices];
    mesh->mBitangents = new aiVector3D[mesh->mNumVertices];

    mesh->mTextureCoords[0] = nullptr;

    mesh->mNumFaces = (gridSize - 1) * (gridSize - 1) * 2;
    mesh->mFaces = new aiFace[mesh->mNumFaces];

    for (unsigned int z = 0; z < gridSize; z++) {
        for (unsigned int x = 0; x < gridSize; x++) {


            float height = stb_perlin_noise3(x * 0.1f, z * 0.1f, 0.0f, 0, 0, 0);

            // Wsp�rz�dne (x, y, z)
            aiVector3D vec;
            vec.x = x * gridSpacing;
            vec.y = height * 5.0f;
            vec.z = z * gridSpacing;
            mesh->mVertices[vertexIndex] = vec;

            // Obliczanie normalnej jako iloczyn wektorowy wektor�w s�siednich
            float hL = stb_perlin_noise3((x - 1) * 0.1f, z * 0.1f, 0.0f, 0, 0, 0) * 5.0f; // wysoko�� w lewo
            float hR = stb_perlin_noise3((x + 1) * 0.1f, z * 0.1f, 0.0f, 0, 0, 0) * 5.0f; // wysoko�� w prawo
            float hD = stb_perlin_noise3(x * 0.1f, (z - 1) * 0.1f, 0.0f, 0, 0, 0) * 5.0f; // wysoko�� w d�
            float hU = stb_perlin_noise3(x * 0.1f, (z + 1) * 0.1f, 0.0f, 0, 0, 0) * 5.0f; // wysoko�� w g�r�

            aiVector3D dx(2.0f * gridSpacing, hR - hL, 0.0f); // Wektor r�nicy w osi X
            aiVector3D dz(0.0f, hU - hD, 2.0f * gridSpacing); // Wektor r�nicy w osi Z

            // Normalna = iloczyn wektorowy dx � dz
            aiVector3D normal = dx ^ dz; // Operator ^ w Assimp to iloczyn wektorowy
            normal.Normalize(); // Normalizacja wektora normalnego

            mesh->mNormals[vertexIndex] = normal;


            //std::cout << vec.x << "  " << vec.y << "  " << vec.z << std::endl;
            vertexIndex++;
        }
    }

    unsigned int* indices = new unsigned int[(gridSize - 1) * (gridSize - 1) * 6];

    vertexIndex = 0;
    for (unsigned int z = 0; z < gridSize - 1; z++) {
        for (unsigned int x = 0; x < gridSize - 1; x++) {

            unsigned int topLeft = vertexIndex + gridSize;
            unsigned int topRight = vertexIndex + gridSize + 1;
            unsigned int bottomLeft = vertexIndex;
            unsigned int bottomRight = vertexIndex + 1;

            indices[indexIndex++] = bottomLeft;
            indices[indexIndex++] = bottomRight;
            indices[indexIndex++] = topRight;

            indices[indexIndex++] = bottomLeft;
            indices[indexIndex++] = topRight;
            indices[indexIndex++] = topLeft;

            /*unsigned int topLeft = z * (gridSize + 1) + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * (gridSize + 1) + x;
            unsigned int bottomRight = bottomLeft + 1;

            indices[indexIndex++] = topLeft;
            indices[indexIndex++] = bottomLeft;
            indices[indexIndex++] = topRight;

            indices[indexIndex++] = topRight;
            indices[indexIndex++] = bottomLeft;
            indices[indexIndex++] = bottomRight;*/

            //std::cout << indices[indexIndex - 6] << " " << indices[indexIndex - 5] << " " << indices[indexIndex - 4] << "  " << indices[indexIndex - 3] << " " << indices[indexIndex - 2] << " " << indices[indexIndex - 1] << std::endl;
            vertexIndex++;
        }
        vertexIndex++;
    }
    for (size_t i = 0; i < mesh->mNumFaces; ++i) {
        mesh->mFaces[i].mNumIndices = 3;
        mesh->mFaces[i].mIndices = new unsigned int[3];
        mesh->mFaces[i].mIndices[0] = indices[i * 3 + 0];
        mesh->mFaces[i].mIndices[1] = indices[i * 3 + 1];
        mesh->mFaces[i].mIndices[2] = indices[i * 3 + 2];
    }
}








void generateTerrainArray(float* vertices, unsigned int* indices, unsigned int maxVertices, unsigned int maxIndices) {
    unsigned int vertexIndex = 0;
    unsigned int indexIndex = 0;

    for (unsigned int z = 0; z < gridSize; z++) {
        for (unsigned int x = 0; x < gridSize; x++) {
            

            float height = stb_perlin_noise3(x * 0.1f, z * 0.1f, 0.0f, 0, 0, 0);

            // Wsp�rz�dne (x, y, z)
            vertices[vertexIndex++] = x * gridSpacing;
            vertices[vertexIndex++] = height * 5.0f;
            vertices[vertexIndex++] = z * gridSpacing;

            // Wektor normalny
            //vertices[vertexIndex++] = 0.0f;
            //vertices[vertexIndex++] = 1.0f;
            //vertices[vertexIndex++] = 0.0f;

            //std::cout << vertices[vertexIndex - 3] << "  " << vertices[vertexIndex - 2] << "  " << vertices[vertexIndex - 1] << std::endl;
        }
    }

    for (unsigned int z = 0; z < gridSize; ++z) {
        for (unsigned int x = 0; x < gridSize; ++x) {

            unsigned int topLeft = z * (gridSize + 1) + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * (gridSize + 1) + x;
            unsigned int bottomRight = bottomLeft + 1;

            indices[indexIndex++] = topLeft;
            indices[indexIndex++] = bottomLeft;
            indices[indexIndex++] = topRight;

            indices[indexIndex++] = topRight;
            indices[indexIndex++] = bottomLeft;
            indices[indexIndex++] = bottomRight;

            //std::cout << indices[indexIndex - 6] << " " << indices[indexIndex - 5] << " " << indices[indexIndex - 4] << "  " << indices[indexIndex - 3] << " " << indices[indexIndex - 2] << " " << indices[indexIndex - 1] << std::endl;
        }
    }
}
void generateTerrain(std::vector<float>& vertices, std::vector<unsigned int>& indices) 
{
    for (unsigned int z = 0; z <= gridSize; ++z) {
        for (unsigned int x = 0; x <= gridSize; ++x) {
            // Wsp�rz�dne (x, y, z)
            float height = stb_perlin_noise3(x * 0.1f, z * 0.1f, 0.0f, 0, 0, 0); // Szum Perlin'a
            vertices.push_back(x * gridSpacing);  // x
            vertices.push_back(height * 5.0f);    // y (wysoko��)
            vertices.push_back(z * gridSpacing);  // z

            // Wektor normalny (ustawiony domy�lnie)
            //vertices.push_back(0.0f); // nx
            //vertices.push_back(1.0f); // ny
            //vertices.push_back(0.0f); // nz
        }
    }

    // Tworzenie indeks�w do rysowania siatki
    for (unsigned int z = 0; z < gridSize; ++z) {
        for (unsigned int x = 0; x < gridSize; ++x) {
            unsigned int topLeft = z * (gridSize + 1) + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * (gridSize + 1) + x;
            unsigned int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}


 // fabs()

// not same permutation table as Perlin's reference to avoid copyright issues;
// Perlin's table can be found at http://mrl.nyu.edu/~perlin/noise/
static unsigned char stb__perlin_randtab[512] =
{
   23, 125, 161, 52, 103, 117, 70, 37, 247, 101, 203, 169, 124, 126, 44, 123,
   152, 238, 145, 45, 171, 114, 253, 10, 192, 136, 4, 157, 249, 30, 35, 72,
   175, 63, 77, 90, 181, 16, 96, 111, 133, 104, 75, 162, 93, 56, 66, 240,
   8, 50, 84, 229, 49, 210, 173, 239, 141, 1, 87, 18, 2, 198, 143, 57,
   225, 160, 58, 217, 168, 206, 245, 204, 199, 6, 73, 60, 20, 230, 211, 233,
   94, 200, 88, 9, 74, 155, 33, 15, 219, 130, 226, 202, 83, 236, 42, 172,
   165, 218, 55, 222, 46, 107, 98, 154, 109, 67, 196, 178, 127, 158, 13, 243,
   65, 79, 166, 248, 25, 224, 115, 80, 68, 51, 184, 128, 232, 208, 151, 122,
   26, 212, 105, 43, 179, 213, 235, 148, 146, 89, 14, 195, 28, 78, 112, 76,
   250, 47, 24, 251, 140, 108, 186, 190, 228, 170, 183, 139, 39, 188, 244, 246,
   132, 48, 119, 144, 180, 138, 134, 193, 82, 182, 120, 121, 86, 220, 209, 3,
   91, 241, 149, 85, 205, 150, 113, 216, 31, 100, 41, 164, 177, 214, 153, 231,
   38, 71, 185, 174, 97, 201, 29, 95, 7, 92, 54, 254, 191, 118, 34, 221,
   131, 11, 163, 99, 234, 81, 227, 147, 156, 176, 17, 142, 69, 12, 110, 62,
   27, 255, 0, 194, 59, 116, 242, 252, 19, 21, 187, 53, 207, 129, 64, 135,
   61, 40, 167, 237, 102, 223, 106, 159, 197, 189, 215, 137, 36, 32, 22, 5,

   // and a second copy so we don't need an extra mask or static initializer
   23, 125, 161, 52, 103, 117, 70, 37, 247, 101, 203, 169, 124, 126, 44, 123,
   152, 238, 145, 45, 171, 114, 253, 10, 192, 136, 4, 157, 249, 30, 35, 72,
   175, 63, 77, 90, 181, 16, 96, 111, 133, 104, 75, 162, 93, 56, 66, 240,
   8, 50, 84, 229, 49, 210, 173, 239, 141, 1, 87, 18, 2, 198, 143, 57,
   225, 160, 58, 217, 168, 206, 245, 204, 199, 6, 73, 60, 20, 230, 211, 233,
   94, 200, 88, 9, 74, 155, 33, 15, 219, 130, 226, 202, 83, 236, 42, 172,
   165, 218, 55, 222, 46, 107, 98, 154, 109, 67, 196, 178, 127, 158, 13, 243,
   65, 79, 166, 248, 25, 224, 115, 80, 68, 51, 184, 128, 232, 208, 151, 122,
   26, 212, 105, 43, 179, 213, 235, 148, 146, 89, 14, 195, 28, 78, 112, 76,
   250, 47, 24, 251, 140, 108, 186, 190, 228, 170, 183, 139, 39, 188, 244, 246,
   132, 48, 119, 144, 180, 138, 134, 193, 82, 182, 120, 121, 86, 220, 209, 3,
   91, 241, 149, 85, 205, 150, 113, 216, 31, 100, 41, 164, 177, 214, 153, 231,
   38, 71, 185, 174, 97, 201, 29, 95, 7, 92, 54, 254, 191, 118, 34, 221,
   131, 11, 163, 99, 234, 81, 227, 147, 156, 176, 17, 142, 69, 12, 110, 62,
   27, 255, 0, 194, 59, 116, 242, 252, 19, 21, 187, 53, 207, 129, 64, 135,
   61, 40, 167, 237, 102, 223, 106, 159, 197, 189, 215, 137, 36, 32, 22, 5,
};


// perlin's gradient has 12 cases so some get used 1/16th of the time
// and some 2/16ths. We reduce bias by changing those fractions
// to 5/64ths and 6/64ths

// this array is designed to match the previous implementation
// of gradient hash: indices[stb__perlin_randtab[i]&63]
static unsigned char stb__perlin_randtab_grad_idx[512] =
{
    7, 9, 5, 0, 11, 1, 6, 9, 3, 9, 11, 1, 8, 10, 4, 7,
    8, 6, 1, 5, 3, 10, 9, 10, 0, 8, 4, 1, 5, 2, 7, 8,
    7, 11, 9, 10, 1, 0, 4, 7, 5, 0, 11, 6, 1, 4, 2, 8,
    8, 10, 4, 9, 9, 2, 5, 7, 9, 1, 7, 2, 2, 6, 11, 5,
    5, 4, 6, 9, 0, 1, 1, 0, 7, 6, 9, 8, 4, 10, 3, 1,
    2, 8, 8, 9, 10, 11, 5, 11, 11, 2, 6, 10, 3, 4, 2, 4,
    9, 10, 3, 2, 6, 3, 6, 10, 5, 3, 4, 10, 11, 2, 9, 11,
    1, 11, 10, 4, 9, 4, 11, 0, 4, 11, 4, 0, 0, 0, 7, 6,
    10, 4, 1, 3, 11, 5, 3, 4, 2, 9, 1, 3, 0, 1, 8, 0,
    6, 7, 8, 7, 0, 4, 6, 10, 8, 2, 3, 11, 11, 8, 0, 2,
    4, 8, 3, 0, 0, 10, 6, 1, 2, 2, 4, 5, 6, 0, 1, 3,
    11, 9, 5, 5, 9, 6, 9, 8, 3, 8, 1, 8, 9, 6, 9, 11,
    10, 7, 5, 6, 5, 9, 1, 3, 7, 0, 2, 10, 11, 2, 6, 1,
    3, 11, 7, 7, 2, 1, 7, 3, 0, 8, 1, 1, 5, 0, 6, 10,
    11, 11, 0, 2, 7, 0, 10, 8, 3, 5, 7, 1, 11, 1, 0, 7,
    9, 0, 11, 5, 10, 3, 2, 3, 5, 9, 7, 9, 8, 4, 6, 5,

    // and a second copy so we don't need an extra mask or static initializer
    7, 9, 5, 0, 11, 1, 6, 9, 3, 9, 11, 1, 8, 10, 4, 7,
    8, 6, 1, 5, 3, 10, 9, 10, 0, 8, 4, 1, 5, 2, 7, 8,
    7, 11, 9, 10, 1, 0, 4, 7, 5, 0, 11, 6, 1, 4, 2, 8,
    8, 10, 4, 9, 9, 2, 5, 7, 9, 1, 7, 2, 2, 6, 11, 5,
    5, 4, 6, 9, 0, 1, 1, 0, 7, 6, 9, 8, 4, 10, 3, 1,
    2, 8, 8, 9, 10, 11, 5, 11, 11, 2, 6, 10, 3, 4, 2, 4,
    9, 10, 3, 2, 6, 3, 6, 10, 5, 3, 4, 10, 11, 2, 9, 11,
    1, 11, 10, 4, 9, 4, 11, 0, 4, 11, 4, 0, 0, 0, 7, 6,
    10, 4, 1, 3, 11, 5, 3, 4, 2, 9, 1, 3, 0, 1, 8, 0,
    6, 7, 8, 7, 0, 4, 6, 10, 8, 2, 3, 11, 11, 8, 0, 2,
    4, 8, 3, 0, 0, 10, 6, 1, 2, 2, 4, 5, 6, 0, 1, 3,
    11, 9, 5, 5, 9, 6, 9, 8, 3, 8, 1, 8, 9, 6, 9, 11,
    10, 7, 5, 6, 5, 9, 1, 3, 7, 0, 2, 10, 11, 2, 6, 1,
    3, 11, 7, 7, 2, 1, 7, 3, 0, 8, 1, 1, 5, 0, 6, 10,
    11, 11, 0, 2, 7, 0, 10, 8, 3, 5, 7, 1, 11, 1, 0, 7,
    9, 0, 11, 5, 10, 3, 2, 3, 5, 9, 7, 9, 8, 4, 6, 5,
};

static float stb__perlin_lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

static int stb__perlin_fastfloor(float a)
{
    int ai = (int)a;
    return (a < ai) ? ai - 1 : ai;
}

// different grad function from Perlin's, but easy to modify to match reference
static float stb__perlin_grad(int grad_idx, float x, float y, float z)
{
    static float basis[12][4] =
    {
       {  1, 1, 0 },
       { -1, 1, 0 },
       {  1,-1, 0 },
       { -1,-1, 0 },
       {  1, 0, 1 },
       { -1, 0, 1 },
       {  1, 0,-1 },
       { -1, 0,-1 },
       {  0, 1, 1 },
       {  0,-1, 1 },
       {  0, 1,-1 },
       {  0,-1,-1 },
    };

    float* grad = basis[grad_idx];
    return grad[0] * x + grad[1] * y + grad[2] * z;
}

float stb_perlin_noise3_internal(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap, unsigned char seed)
{
    float u, v, w;
    float n000, n001, n010, n011, n100, n101, n110, n111;
    float n00, n01, n10, n11;
    float n0, n1;

    unsigned int x_mask = (x_wrap - 1) & 255;
    unsigned int y_mask = (y_wrap - 1) & 255;
    unsigned int z_mask = (z_wrap - 1) & 255;
    int px = stb__perlin_fastfloor(x);
    int py = stb__perlin_fastfloor(y);
    int pz = stb__perlin_fastfloor(z);
    int x0 = px & x_mask, x1 = (px + 1) & x_mask;
    int y0 = py & y_mask, y1 = (py + 1) & y_mask;
    int z0 = pz & z_mask, z1 = (pz + 1) & z_mask;
    int r0, r1, r00, r01, r10, r11;

#define stb__perlin_ease(a)   (((a*6-15)*a + 10) * a * a * a)

    x -= px; u = stb__perlin_ease(x);
    y -= py; v = stb__perlin_ease(y);
    z -= pz; w = stb__perlin_ease(z);

    r0 = stb__perlin_randtab[x0 + seed];
    r1 = stb__perlin_randtab[x1 + seed];

    r00 = stb__perlin_randtab[r0 + y0];
    r01 = stb__perlin_randtab[r0 + y1];
    r10 = stb__perlin_randtab[r1 + y0];
    r11 = stb__perlin_randtab[r1 + y1];

    n000 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r00 + z0], x, y, z);
    n001 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r00 + z1], x, y, z - 1);
    n010 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r01 + z0], x, y - 1, z);
    n011 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r01 + z1], x, y - 1, z - 1);
    n100 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r10 + z0], x - 1, y, z);
    n101 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r10 + z1], x - 1, y, z - 1);
    n110 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r11 + z0], x - 1, y - 1, z);
    n111 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r11 + z1], x - 1, y - 1, z - 1);

    n00 = stb__perlin_lerp(n000, n001, w);
    n01 = stb__perlin_lerp(n010, n011, w);
    n10 = stb__perlin_lerp(n100, n101, w);
    n11 = stb__perlin_lerp(n110, n111, w);

    n0 = stb__perlin_lerp(n00, n01, v);
    n1 = stb__perlin_lerp(n10, n11, v);

    return stb__perlin_lerp(n0, n1, u);
}

float stb_perlin_noise3(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap)
{
    return stb_perlin_noise3_internal(x, y, z, x_wrap, y_wrap, z_wrap, 0);
}

float stb_perlin_noise3_seed(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap, int seed)
{
    return stb_perlin_noise3_internal(x, y, z, x_wrap, y_wrap, z_wrap, (unsigned char)seed);
}

float stb_perlin_ridge_noise3(float x, float y, float z, float lacunarity, float gain, float offset, int octaves)
{
    int i;
    float frequency = 1.0f;
    float prev = 1.0f;
    float amplitude = 0.5f;
    float sum = 0.0f;

    for (i = 0; i < octaves; i++) {
        float r = stb_perlin_noise3_internal(x * frequency, y * frequency, z * frequency, 0, 0, 0, (unsigned char)i);
        r = offset - (float)fabs(r);
        r = r * r;
        sum += r * amplitude * prev;
        prev = r;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return sum;
}

float stb_perlin_fbm_noise3(float x, float y, float z, float lacunarity, float gain, int octaves)
{
    int i;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float sum = 0.0f;

    for (i = 0; i < octaves; i++) {
        sum += stb_perlin_noise3_internal(x * frequency, y * frequency, z * frequency, 0, 0, 0, (unsigned char)i) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return sum;
}

float stb_perlin_turbulence_noise3(float x, float y, float z, float lacunarity, float gain, int octaves)
{
    int i;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float sum = 0.0f;

    for (i = 0; i < octaves; i++) {
        float r = stb_perlin_noise3_internal(x * frequency, y * frequency, z * frequency, 0, 0, 0, (unsigned char)i) * amplitude;
        sum += (float)fabs(r);
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return sum;
}

float stb_perlin_noise3_wrap_nonpow2(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap, unsigned char seed)
{
    float u, v, w;
    float n000, n001, n010, n011, n100, n101, n110, n111;
    float n00, n01, n10, n11;
    float n0, n1;

    int px = stb__perlin_fastfloor(x);
    int py = stb__perlin_fastfloor(y);
    int pz = stb__perlin_fastfloor(z);
    int x_wrap2 = (x_wrap ? x_wrap : 256);
    int y_wrap2 = (y_wrap ? y_wrap : 256);
    int z_wrap2 = (z_wrap ? z_wrap : 256);
    int x0 = px % x_wrap2, x1;
    int y0 = py % y_wrap2, y1;
    int z0 = pz % z_wrap2, z1;
    int r0, r1, r00, r01, r10, r11;

    if (x0 < 0) x0 += x_wrap2;
    if (y0 < 0) y0 += y_wrap2;
    if (z0 < 0) z0 += z_wrap2;
    x1 = (x0 + 1) % x_wrap2;
    y1 = (y0 + 1) % y_wrap2;
    z1 = (z0 + 1) % z_wrap2;

#define stb__perlin_ease(a)   (((a*6-15)*a + 10) * a * a * a)

    x -= px; u = stb__perlin_ease(x);
    y -= py; v = stb__perlin_ease(y);
    z -= pz; w = stb__perlin_ease(z);

    r0 = stb__perlin_randtab[x0];
    r0 = stb__perlin_randtab[r0 + seed];
    r1 = stb__perlin_randtab[x1];
    r1 = stb__perlin_randtab[r1 + seed];

    r00 = stb__perlin_randtab[r0 + y0];
    r01 = stb__perlin_randtab[r0 + y1];
    r10 = stb__perlin_randtab[r1 + y0];
    r11 = stb__perlin_randtab[r1 + y1];

    n000 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r00 + z0], x, y, z);
    n001 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r00 + z1], x, y, z - 1);
    n010 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r01 + z0], x, y - 1, z);
    n011 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r01 + z1], x, y - 1, z - 1);
    n100 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r10 + z0], x - 1, y, z);
    n101 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r10 + z1], x - 1, y, z - 1);
    n110 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r11 + z0], x - 1, y - 1, z);
    n111 = stb__perlin_grad(stb__perlin_randtab_grad_idx[r11 + z1], x - 1, y - 1, z - 1);

    n00 = stb__perlin_lerp(n000, n001, w);
    n01 = stb__perlin_lerp(n010, n011, w);
    n10 = stb__perlin_lerp(n100, n101, w);
    n11 = stb__perlin_lerp(n110, n111, w);

    n0 = stb__perlin_lerp(n00, n01, v);
    n1 = stb__perlin_lerp(n10, n11, v);

    return stb__perlin_lerp(n0, n1, u);
}

