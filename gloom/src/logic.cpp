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
#include <gloom/Camera.hpp>






SceneNode* rootNode;
SceneNode* cubeNode;
Gloom::Camera* cam = new Gloom::Camera();


Gloom::Shader* shader;
Gloom::Shader* shaderText; // Added extra shader for drawing text output



void initScene(GLFWwindow* window) 
{
    

    shader = new Gloom::Shader();
    shader->makeBasicShader("../gloom/shaders/simple.vert", "../gloom/shaders/simple.frag");
    shader->activate();


    // Create mesh (Full screen: 600 x 400)
    Mesh square = cube(glm::vec3(350,300,0), glm::vec2(1,1));

    // Fill buffers
    unsigned int cubeVAO = generateBuffer(square);

    // Construct scene
    rootNode = createSceneNode();
    cubeNode = createSceneNode();

    rootNode->children.push_back(cubeNode);
    cubeNode->nodeType = GEOMETRY;
    cubeNode->vertexArrayObjectID = cubeVAO;
    cubeNode->VAOIndexCount = square.indices.size();
    cubeNode->position = glm::vec3(0,0, -400);


}

void updateFrame(GLFWwindow* window) {

    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), float(windowWidth) / float(windowHeight), 0.1f, 750.f);

    glm::vec3 cameraPosition = glm::vec3(-1.0, -1.0, -10.0);

    // Some math to make the camera move in a nice way
    glm::mat4 cameraTransform = glm::translate(-cameraPosition);


    updateNodeTransformations(rootNode, cameraTransform, projection);


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




    double last_time = 0;
    double current_time = 0;
    float deltatime = 0;


    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                
                // Time for animation
                current_time = glfwGetTime();
                deltatime = current_time - last_time;
                last_time = current_time;

                GLint iTime = glGetUniformLocation(shader->get(), "u_time");
                glUniform1f(iTime, last_time);
                
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

    renderNode(rootNode);

  

}
