// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/text2D.hpp>

// will leave it here for a moment
const float MAX_DISTANCE = 30, START_ENEMIES = 10, MAX_ENEMIES = 50;
std::vector<glm::vec3> default_enemy_vertices, default_fireball_vertices, default_enemy_normals, default_fireball_normals, 
	enemy_vertices, enemy_normals, fireball_vertices, fireball_normals, all_vertices, all_normals;
std::vector<glm::vec2> default_enemy_uvs, default_fireball_uvs, enemy_uvs, fireball_uvs, all_uvs;
int fireballs_shot = 0, enemies_hit = 0, n_enemies = 0;
const double cpi = 3.14159265359;
vec3 looking_at(1.0f, 0.0f, 1.0f), default_lookat(0.0f, 0.0f, -1.0f),
		default_fireball_center(0.0f, 0.0f, 0.0f), default_enemy_center(0.0f, 0.0f, 0.0f);
bool has_clicked;

float horizontal_angle(vec3 v1, vec3 v2) {
	v1[1] = 0;
	v2[1] = 0;
	float angle = acos(dot(normalize(v1), normalize(v2)));
	if (v1[0] * v2[2] - v1[2] * v2[0] < 0) {
		angle *= -1;
	}
	return angle;
}

vec3 rand_vec3(int min, int max) {
	double phi = 2 * cpi * rand() / RAND_MAX;
	double costheta = (double)rand() / RAND_MAX;
	double theta = acos(costheta);
	int r = (rand() % (max - min) + min);
	double x = r * sin(theta) * cos(phi) ;
	double y = r * sin(theta) * sin(phi);
	double z = r * cos(theta);
	return vec3(x, y, z);
}

vec3 rand_horizontal_vec3(int min, int max) {
	double phi = 2 * cpi * rand() / RAND_MAX;
	int r = (rand() % (max - min) + min);
	double x = r * sin(phi);
	double y = 0;
	double z = r * cos(phi);
	return vec3(x, y, z);
}

class fireball {
public:
	std::vector<glm::vec3> vertices, normals;
	std::vector<glm::vec2> uvs;
	vec3 center = default_fireball_center, direction;
	float collide_dist = 0.35f;

	fireball() {
		vertices = default_fireball_vertices;
		uvs = default_fireball_uvs;
		normals = default_fireball_normals;
		vec3 cur_position = get_position();

		glm::vec3 offset = normalize(looking_at) * 2.0f;
		offset[1] = -1.3f;
		offset[2] *= -1.0f;
		offset += cur_position;

		float angle = horizontal_angle(looking_at, default_lookat);
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, -angle, glm::vec3(0.0f, 1.0f, 0.0f));

		center += offset;
		for (int i = 0; i < vertices.size(); ++i) {
			vertices[i] = vec3(trans * vec4(vertices[i], 1));
			vertices[i] += offset;
		}

		direction = center - cur_position;
		direction[1] = 0;
		direction *= 0.1f;
	}

	void move() {
		center += direction;
		for (int i = 0; i < vertices.size(); ++i) {
			vertices[i] += direction;
		}
	}
};

class enemy {
public:
	std::vector<glm::vec3> vertices, normals;
	std::vector<glm::vec2> uvs;
	vec3 center = default_enemy_center;
	float collide_distance = 1.0f;

	enemy() {
		vertices = default_enemy_vertices;
		uvs = default_enemy_uvs;
		normals = default_enemy_normals;

		glm::vec3 offset = rand_horizontal_vec3(10, 25);
		offset[1] = -1.3f;

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, float(glm::radians((float)(rand() % 360))), glm::vec3(0.0f, 1.0f, 0.0f));

		center += offset;
		for (int i = 0; i < vertices.size(); ++i) {
			vertices[i] = vec3(trans * vec4(vertices[i], 1));
			vertices[i] += offset;
		}
	}
};

std::vector<fireball> balls;
std::vector<enemy> enemies;

void shoot_fireball() {
	++fireballs_shot;
	fireball new_ball;
	balls.push_back(new_ball);
}

void add_random_enemy() {
	enemy new_enemy;
	enemies.push_back(new_enemy);
	++n_enemies;
}

void delete_enemy(int k) {
	enemies.erase(enemies.begin() + k);
	--n_enemies;
}

void delete_fireball(int k) {
	balls.erase(balls.begin() + k);
}

void check_collisions() {
	for (int f = 0; f < balls.size(); ++f) {
		for (int e = 0; e < enemies.size(); ++e) {
			if (distance(balls[f].center, enemies[e].center) < 
					balls[f].collide_dist + enemies[e].collide_distance) {
				++enemies_hit;
				delete_enemy(e);
				delete_fireball(f);
				//add_random_enemy();
				--f;
				break;
			}
		}
	}
}

void move_fireballs() {
	// first delete ones too far away
	for (int k = 0; k < balls.size(); ++k) {
		if (length(balls[k].center) > MAX_DISTANCE) {
			delete_fireball(k);
			--k;
		}
	}

	for (int k = 0; k < balls.size(); ++k) {
		balls[k].move();
	}
	check_collisions();
}

void rotate_default_fireball() {
	float angle = acos(dot(looking_at, default_lookat));
	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	for (int i = 0; i < default_fireball_vertices.size(); ++i) {
		default_fireball_vertices[i] = vec3(trans * vec4(default_fireball_vertices[i], 1));
	}
}

void concatenate_enemies() {
	enemy_vertices.clear();
	enemy_uvs.clear();
	enemy_normals.clear();
	for (int k = 0; k < enemies.size(); ++k) {
		enemy_vertices.insert(enemy_vertices.end(), enemies[k].vertices.begin(), enemies[k].vertices.end());
		enemy_uvs.insert(enemy_uvs.end(), enemies[k].uvs.begin(), enemies[k].uvs.end());
		enemy_normals.insert(enemy_normals.end(), enemies[k].normals.begin(), enemies[k].normals.end());
	}
}

void concatenate_fireballs() {
	fireball_vertices.clear();
	fireball_uvs.clear();
	fireball_normals.clear();
	for (int k = 0; k < balls.size(); ++k) {
		fireball_vertices.insert(fireball_vertices.end(), balls[k].vertices.begin(), balls[k].vertices.end());
		fireball_uvs.insert(fireball_uvs.end(), balls[k].uvs.begin(), balls[k].uvs.end());
		fireball_normals.insert(fireball_normals.end(), balls[k].normals.begin(), balls[k].normals.end());
	}
}

void concatenate_all() {
	all_vertices.clear();
	all_uvs.clear();
	all_normals.clear();
	for (int k = 0; k < enemies.size(); ++k) {
		all_vertices.insert(all_vertices.end(), enemies[k].vertices.begin(), enemies[k].vertices.end());
		all_uvs.insert(all_uvs.end(), enemies[k].uvs.begin(), enemies[k].uvs.end());
		all_normals.insert(all_normals.end(), enemies[k].normals.begin(), enemies[k].normals.end());
	}
	for (int k = 0; k < balls.size(); ++k) {
		all_vertices.insert(all_vertices.end(), balls[k].vertices.begin(), balls[k].vertices.end());
		all_uvs.insert(all_uvs.end(), balls[k].uvs.begin(), balls[k].uvs.end());
		all_normals.insert(all_normals.end(), balls[k].normals.begin(), balls[k].normals.end());
	}
}

/*void on_click(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		has_clicked = true;
	}
}*/

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		shoot_fireball();
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 07 - Model Loading", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
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
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	//GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Load the texture
	GLuint fire_texture = loadDDS("fire.DDS");
	GLuint water_texture = loadDDS("water.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	bool res1 = loadOBJ("enemy_1.obj", default_enemy_vertices, default_enemy_uvs, default_enemy_normals);
	bool res2 = loadOBJ("firewhat_2.obj", default_fireball_vertices, default_fireball_uvs, default_fireball_normals);
	rotate_default_fireball();

	for (int i = 0; i < START_ENEMIES; ++i) {
		add_random_enemy();
	}
	concatenate_all();

	/*shoot_fireball_at(vec3(1.0, 0.0, -1.0));
	shoot_fireball_at(vec3(1.0, 0.0, 1.0));
	shoot_fireball_at(vec3(-1.0, 0.0, -1.0));
	shoot_fireball_at(vec3(-1.0, 0.0, 1.0));*/

	// Load it into a VBO
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, all_vertices.size() * sizeof(glm::vec3), &all_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, all_uvs.size() * sizeof(glm::vec2), &all_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, all_normals.size() * sizeof(glm::vec3), &all_normals[0], GL_STATIC_DRAW);

	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// Initialize our little text library with the Holstein font
	initText2D("Holstein.DDS");

	// For speed computation
	double fireball_last_time = glfwGetTime(), enemy_last_time = glfwGetTime();
	int frame_counter = 0;
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// get the 3rd column -- the forward direction
		// used for directing fireballs
		looking_at = vec3(ViewMatrix[2]);
		
		// wanted to do on_click fireballs
		// but linking the library made me hate the entire humans race
		/*glutMouseFunc(on_click);
		if (has_clicked) {
			shoot_fireball();
		}*/

		// shoot fireball and spawn enemy each second
		double cur_time = glfwGetTime();
		if (cur_time - enemy_last_time > 1) {
			enemy_last_time = cur_time;
			if (n_enemies < MAX_ENEMIES) {
				add_random_enemy();
			}
		}
		if (cur_time - fireball_last_time > 0.2f) {
			fireball_last_time = cur_time;
			//shoot_fireball();
			int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
			if (state == GLFW_PRESS)
			{
				shoot_fireball();
			}
		}
		// move_fireballs
		move_fireballs();

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(0, 10, 0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, water_texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		concatenate_enemies();
		concatenate_fireballs();
		concatenate_all();
		// Load it into a VBO
		GLuint vertexbuffer;
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, all_vertices.size() * sizeof(glm::vec3), &all_vertices[0], GL_STATIC_DRAW);

		GLuint uvbuffer;
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, all_uvs.size() * sizeof(glm::vec2), &all_uvs[0], GL_STATIC_DRAW);

		GLuint normalbuffer;
		glGenBuffers(1, &normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, all_normals.size() * sizeof(glm::vec3), &all_normals[0], GL_STATIC_DRAW);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, all_vertices.size() );

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		char text[256];
		sprintf(text, "%.2f sec", glfwGetTime());
		std::string fireballs(std::to_string( enemies_hit));
		printText2D(fireballs.data(), 10, 500, 100);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &fire_texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Delete the text's VBO, the shader and the texture
	cleanupText2D();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}