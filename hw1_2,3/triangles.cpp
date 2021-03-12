// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
using namespace glm;
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <common/shader.hpp>

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Triangles", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID_Center = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShaderCenter.fragmentshader");
	GLuint programID_Left = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShaderLeft.fragmentshader");
	GLuint programID_Right = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShaderRight.fragmentshader");

	// alpha channels
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	static const GLfloat g_vertex_buffer_data[] = {
		-0.8f, -0.8f, 0.0f,
		0.8f, -0.8f, 0.0f,
		0.0f, 0.8f, 0.0f,
		-0.1f, -0.7f, 0.0f,
		0.9f, -0.7f, 0.0f,
		0.2f, 0.5f, 0.0f,
		0.1f, -0.7f, 0.0f,
		-0.9f, -0.7f, 0.0f,
		-0.2f, 0.5f, 0.0f,
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// MVP stuff
	GLuint MatrixID_Center = glGetUniformLocation(programID_Center, "MVP");
	GLuint MatrixID_Left = glGetUniformLocation(programID_Left, "MVP");
	GLuint MatrixID_Right = glGetUniformLocation(programID_Right, "MVP");
	
	glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	GLfloat r = 3.0f;
	GLfloat angle = 0.0f;
	GLfloat shift = 0.01f;

	do {
		// camera movement
		glm::mat4 ViewMatrix = glm::lookAt(
			glm::vec3(r * std::cos(angle), 0.0f, r * std::sin(angle)),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0)
		);
		angle += shift;
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glUseProgram(programID_Center);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glUniformMatrix4fv(MatrixID_Center, 1, GL_FALSE, &MVP[0][0]);
		glUseProgram(programID_Right);
		glDrawArrays(GL_TRIANGLES, 3, 3); 
		glUniformMatrix4fv(MatrixID_Right, 1, GL_FALSE, &MVP[0][0]);
		glUseProgram(programID_Left);
		glDrawArrays(GL_TRIANGLES, 6, 3);
		glUniformMatrix4fv(MatrixID_Left, 1, GL_FALSE, &MVP[0][0]);

		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID_Center);
	glDeleteProgram(programID_Left);
	glDeleteProgram(programID_Right);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

