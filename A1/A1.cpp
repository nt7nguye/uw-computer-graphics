// Termm--Fall 2020

#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <set>

#include <sys/types.h>
#include <unistd.h>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;
static const set<int> BOUNDARIES {-1, 16};

static const float PI = 3.14159;

static const float SCALE_RATE = 0.1f;
static const float SCALE_MAX_RANGE = 3.0f;
static const float SCALE_MIN_RANGE = 0.3f;

enum ColorEditorId { BLOCK =  0, FLOOR = 1, AVATAR = 2};


//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 ), maze(DIM)
{
	colour[0] = 0.0f;
	colour[1] = 0.0f;
	colour[2] = 0.0f;
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Initialize random number generator
	int rseed=getpid();
	srandom(rseed);
	// Print random number seed in case we want to rerun with
	// same random numbers
	cout << "Random number seed = " << rseed << endl;
	
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	// Set up models
	defaultSettings();
	initGrid();
	initCube();
	initFloor();
	initAvatar();
	
	// avatar track
	
	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, 2.*float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ));

	proj = glm::perspective( 
		glm::radians( 30.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts; // TODO: Why does this delete doesn't give error ?

	CHECK_GL_ERRORS;
}

void A1::initCube() {
	// NOTE: mirror initGrid() code struct, (check note drawing)
	float verts[] = {
		0.0f, 0.0f, 1.0f,
		0.0f, cube_height, 1.0f,
		1.0f, cube_height, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, cube_height, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, cube_height, 0.0f,
		0.0f, 0.0f, 0.0f
	};

	unsigned int indices[] = {
		0, 1, 2,
		0, 2, 3,
		2, 3, 4,
		3, 4, 5,
		4, 5, 6,
		5, 6, 7,
		0, 1, 6,
		0, 6, 7,
		1, 2, 6,
		2, 4 ,6,
		0, 3, 7,
		3, 5, 7
	};

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_cube_vao );
	glBindVertexArray( m_cube_vao );

	glGenBuffers( 1, &m_cube_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(verts),
		verts, GL_STATIC_DRAW );

	glGenBuffers( 1, &m_cube_ebo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_cube_ebo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
		indices, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	// delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::initFloor() {
	// copied from cube logic
	float verts[] = {
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,
	};

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_floor_vao );
	glBindVertexArray( m_floor_vao );

	glGenBuffers( 1, &m_floor_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_floor_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(verts)*sizeof(float),
		verts, GL_STATIC_DRAW );
	
	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

void A1::initAvatar() {
	// using Song Ho parametric equations and naming conventions
	// Sector angle theta and stack angle phi.
	size_t sz = 180 * 360 * 9 * 2;
	float* verts = new float[sz];
	float r = avatar_radius;
	float PI_offset = PI / 180.0f;
	int i = 0;

	for (int sector=0;sector<360;sector++) {
		for (int stack=-90;stack<90;stack++) {
			float theta = PI * sector / 180.0f;
			float phi = PI * stack / 180.0f;
			
			verts[i] = r * cos(phi - PI_offset) * cos(theta);
			verts[i + 1] = r * sin(phi - PI_offset);
			verts[i + 2] = r * cos(phi - PI_offset) * sin(theta);

			verts[i + 3] = r * cos(phi) * cos(theta);
			verts[i + 4] = r * sin(phi);
			verts[i + 5] = r * cos(phi) * sin(theta);

			verts[i + 6] = r * cos(phi) * cos(theta + PI_offset);
			verts[i + 7] = r * sin(phi);
			verts[i + 8] = r * cos(phi) * sin(theta + PI_offset);

			i += 9;

			verts[i] = r * cos(phi - PI_offset) * cos(theta);
			verts[i + 1] = r * sin(phi - PI_offset);
			verts[i + 2] = r * cos(phi - PI_offset) * sin(theta);

			verts[i + 3] = r * cos(phi - PI_offset) * cos(theta + PI_offset);
			verts[i + 4] = r * sin(phi - PI_offset);
			verts[i + 5] = r * cos(phi - PI_offset) * sin(theta + PI_offset);

			verts[i + 6] = r * cos(phi) * cos(theta + PI_offset);
			verts[i + 7] = r * sin(phi);
			verts[i + 8] = r * cos(phi) * sin(theta + PI_offset);

			i += 9;
		}
	}

	glGenVertexArrays( 1, &m_avatar_vao );
	glBindVertexArray( m_avatar_vao );

	// Create the avatar vertex buffer
	glGenBuffers( 1, &m_avatar_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_avatar_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Reset helper
 */
void A1::defaultSettings() {
	// Scale
	zoom = 1.0f;

	// Rotate
	rotating_rate = 0.0f;
	current_rotation = 0.0f;
	prev_xPos = 0;

	// TRON colors
	// Cube
	cube_height = 1.0f;
	cube_color = vec3(0.3f, 0.8f, 0.8f);

	// Floor
	floor_color = vec3(0.2f, 0.2f, 0.2f);

	// Avatar
	avatar_color = vec3(1.0f, 0.5f, 0.0f);
	avatar_pos = vec3(0.0f, 0.0f, 0.0f);
	avatar_radius = 0.5f;


}

//----------------------------------------------------------------------------------------
/*
 * Color helper. Store original colour 
 */
void A1::displayColorOnSelect(int id, float* color) {
	if (id == ColorEditorId::BLOCK) {
		color[0] = cube_color.r;
		color[1] = cube_color.g;
		color[2] = cube_color.b;
	} else if (id == ColorEditorId::FLOOR) {
		color[0] = floor_color.r;
		color[1] = floor_color.g;
		color[2] = floor_color.b;
	} else if (id == ColorEditorId::AVATAR) {
		color[0] = avatar_color.r;
		color[1] = avatar_color.g;
		color[2] = avatar_color.b;
	}
}

void A1::saveColorToObject(int id, float* color) {
	vec3 tmp_color(color[0], color[1], color[2]);
	if (id == ColorEditorId::BLOCK) {
		cube_color = tmp_color;
	} else if (id == ColorEditorId::FLOOR) {
		floor_color = tmp_color;
	} else if (id == ColorEditorId::AVATAR)  {
		avatar_color = tmp_color;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()

{
	// Place per frame, application logic here ...
	// Use this instead of timer
	current_rotation += rotating_rate;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if ( ImGui::Button( "Reset" ) ) {
			maze.reset();
			defaultSettings();
		}
		if ( ImGui::Button( "Dig Maze ") ) {
			maze.digMaze();
			maze.printMaze();
			for (int i=0; i<DIM; i++) {
				for (int j=0; j<DIM;j++) {
					if ((i == 0 || i == DIM - 1 || j == 0 || j == DIM - 1) && maze.getValue(i,j) == 0) {
						avatar_pos = vec3(j,0,i);
						// cout << "DEBUG" <<  i << j << endl;
					}
				}
			}
		}

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.
		ImGui::PushID("widget");
		static int selected_id = 0;
        ImGui::RadioButton("Cube", &selected_id, 0); ImGui::SameLine();
        ImGui::RadioButton("Floor", &selected_id, 1); ImGui::SameLine();
        ImGui::RadioButton("Avatar", &selected_id, 2); 
		ImGui::PopID();

		if (last_selected != selected_id) {
			saveColorToObject(last_selected, original_colour);
			
			displayColorOnSelect(selected_id, colour);
			displayColorOnSelect(selected_id, original_colour);
			last_selected = selected_id;
		}
		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		ImGui::PushID( 0 );
		ImGui::ColorEdit3( "##Colour", colour );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 0 ) ) {
			// Select this colour.
			displayColorOnSelect(selected_id, original_colour);
		}
		ImGui::PopID();

		saveColorToObject(selected_id, colour);

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;

	W = glm::scale(W, vec3(zoom));
	W = glm::rotate( W, 2 * PI * current_rotation, vec3( 0, 1, 0) );
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );
	

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

		// Draw the cubes
		mat4 base = W;

		for (int i=0;i<DIM+2;i++) {
			for (int j=0;j<DIM+2;j++) {
				W = glm::translate( W, vec3(j-1, 0, i-1));
				glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

				if (BOUNDARIES.count(i-1) || BOUNDARIES.count(j-1) || maze.getValue(i-1,j-1) == 0) {
					glBindVertexArray( m_floor_vao );
					glUniform3f( col_uni, floor_color.r, floor_color.g, floor_color.b );
					glDrawArrays( GL_TRIANGLES, 0, 6 );

					// cout << "DEBUG " << i << j << "floors" << endl;;
				} else {
					glBindVertexArray( m_cube_vao );
					glUniform3f( col_uni, cube_color.r, cube_color.g, cube_color.b );
					glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
					// cout << "DEBUG " << i << j << "cube" << endl;;
				}
				// reset W
				W = base;
			}
		}
		
		glBindVertexArray( m_avatar_vao );
		glUniform3f( col_uni, avatar_color.r, avatar_color.g, avatar_color.b );
		W = glm::translate( W, avatar_pos + vec3(avatar_radius));
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
		glDrawArrays( GL_TRIANGLES, 0, 360*180*6);
		W = base;

		// TODO: Highlight the active square ?

	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
		if (ImGui::IsMouseDragging(0)) {
			rotating_rate = (xPos - prev_xPos) / 1000;
			current_rotation += rotating_rate;
		}
		prev_xPos = xPos;

		eventHandled = true;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
		if (button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_RELEASE) {
			rotating_rate = 0.001f;
		} else if (button != GLFW_MOUSE_BUTTON_LEFT) {
			// stop current rotatikon
			rotating_rate = 0.0f;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.
	if (yOffSet > 0) {
		if (zoom < 3.0f) zoom += 0.1f;
	} else {
		if (zoom > 0.3f) zoom -= 0.1;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool valid_range(float index) { 
	return (index <= DIM && index >= -1.0f); 
}

bool pressed_shift(void) { 
	return (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
}

bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.
		// rotating_rate = 0.0f;

		// BASIC
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true; 
		} 

		if (key == GLFW_KEY_D) {
			maze.digMaze();
			maze.printMaze();
			for (int i=0; i<DIM; i++) {
				for (int j=0; j<DIM;j++) {
					if ((i == 0 || i == DIM - 1 || j == 0 || j == DIM - 1) && maze.getValue(i,j) == 0) {
						avatar_pos = vec3(j,0,i);
						// cout << "DEBUG" <<  i << j << endl;
					}
				}
			}
			eventHandled = true; 
		} 

		if (key == GLFW_KEY_R) {
			maze.reset();
			defaultSettings();
			eventHandled = true; 
		} 

		// WALL HEIGHT SCALING
		if (key == GLFW_KEY_SPACE) {
			if (cube_height < SCALE_MAX_RANGE) cube_height += SCALE_RATE;
			// Rerender cube triangles
			initCube();
			eventHandled = true; 
		} 
		
		if (key == GLFW_KEY_BACKSPACE) {
			if (cube_height > SCALE_MIN_RANGE) cube_height -= SCALE_RATE;
			initCube();
			eventHandled = true;
		}

		// AVATAR MOVING CONTROLS
		if (key == GLFW_KEY_DOWN && valid_range(avatar_pos.z + 1)) {
			if (BOUNDARIES.count(avatar_pos.x) || BOUNDARIES.count(avatar_pos.z + 1)) {
				avatar_pos.z += 1;
			} else if (pressed_shift()) {
				maze.setValue(avatar_pos.z + 1, avatar_pos.x, 0);
				avatar_pos.z += 1;
			} else if (maze.getValue(avatar_pos.z + 1, avatar_pos.x) != 1) {
				avatar_pos.z += 1;
			}
			eventHandled = true;
		}

		if (key == GLFW_KEY_UP && valid_range(avatar_pos.z - 1)) {
			if (BOUNDARIES.count(avatar_pos.x) || BOUNDARIES.count(avatar_pos.z - 1) ) {
				avatar_pos.z -= 1;
			} else if (pressed_shift()) {
				maze.setValue(avatar_pos.z - 1, avatar_pos.x, 0);
				avatar_pos.z -= 1;
			} else if (maze.getValue(avatar_pos.z - 1, avatar_pos.x) != 1) {
				avatar_pos.z -= 1;
			}
			eventHandled = true;
		}

		if (key == GLFW_KEY_RIGHT && valid_range(avatar_pos.x + 1)) {
			if (BOUNDARIES.count(avatar_pos.z) || BOUNDARIES.count(avatar_pos.x + 1)) {
				avatar_pos.x += 1;
			} else if (pressed_shift()) {
				maze.setValue(avatar_pos.z, avatar_pos.x + 1, 0);
				avatar_pos.x += 1;
			} else if (maze.getValue(avatar_pos.z, avatar_pos.x + 1) != 1) {
				avatar_pos.x += 1;
			}
			// cout << avatar_pos.x << " " << avatar_pos.y << " " << avatar_pos.z << endl;
			eventHandled = true;
		}

		if (key == GLFW_KEY_LEFT && valid_range(avatar_pos.x - 1)) {
			if (BOUNDARIES.count(avatar_pos.z) || BOUNDARIES.count(avatar_pos.x - 1)) {
				avatar_pos.x -= 1;
			} else if (pressed_shift()) {
				maze.setValue(avatar_pos.z, avatar_pos.x - 1, 0);
				avatar_pos.x -= 1;
			} else if (maze.getValue(avatar_pos.z, avatar_pos.x - 1) != 1) {
				avatar_pos.x -= 1;
			}
			
			// cout << avatar_pos.x << " " << avatar_pos.y << " " << avatar_pos.z << endl;
			eventHandled = true;
		}
	}

	return eventHandled;
}

