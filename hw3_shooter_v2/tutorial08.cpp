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
const float MAX_DISTANCE = 40, START_ENEMIES = 20, MAX_ENEMIES = 50;
std::vector<glm::vec3> default_enemy_vertices, default_fireball_vertices, default_enemy_normals, default_fireball_normals, 
	enemy_vertices, enemy_normals, fireball_vertices, fireball_normals, all_vertices, all_normals, spheres_vertices, 
	spheres_normals, floor_vertices, floor_normals, sky_vertices, sky_normals;
std::vector<glm::vec2> default_enemy_uvs, default_fireball_uvs, enemy_uvs, fireball_uvs, all_uvs, spheres_uvs, sky_uvs, floor_uvs;
int fireballs_shot = 0, enemies_hit = 0, n_enemies = 0, cur_det = 1, spheres_shot = 0, oct_size=16;
const double cpi = 3.14159265359;
vec3 looking_at(1.0f, 0.0f, 1.0f), default_lookat(0.0f, 0.0f, -1.0f),
		default_fireball_center(0.0f, 0.0f, 0.0f), default_enemy_center(0.0f, 0.0f, 0.0f);
float time_coefficient = 1.0;

std::vector<std::vector<vec3>> triangle_detalisation_up(const std::vector<vec3>& tr) {
	return {
		{ (tr[2] + tr[0]) * 0.5f, tr[0], (tr[1] + tr[0]) * 0.5f },
		{ tr[1], (tr[2] + tr[1]) * 0.5f, (tr[0] + tr[1]) * 0.5f },
		{ tr[2], (tr[0] + tr[2]) * 0.5f, (tr[1] + tr[2]) * 0.5f },
		{ (tr[0] + tr[2]) * 0.5f, (tr[0] + tr[1]) * 0.5f, (tr[1] + tr[2]) * 0.5f },
	};
}
std::vector<vec3> triangle_detalisation_down(const std::vector<std::vector<vec3>>& trs) {
	return 	{ trs[0][1], trs[1][0], trs[2][0] };
}
std::vector<std::vector<vec3>> get_octahedron_triangles() {
	std::vector<std::vector<vec3>> octahedron_triangles = {
		// top part
		{ { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		// bottom part
		{ { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		// изнанка
		// top part
		{ { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, 0.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		// bottom part
		{ { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { 0.0f, 0.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f } },
	};
	return octahedron_triangles;
}
void up_octahedron_detalisation(std::vector<std::vector<vec3>>& octahedron_triangles) {
	std::vector<std::vector<vec3>> new_octahedron_triangles;
	for (auto tr : octahedron_triangles) {
		std::vector<std::vector<vec3>> upped = triangle_detalisation_up(tr);
		new_octahedron_triangles.insert(new_octahedron_triangles.end(), upped.begin(), upped.end());
	}
	octahedron_triangles = new_octahedron_triangles;
}
void down_octahedron_detalisation(std::vector<std::vector<vec3>>& octahedron_triangles) {
	std::vector<std::vector<vec3>> new_octahedron_triangles;
	for (int i = 0; i < octahedron_triangles.size(); i += 4) {
		std::vector<std::vector<vec3>> trs(octahedron_triangles.begin() + i, octahedron_triangles.begin() + i + 4);
		std::vector<vec3> downed = triangle_detalisation_down(trs);
		new_octahedron_triangles.push_back(downed);
	}
	octahedron_triangles = new_octahedron_triangles;
}
std::vector<vec3> octahedron_to_sphere(const std::vector<std::vector<vec3>>& octahedron_triangles, float radius, vec3 center) {
	std::vector<vec3> sphere;
	for (auto& tr : octahedron_triangles) {
		for (auto& p : tr) {
			sphere.push_back(normalize(p) * radius + center);
		}
	}
	return sphere;
}

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

class sphere {
public:
	std::vector < std::vector<glm::vec3>> octahedron_triangles;
	std::vector<glm::vec3> vertices;
	vec3 center = default_fireball_center, direction;
	float collide_dist = 0.35f, radius = 0.2f;

	sphere() {
		octahedron_triangles = get_octahedron_triangles();
		for (int i = 1; i < cur_det; ++i) {
			up_octahedron_detalisation(octahedron_triangles);
		}
		vec3 cur_position = get_position();

		glm::vec3 offset = normalize(looking_at) * 2.0f;
		offset[2] *= -1.0f;
		offset[1] = -1.3f;
		offset += cur_position;

		float angle = horizontal_angle(looking_at, default_lookat);
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, -angle, glm::vec3(0.0f, 1.0f, 0.0f));

		center += offset;
		vertices = octahedron_to_sphere(octahedron_triangles, radius, center);

		direction = center - cur_position;
		direction[1] = 0;
		direction *= 0.1f;
	}

	void move() {
		vec3 diff = direction * time_coefficient;
		center += diff;
		for (int i = 0; i < vertices.size(); ++i) {
			vertices[i] += diff;
		}
	}

	void renew_vertices() {
		vertices = octahedron_to_sphere(octahedron_triangles, radius, center);
	}
};
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
		vec3 diff = direction * time_coefficient;
		center += diff;
		for (int i = 0; i < vertices.size(); ++i) {
			vertices[i] += diff;
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
std::vector<sphere> spheres;
std::vector<enemy> enemies;

void shoot_fireball() {
	++fireballs_shot;
	fireball new_ball;
	balls.push_back(new_ball);
}
void shoot_sphere() {
	++spheres_shot;
	sphere new_sphere;
	spheres.push_back(new_sphere);
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
void delete_sphere(int k) {
	spheres.erase(spheres.begin() + k);
}

void check_collisions() {
	for (int f = 0; f < spheres.size(); ++f) {
		for (int e = 0; e < enemies.size(); ++e) {
			if (distance(spheres[f].center, enemies[e].center) <
				spheres[f].collide_dist + enemies[e].collide_distance) {
				++enemies_hit;
				delete_enemy(e);
				delete_sphere(f);
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
void move_spheres() {
	// first delete ones too far away
	for (int k = 0; k < spheres.size(); ++k) {
		if (length(spheres[k].center) > MAX_DISTANCE) {
			delete_sphere(k);
			--k;
		}
	}

	for (int k = 0; k < spheres.size(); ++k) {
		spheres[k].move();
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

void sphere_detalisation_up() {
	if (cur_det >= 5) {
		return;
	}
	++cur_det;
	for (auto& sphere : spheres) {
		up_octahedron_detalisation(sphere.octahedron_triangles);
		sphere.renew_vertices();
	}
}
void sphere_detalisation_down() {
	if (cur_det <= 1) {
		return;
	}
	--cur_det;
	for (auto& sphere : spheres) {
		down_octahedron_detalisation(sphere.octahedron_triangles);
		sphere.renew_vertices();
	}
}

void create_floor() {
	floor_vertices.clear();
	floor_uvs.clear();
	floor_normals.clear();
	std::vector<vec3> default_tile_vertices = {
		// 1
		{ 1.0f, -2.0f, 1.0f },
		{ 1.0f, -2.0f, -1.0f },
		{ -1.0f, -2.0f, 1.0f },
		// 2
		{ 1.0f, -2.0f, -1.0f },
		{ -1.0f, -2.0f, -1.0f },
		{ -1.0f, -2.0f, 1.0f },
	};
	std::vector<vec3> tile_vertices(6);
	std::vector<vec3> default_tile_normals = {
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		// 2
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
	};	
	/*std::vector<vec2> default_tile_uvs = {
		{ 1.0f, 1.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 1.0f },
		// 2
		{ 1.0f, 1.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 1.0f },
	}; */
	std::vector<glm::vec2> default_tile_uvs(default_enemy_uvs.begin(), default_enemy_uvs.begin() + 6);
	
	for (int i = -25; i <= 25; ++i) {
		for (int j = -25; j <= 25; ++j) {
			for (int k = 0; k < 6; ++k) {
				tile_vertices[k] = default_tile_vertices[k];
				tile_vertices[k][0] += 2*i;
				tile_vertices[k][2] += 2*j;
			}
			floor_vertices.insert(floor_vertices.end(), tile_vertices.begin(), tile_vertices.end());
			floor_uvs.insert(floor_uvs.end(), default_tile_uvs.begin(), default_tile_uvs.end());
			floor_normals.insert(floor_normals.end(), default_tile_normals.begin(), default_tile_normals.end());
		}
	}
}
void create_sky() {
	sky_vertices.clear();
	sky_uvs.clear();
	sky_normals.clear();

	cur_det = 5;
	sphere sky;
	cur_det = 1;
	sky_vertices = octahedron_to_sphere(sky.octahedron_triangles, 50, {0.0f, 0.0f, 0.0f});

	std::vector<vec2> default_sky_uvs(default_enemy_uvs.begin(), default_enemy_uvs.begin() + 16);
	std::vector<vec3> default_sky_normals, default_normal = {
		{ 0.0f, -1.0f, 0.0f },
	};
	for (int i = 0; i < 16; ++i) {
		default_sky_normals.insert(default_sky_normals.end(), default_normal.begin(), default_normal.end());
	}
	for (int i = 0; i < sky_vertices.size() / 16; ++i) {
		sky_uvs.insert(sky_uvs.end(), default_sky_uvs.begin(), default_sky_uvs.end());
		sky_normals.insert(sky_normals.end(), default_sky_normals.begin(), default_sky_normals.end());
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
void concatenate_spheres() {
	spheres_vertices.clear();
	spheres_uvs.clear();
	spheres_normals.clear();
	if (spheres.size() > 0) {
		std::vector<glm::vec2> dddefault_sphere_uvs(default_enemy_uvs.begin(), default_enemy_uvs.begin() + oct_size),
			default_sphere_uvs;
		for (int i = 0; i < spheres[0].vertices.size() / oct_size; ++i) {
			default_sphere_uvs.insert(default_sphere_uvs.end(), dddefault_sphere_uvs.begin(), dddefault_sphere_uvs.end());
		}
		std::vector<glm::vec3> default_sphere_normals(spheres[0].vertices.size(), { 0.0f, 1.0f, 0.0f });

		for (int k = 0; k < spheres.size(); ++k) {
			spheres_vertices.insert(spheres_vertices.end(), spheres[k].vertices.begin(), spheres[k].vertices.end());
			spheres_uvs.insert(spheres_uvs.end(), default_sphere_uvs.begin(), default_sphere_uvs.end());
			spheres_normals.insert(spheres_normals.end(), default_sphere_normals.begin(), default_sphere_normals.end());
		}
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
	/*for (int k = 0; k < balls.size(); ++k) {
		all_vertices.insert(all_vertices.end(), balls[k].vertices.begin(), balls[k].vertices.end());
		all_uvs.insert(all_uvs.end(), balls[k].uvs.begin(), balls[k].uvs.end());
		all_normals.insert(all_normals.end(), balls[k].normals.begin(), balls[k].normals.end());
	}*/
	if (spheres.size() > 0) {
		std::vector<glm::vec2> default_sphere_uvs(spheres[0].vertices.size(), { 0.0f, 0.0f });
		std::vector<glm::vec3> default_sphere_normals(spheres[0].vertices.size(), { 0.0f, 0.0f, 0.0f });

		for (int k = 0; k < spheres.size(); ++k) {
			all_vertices.insert(all_vertices.end(), spheres[k].vertices.begin(), spheres[k].vertices.end());
			all_uvs.insert(all_uvs.end(), default_sphere_uvs.begin(), default_sphere_uvs.end());
			all_normals.insert(all_normals.end(), default_sphere_normals.begin(), default_sphere_normals.end());
		}
	}
	all_vertices.insert(all_vertices.end(), floor_vertices.begin(), floor_vertices.end());
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		//shoot_fireball();
		shoot_sphere();
	}
}
void button_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		sphere_detalisation_down();
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		sphere_detalisation_up();
	}
	if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
		time_coefficient = std::max(0.125f, time_coefficient / 2);
	}
	if (key == GLFW_KEY_9 && action == GLFW_PRESS) {
		time_coefficient = std::min(4.0f, time_coefficient * 2);
	}
}

void populate_buffers(const std::vector<vec3>& vertices,
		const std::vector<vec2>& uvs,
		const std::vector<vec3>& normals) {

	if (vertices.size() > 0) {
		GLuint vertexbuffer;
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		GLuint uvbuffer;
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

		GLuint normalbuffer;
		glGenBuffers(1, &normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

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
	}
}

std::string det_id() {
	switch (cur_det) {
	case 1:
		return "Low";
		break;
	case 2:
		return "Medium";
		break;
	case 3:
		return "High";
		break;
	case 4:
		return "Ultra";
		break;
	case 5:
		return "Laggy";
		break;
	}
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
	GLuint programID = LoadShaders("shaders/StandardShading.vertexshader", "shaders/StandardShading.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Load the texture
	GLuint fire_texture = loadDDS("assets/fire.DDS");
	GLuint water_texture = loadDDS("assets/water.DDS");
	GLuint grass_texture = loadDDS("assets/grass.DDS");
	GLuint sky_texture = loadDDS("assets/sky.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	bool res1 = loadOBJ("assets/enemy_1.obj", default_enemy_vertices, default_enemy_uvs, default_enemy_normals);
	bool res2 = loadOBJ("assets/firewhat_2.obj", default_fireball_vertices, default_fireball_uvs, default_fireball_normals);
	rotate_default_fireball();

	for (int i = 0; i < START_ENEMIES; ++i) {
		add_random_enemy();
	}
	concatenate_all();
	//std::string to_print("Vertices  " + std::to_string(default_enemy_vertices.size()) + " Normals "  + std::to_string(default_enemy_normals.size()));
	//fprintf(stderr, to_print.data());
	create_floor();
	create_sky();

	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// Initialize our little text library with the Holstein font
	initText2D("assets/Holstein.DDS");

	// For speed computation
	double fireball_last_time = glfwGetTime(), enemy_last_time = glfwGetTime();
	int frame_counter = 0;
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, button_callback);

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
			int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
			if (state == GLFW_PRESS)
			{
				shoot_sphere();
			}
		}
		// move_fireballs
		move_spheres();

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(0, 35, 0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, water_texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		concatenate_enemies();
		concatenate_fireballs();
		concatenate_spheres();
		concatenate_all();
		
		glBindTexture(GL_TEXTURE_2D, water_texture);
		populate_buffers(enemy_vertices, enemy_uvs, enemy_normals);
		glDrawArrays(GL_TRIANGLES, 0, enemy_vertices.size());

		glBindTexture(GL_TEXTURE_2D, fire_texture);
		populate_buffers(spheres_vertices, spheres_uvs, spheres_normals);
		glDrawArrays(GL_TRIANGLES, 0, spheres_vertices.size());

		glBindTexture(GL_TEXTURE_2D, grass_texture);
		populate_buffers(floor_vertices, floor_uvs, floor_normals);
		glDrawArrays(GL_TRIANGLES, 0, floor_vertices.size());

		glBindTexture(GL_TEXTURE_2D, sky_texture);
		populate_buffers(sky_vertices, sky_uvs, sky_normals);
		glDrawArrays(GL_TRIANGLES, 0, sky_vertices.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		char text[256];
		sprintf(text, "%.2f sec", glfwGetTime());
		std::string s_det("Detalisation: " + det_id()), 
			s_time("Time " + std::to_string(time_coefficient)),
			s_enemies(std::to_string(enemies_hit));
		s_time = s_time.substr(0, 10) + "x";
		printText2D(s_time.data(), 10, 560, 20);
		printText2D(s_det.data(), 10, 530, 20);
		printText2D(s_enemies.data(), 10, 500, 20);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteProgram(programID);
	glDeleteTextures(1, &fire_texture);
	glDeleteVertexArrays(1, &VertexArrayID);
	// Delete the text's VBO, the shader and the texture
	cleanupText2D();
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}