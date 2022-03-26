#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
//#include <SFML/Audio/SoundBuffer.hpp>
#include <gloom/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <gloom/timeutils.h>
#include <gloom/mesh.h>
#include <gloom/shapes.h>
#include <gloom/glutils.h>
//#include <gloom/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <fmt/format.h>
#include "logic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "gloom/imageLoader.hpp"
#include "gloom/glfont.h"
#include <gloom/gloom.hpp>




SceneNode* rootNode;
SceneNode* ballNode;


Gloom::Shader* shader;
Gloom::Shader* shaderText; // Added extra shader for drawing text output





unsigned int getTextureID(PNGImage *img)
{
    // Texture ID
    unsigned int textureID;

    // How many textures we want
    glGenTextures(1, &textureID);

    // Bound texture
    glBindTexture(GL_TEXTURE_2D, textureID);


    // Setting texture wrapping options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    // Generate textures
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels.data());

    // Genereate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);


    return textureID;

}




void initGame(GLFWwindow* window) {
    

  
    shader = new Gloom::Shader();
    shader->makeBasicShader("../shaders/simple.vert", "../shaders/simple.frag");
   
 
    shader->activate();



    // Create meshes
    Mesh sphere = generateSphere(100, 100, 40);

   
   

    // Load charmap texture PNG image
    PNGImage charmap = loadPNGFile("../textures/charmap.png");



    // Get charmap ID
    unsigned int charmapID = getTextureID(&charmap);


    // Fill buffers
    unsigned int ballVAO = generateBuffer(sphere);
   

    // Construct scene
    rootNode = createSceneNode();
    ballNode = createSceneNode();

    ballNode->nodeType = GEOMETRY;



    rootNode->children.push_back(ballNode);



    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount       = sphere.indices.size();
    ballNode->position = glm::vec3(10, 10, -50);


}

void updateFrame(GLFWwindow* window) {

    double timeDelta = getTimeDeltaSeconds();

    
    const float cameraWallOffset = 30; // Arbitrary addition to prevent ball from going too much into camera

    
    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);

    glm::vec3 cameraPosition = glm::vec3(0, 2, -10);

    // Some math to make the camera move in a nice way
    glm::mat4 cameraTransform = glm::translate(-cameraPosition);


    // To fragment shader camera postion
    glUniform3fv(13, 1, glm::value_ptr(cameraPosition));

    

    updateNodeTransformations(rootNode, cameraTransform, projection);


    // Ball location
    GLint ballLoc = glGetUniformLocation(shader->get(),"ballLoc");
    glUniform3fv(ballLoc, 1, glm::value_ptr(glm::vec3(ballNode->ModelMatrix * glm::vec4(0.0,0.0,0.0,1.0))));



}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar, glm::mat4 projection) {
  
    glm::mat4 transformationMatrix(1.0);
    
   
    switch(node->nodeType) {
        case GEOMETRY: 
        {

           transformationMatrix =
                glm::translate(node->position)
                * glm::translate(node->referencePoint)
                * glm::rotate(node->rotation.y, glm::vec3(0, 1, 0))
                * glm::rotate(node->rotation.x, glm::vec3(1, 0, 0))
                * glm::rotate(node->rotation.z, glm::vec3(0, 0, 1))
                * glm::scale(node->scale)
                * glm::translate(-node->referencePoint);





        }
            
            
            break;
        case POINT_LIGHT:
        {

            

        }
           
            break;
        case SPOT_LIGHT: break;
    }


    // MV Matrix
    node->ModelMatrix = transformationThusFar * transformationMatrix;

    // MVP Matrix
    node->currentTransformationMatrix = projection * node->ModelMatrix;


    // Normals (removing translations and scaling)
    auto normals = glm::mat3(glm::transpose(glm::inverse(node->ModelMatrix)));

    // Add normals to node
    node->normalMatrix = normals;

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->ModelMatrix, projection);
    }
}

void renderNode(SceneNode* node) 
{

   
    // MV Matrix to verte shader
    glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(node->ModelMatrix));

    // MVP matrix to vertex shader
    glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

    // Normal to vertex shader
    glUniformMatrix3fv(12, 1, GL_FALSE, glm::value_ptr(node->normalMatrix));



    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case POINT_LIGHT: 
        {

           
          

        }
            
            break;
        case SPOT_LIGHT: break;
        case NORMAL_MAPPED_GEOMETRY:
        {
            /*
            // Activate rendering for normal mapped geometry
            glUniform1ui(9, 1);
            
            // Bind vertex
            glBindVertexArray(node->vertexArrayObjectID);

            // Bind texture 
            glBindTextureUnit(0, node->textureID);

            // Bind normal map
            glBindTextureUnit(1, node->normalMapTextureID);

            // Bind roughness map (NEW)
            glBindTextureUnit(1, node->normalMapTextureID);

            // Draw elements
            glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

            // Deactivate
            glUniform1ui(9, 0);
            */

        }
            
            break;
    }


    

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}








void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    // Render 3d geometry
    renderNode(rootNode);
    

}
