#ifndef CONTROLS_HPP
#define CONTROLS_HPP

vec3 get_position();
void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif