#pragma once

#include <gloom/gloom.hpp>
#include "sceneGraph.hpp"

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar, glm::mat4 projection);
void initGame(GLFWwindow* window);
void updateFrame(GLFWwindow* window);
void renderFrame(GLFWwindow* window);