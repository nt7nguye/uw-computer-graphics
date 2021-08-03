// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "maze.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;
	
	// ADDED
	virtual void defaultSettings();

private:
	Maze maze;

	void initGrid();
	void initCube();
	void initFloor();
	void initAvatar();

	// TODO: Color and settings
	void saveColorToObject(int id, float* color);
	void displayColorOnSelect(int id, float* color);
	int last_selected;

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Unifofrm location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	float colour[3];
	float original_colour[3];
	int current_col;

	// ADDED
	// Zoom
	float zoom;

	// Rotate
	float rotating_rate;
	float current_rotation;
	double prev_xPos;

	// Cube 
	GLuint m_cube_vao;
	GLuint m_cube_vbo;
	GLuint m_cube_ebo;
	float cube_height;
	glm::vec3 cube_color;

	// Floor
	GLuint m_floor_vao;
	GLuint m_floor_vbo;
	glm::vec3 floor_color;

	// Avatar
	GLuint m_avatar_vao;
	GLuint m_avatar_vbo;
	glm::vec3 avatar_color;
	glm::vec3 avatar_pos;
	float avatar_radius;

};
