// Termm--Fall 2020

#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <functional>
using namespace std;


#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

float static const FOV_LOW = 5.0f;
float static const FOV_HIGH = 160.0f;
float static const FOV_FACTOR = 0.5f;
float static const FACTOR = 0.01f;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f))
{

}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();

	// TODO
	initCube();
	initCoordinatesFrame();
	reset();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//---------------------------------------------------------------------------------------- Spring 2020
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & V0,   // Line Start (NDC coordinate)
		const glm::vec2 & V1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = V0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = V1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}


glm::vec4 A2::homogenize(glm::vec4& v) {
	return glm::vec4{
		v.x / v.w,
		v.y / v.w,
		v.z / v.w,
		1.0f
	};
}

// TODO
void A2::reset() {
	// Default mode
	mode = RotateModel;

	// Model info
	model = mat4(1.0f);
	modelTransformation = mat4(1.0f);
	modelScale = mat4(1.0f);

	// Viewport 5% margin
	viewportX1 = 0.05 * m_framebufferWidth;
	viewportX2 = 0.95 * m_framebufferWidth;
	viewportY1 = 0.05 * m_framebufferHeight;
	viewportY2 = 0.95 * m_framebufferHeight;

	// View matrix
	viewMatrix = createViewMatrix();
	viewTransformation = mat4(1.0f);

	// Cliping planes and FOV
	near = 5.0f;
	far = 20.0f;
	fov = 30.0f; // Field of viewMatrix suggested 30

	// Create proj
	projMatrix = createProj();
}

glm::mat4 A2::createViewMatrix() {
	// From pseudo code
	glm::vec3 lookAt(0.0f,0.0f,-1.0f);
	glm::vec3 lookFrom(0.0f, 0.0f, 15.0f); // TODO: Check this default range
	glm::vec3 lookUp(0.0f, 1.0f, 15.0f);

	glm::vec3 z((lookAt - lookFrom) / glm::distance(lookAt,lookFrom));
	glm::vec3 x(glm::cross(z,lookUp) / glm::distance(glm::cross(z,lookUp), glm::vec3(0.0f)));
	glm::vec3 y(glm::cross(x,z));

	glm::mat4 R {
		vec4(x,0),
		vec4(y,0),
		vec4(z,0),
		vec4(0.0f, 0.0f, 0.0f, 1.0f),
	};
	R = glm::transpose(R);
	glm::mat4 T (1.0f);
	T[3].z = -lookFrom.z;

	return R * T;
}

glm::mat4 A2::createProj() {
	// from pseudocode
	float aspect = (float)(abs(viewportX1-viewportX2)) / (float)(abs(viewportY1 - viewportY2));
	float theta = glm::radians(fov);
	float cot = cos(theta/2) / sin(theta/2);

	glm::mat4 P(
		cot / aspect, 0, 0 ,0,
		0, cot, 0, 0,
		0, 0, (far + near)/(far - near), 1,
		0, 0, -2 * far * near / (far - near), 0
	);

	return P;
}

void A2::initCube() {
	// 12 lines (24 coords)
	// x - y - z
	// Deduplicated
	cubeVectors.push_back(vec4(-1.0f,-1.0f,-1.0f, 1.0f));
	cubeVectors.push_back(vec4(1.0f,-1.0f,-1.0f, 1.0f));

	cubeVectors.push_back(vec4(-1.0f,-1.0f,-1.0f, 1.0f));
	cubeVectors.push_back(vec4(-1.0f,1.0f,-1.0f, 1.0f));

	cubeVectors.push_back(vec4(-1.0f,1.0f,-1.0f, 1.0f));
	cubeVectors.push_back(vec4(1.0f,1.0f,-1.0f, 1.0f));

	cubeVectors.push_back(vec4(1.0f,1.0f,-1.0f, 1.0f));
	cubeVectors.push_back(vec4(1.0f,-1.0f,-1.0f, 1.0f));

	cubeVectors.push_back(vec4(-1.0f,-1.0f,-1.0f, 1.0f));
	cubeVectors.push_back(vec4(-1.0f,-1.0f,1.0f, 1.0f));
	
	cubeVectors.push_back(vec4(-1.0f,1.0f,-1.0f, 1.0f));
	cubeVectors.push_back(vec4(-1.0f,1.0f,1.0f, 1.0f));

	cubeVectors.push_back(vec4(1.0f,1.0f,-1.0f, 1.0f));
	cubeVectors.push_back(vec4(1.0f,1.0f,1.0f, 1.0f));

	cubeVectors.push_back(vec4(1.0f,-1.0f,1.0f, 1.0f));
	cubeVectors.push_back(vec4(1.0f,-1.0f,-1.0f, 1.0f));

	cubeVectors.push_back(vec4(1.0f,-1.0f,1.0f, 1.0f));
	cubeVectors.push_back(vec4(1.0f,1.0f,1.0f, 1.0f));

	cubeVectors.push_back(vec4(1.0f,-1.0f,1.0f, 1.0f));
	cubeVectors.push_back(vec4(-1.0f,-1.0f,1.0f, 1.0f));

	cubeVectors.push_back(vec4(-1.0f,1.0f,1.0f, 1.0f));
	cubeVectors.push_back(vec4(1.0f,1.0f,1.0f, 1.0f));

	cubeVectors.push_back(vec4(-1.0f,1.0f,1.0f, 1.0f));
	cubeVectors.push_back(vec4(-1.0f,-1.0f,1.0f, 1.0f));

	// // xy - back TODO
	// cubeVectors.push_back(vec4(1.0f, 1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, -1.0f, -1.0f, 1.0f));

	// cubeVectors.push_back(vec4(1.0f, 1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(-1.0f, 1.0f, -1.0f, 1.0f));

	// cubeVectors.push_back(vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(-1.0f, 1.0f, -1.0f, 1.0f));

	// cubeVectors.push_back(vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, -1.0f, -1.0f, 1.0f));

	// // xy - front
	// cubeVectors.push_back(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, -1.0f, 1.0f, 1.0f));

	// cubeVectors.push_back(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// cubeVectors.push_back(vec4(-1.0f, 1.0f, 1.0f, 1.0f));

	// cubeVectors.push_back(vec4(-1.0f, -1.0f, 1.0f, 1.0f));
	// cubeVectors.push_back(vec4(-1.0f, 1.0f, 1.0f, 1.0f));

	// cubeVectors.push_back(vec4(-1.0f, -1.0f, 1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, -1.0f, 1.0f, 1.0f));

	// // xz - top
	// cubeVectors.push_back(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, 1.0f, -1.0f, 1.0f));

	// cubeVectors.push_back(vec4(-1.0f, 1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(-1.0f, 1.0f, 1.0f, 1.0f));

	// cubeVectors.push_back(vec4(-1.0f, 1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, 1.0f, -1.0f, 1.0f));

	// // xz - bottom
	// cubeVectors.push_back(vec4(1.0f, -1.0f, 1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, -1.0f, -1.0f, 1.0f));

	// cubeVectors.push_back(vec4(1.0f, -1.0f, 1.0f, 1.0f));
	// cubeVectors.push_back(vec4(-1.0f, -1.0f, 1.0f, 1.0f));

	// cubeVectors.push_back(vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(-1.0f, -1.0f, 1.0f, 1.0f));

	// cubeVectors.push_back(vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, -1.0f, -1.0f, 1.0f));

	// // yz - right
	// cubeVectors.push_back(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, 1.0f, -1.0f, 1.0f));

	// cubeVectors.push_back(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, -1.0f, 1.0f, 1.0f));

	// cubeVectors.push_back(vec4(1.0f, -1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, -1.0f, 1.0f, 1.0f));

	// cubeVectors.push_back(vec4(1.0f, -1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(1.0f, 1.0f, -1.0f, 1.0f));

	// // yz - left
	// cubeVectors.push_back(vec4(-1.0f, 1.0f, 1.0f, 1.0f));
	// cubeVectors.push_back(vec4(-1.0f, 1.0f, -1.0f, 1.0f));

	// cubeVectors.push_back(vec4(-1.0f, 1.0f, 1.0f, 1.0f));
	// cubeVectors.push_back(vec4(-1.0f, -1.0f, 1.0f, 1.0f));

	// cubeVectors.push_back(vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(-1.0f, -1.0f, 1.0f, 1.0f));

	// cubeVectors.push_back(vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	// cubeVectors.push_back(vec4(-1.0f, 1.0f, -1.0f, 1.0f));
}

void A2::initCoordinatesFrame() {
	// x
	coordinateVectors.push_back(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	coordinateVectors.push_back(vec4(1.0f, 0.0f, 0.0f, 1.0f));

	// y
	coordinateVectors.push_back(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	coordinateVectors.push_back(vec4(0.0f, 1.0f, 0.0f, 1.0f));

	// z
	coordinateVectors.push_back(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	coordinateVectors.push_back(vec4(0.0f, 0.0f, 1.0f, 1.0f));
}


bool A2::clipping(glm::vec4& A, glm::vec4 & B) {
	std::function<float (glm::vec4&)> BL = [](glm::vec4& p) { return p.x + p.w; };
	std::function<float (glm::vec4&)> BR = [](glm::vec4& p) { return p.w - p.x; };
	std::function<float (glm::vec4&)> BB = [](glm::vec4& p) { return p.w + p.y; };
	std::function<float (glm::vec4&)> BT = [](glm::vec4& p) { return p.w - p.y; };
	std::function<float (glm::vec4&)> BN = [](glm::vec4& p) { return p.z + p.w; };
	std::function<float (glm::vec4&)> BF = [](glm::vec4& p) { return p.w - p.z; };

	std::vector<std::function<float (glm::vec4&)>> boundaries {
		BL, BR, BB, BT, BN, BF
	};

	for (auto clipping : boundaries) {
		float boundary1 = clipping(A);
		float boundary2 = clipping(B);

		if (boundary1 < 0 && boundary2 < 0) {
			return false;
		}
		if (boundary1 >= 0 && boundary2 >= 0) {
			continue;
		}
		float a = boundary1 / (boundary1 - boundary2);
		if (boundary1 < 0) {
			A = (1-a)*A + a * B;
		} else {
			B = (1-a)*A + a * B;
		}
	}
	return true;
}

void A2::rotateView(double xPos, double yPos) {
	float offset = (xPos - prevMouseX) * FACTOR;
	if (leftMouse) {
		mat4 R(1.0f);
		R[1][1] = cos(offset);
		R[1][2] = sin(offset);
		R[2][1] = -sin(offset);
		R[2][2] = cos(offset);
		glm::mat4 inverse_R = glm::mat4(1.0f) / R;
		viewTransformation *= inverse_R;
	}
	if (middleMouse) {
		mat4 R(1.0f);
		R[0][0] = cos(offset);
		R[2][0] = sin(offset);
		R[0][2] = -sin(offset);
		R[2][2] = cos(offset);
		glm::mat4 inverse_R = glm::mat4(1.0f) / R;
		viewTransformation *= inverse_R;
	}
	if (rightMouse) {
		mat4 R(1.0f);
		R[0][0] = cos(offset);
		R[0][1] = sin(offset);
		R[1][0] = -sin(offset);
		R[1][1] = cos(offset);
		glm::mat4 inverse_R = glm::mat4(1.0f) / R;
		viewTransformation *= inverse_R;
	}
}

void A2::rotateModel(double xPos, double yPos) {
	// pseudocode
	float offset = (xPos - prevMouseX) * FACTOR;
	if (leftMouse) {
		mat4 M(1.0f);
		M[1][1] = cos(offset);
		M[1][2] = sin(offset);
		M[2][1] = -sin(offset);
		M[2][2] = cos(offset);
		modelTransformation *= M;
	} 
	if (middleMouse) {
		mat4 M(1.0f);
		M[0][0] = cos(offset);
		M[2][0] = sin(offset);
		M[0][2] = -sin(offset);
		M[2][2] = cos(offset);
		modelTransformation *= M;
	}
	if (rightMouse) {
		mat4 M(1.0f);
		M[0][0] = cos(offset);
		M[0][1] = sin(offset);
		M[1][0] = -sin(offset);
		M[1][1] = cos(offset);
		modelTransformation *= M;
	}
}

void A2::translateModel(double xPos, double yPos) {
	// pseudocode
	float offset = (xPos - prevMouseX) * FACTOR;
	if (leftMouse) {
		mat4 T(1.0f);
		T[3].x = offset;
		modelTransformation *= T;
	}
	if (middleMouse) {
		mat4 T(1.0f);
		T[3].y = offset;
		modelTransformation *= T;
	}
	if (rightMouse) {
		mat4 T(1.0f);
		T[3].z = offset;
		modelTransformation *= T;
	}
}

void A2::scaleModel(double xPos, double yPos) {
	float offset = (xPos - prevMouseX) * FACTOR;
	if (leftMouse) {
		mat4 S(1.0f);
		if (S[0].x + offset > 0) S[0].x += offset;
		modelScale *= S;
	}
	if (middleMouse) {
		mat4 S(1.0f);
		if (S[1].y + offset > 0) S[1].y += offset;
		modelScale *= S;
	}
	if (rightMouse) {
		mat4 S(1.0f);
		if (S[2].z + offset > 0) S[2].z += offset;
		modelScale *= S;
	}
}

void A2::perspective(double xPos, double yPos) {
	float offset = (xPos - prevMouseX);
	if (leftMouse) {
		fov += offset * FOV_FACTOR;
		fov = std::max(fov, FOV_LOW);
		fov = std::min(fov, FOV_HIGH);
		projMatrix = createProj();
	}
	if (middleMouse) {
		near = std::min(far, near + offset * FACTOR);
		projMatrix = createProj();
	}
	if (rightMouse) {
		far = std::max(near, far + offset * FACTOR);
		projMatrix = createProj();
	}
}

void A2::viewport(double xPos,double yPos) {
	if (leftMouse) {
		viewportX2 = glm::clamp((float)xPos, 0.0f, (float)m_framebufferWidth);
		viewportY2 = glm::clamp((float)yPos, 0.0f, (float)m_framebufferHeight);
	}
}

void A2::translateView(double xPos, double yPos) {
	float offset = (xPos - prevMouseX) * FACTOR;
	if (leftMouse) {
		mat4 T(1.0f);
		T[3].x = offset;
		glm::mat4 inverse_T = glm::mat4(1.0f) / T;
		viewTransformation *= inverse_T;
	}
	if (middleMouse) {
		mat4 T(1.0f);
		T[3].y = offset;
		glm::mat4 inverse_T = glm::mat4(1.0f) / T;
		viewTransformation *= inverse_T;
	}
	if (rightMouse) {
		mat4 T(1.0f);
		T[3].z = offset;
		glm::mat4 inverse_T = glm::mat4(1.0f) / T;
		viewTransformation *= inverse_T;
	}
}



//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();

	// Viewport calculation
	// Support sketching in both directions
	float viewportLeft = viewportX1 < viewportX2 ? viewportX1 : viewportX2;
	float viewportRight = viewportX1 < viewportX2 ? viewportX2 : viewportX1;
	float viewportTop = viewportY1 < viewportY2 ? viewportY1 : viewportY2;
	float viewportBottom = viewportY1 < viewportY2 ? viewportY2 : viewportY1;

	float xScale = (viewportRight - viewportLeft) / m_framebufferWidth;
	float yScale = (viewportBottom - viewportTop) / m_framebufferHeight;

	float normLeft = (viewportLeft - m_framebufferWidth / 2) / (m_framebufferWidth / 2);
	float normRight = (viewportRight - m_framebufferWidth / 2) / (m_framebufferWidth / 2);
	float normTop = (m_framebufferHeight / 2 - viewportTop) / (m_framebufferHeight / 2);
	float normBottom = (m_framebufferHeight / 2 - viewportBottom) / (m_framebufferHeight / 2);

	float xShift = (normLeft + normRight) / 2;
	float yShift = (normTop + normBottom) / 2;

	// Draw cube
	for (int i=0; i < cubeVectors.size(); i += 2) {
		if (i < 8) {
			setLineColour(vec3(1.0f, 0.7f, 0.8f));
		} else if (i < 16) {
			setLineColour(vec3(1.0f, 0.0f, 1.0f));
		} else {
			setLineColour(vec3(0.2f, 1.0f, 1.0f));
		}

		// Translation
		glm::vec4 A = projMatrix * viewTransformation * viewMatrix * model * modelTransformation * modelScale * cubeVectors[i] ;
		glm::vec4 B = projMatrix * viewTransformation * viewMatrix * model * modelTransformation * modelScale * cubeVectors[i + 1];

		if (clipping(A, B)) {
			A = homogenize(A);
			B = homogenize(B);
			
			drawLine(
				glm::vec2(A.x * xScale + xShift, A.y * yScale + yShift),
				glm::vec2(B.x * xScale + xShift, B.y * yScale + yShift)
			);
		}

	}

	// Draw world coordinates
	for (int i=0; i < coordinateVectors.size(); i+=2) {
		// rgb
		if (i < 2) {
			setLineColour(vec3(1.0f, 1.0f, 0.0f));
		} else if (i < 4) {
			setLineColour(vec3(0.0f, 1.0f, 1.0f));
		} else {
			setLineColour(vec3(1.0f, 0.0f, 1.0f));
		}

		glm::vec4 A = projMatrix * viewTransformation * viewMatrix * coordinateVectors[i] ;
		glm::vec4 B = projMatrix * viewTransformation * viewMatrix * coordinateVectors[i + 1];

		if (clipping(A, B)) {
			A = homogenize(A);
			B = homogenize(B);

			drawLine(
				glm::vec2(A.x * xScale + xShift, A.y * yScale + yShift),
				glm::vec2(B.x * xScale + xShift, B.y * yScale + yShift)
			);
		}
	}

	// Draw model coordinates
	for (int i=0; i < coordinateVectors.size(); i+=2) {
		// rgb
		if (i < 2) {
			setLineColour(vec3(1.0f, 0.0f, 0.0f));
		} else if (i < 4) {
			setLineColour(vec3(0.0f, 1.0f, 0.0f));
		} else {
			setLineColour(vec3(0.0f, 0.0f, 1.0f));
		}

		glm::vec4 A = projMatrix * viewTransformation * viewMatrix * model * modelTransformation  * coordinateVectors[i];
		glm::vec4 B = projMatrix * viewTransformation * viewMatrix * model * modelTransformation  * coordinateVectors[i + 1];

		if (clipping(A, B)) {
			A = homogenize(A);
			B = homogenize(B);

			drawLine(
				glm::vec2(A.x * xScale + xShift, A.y * yScale + yShift),
				glm::vec2(B.x * xScale + xShift, B.y * yScale + yShift)
			);
		}

	}

	// Draw viewMatrix port
	setLineColour(vec3(0.1f, 0.1f, 0.1f));
	drawLine(glm::vec2(normRight, normBottom), glm::vec2(normLeft, normBottom));
	drawLine(glm::vec2(normRight, normBottom), glm::vec2(normRight, normTop));
	drawLine(glm::vec2(normLeft, normTop), glm::vec2(normLeft, normBottom));
	drawLine(glm::vec2(normLeft, normTop), glm::vec2(normRight, normTop));
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...
		ImGui::RadioButton("Rotate View (o)", (int*)&mode, RotateView);
        ImGui::RadioButton("Translate View (e)", (int*)&mode, TranslateView);
        ImGui::RadioButton("Perspective (p)", (int*)&mode, Perspective);
		ImGui::RadioButton("Rotate Model (r)", (int*)&mode, RotateModel);
        ImGui::RadioButton("Translate Model (t)", (int*)&mode, TranslateModel);
        ImGui::RadioButton("Scale Model (s)", (int*)&mode, ScaleModel);
		ImGui::RadioButton("Viewport (v)", (int*)&mode, Viewport);

		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Reset (a)" ) ) {
			reset();
		}
		if( ImGui::Button( "Quit Application (q)" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		
		ImGui::Text( "Near Plane: %.1f, Far Plane: %.1f", near, far);
		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
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
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		switch(mode) {
			case RotateModel:
				A2::rotateModel(xPos, yPos);
				break;
			case ScaleModel:
				A2::scaleModel(xPos, yPos);
				break;
			case TranslateModel:
				A2::translateModel(xPos, yPos);
				break;
			case RotateView:
				A2::rotateView(xPos, yPos);
				break;
			case TranslateView:
				A2::translateView(xPos, yPos);
				break;
			case Perspective:
				A2::perspective(xPos, yPos);
				break;
			case Viewport:
				A2::viewport(xPos, yPos);
				break;
			default:
				break;
		}
	}

	prevMouseX = xPos;
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			leftMouse = actions == GLFW_PRESS;
			if (leftMouse && mode == Viewport) {
				// viewport case
				viewportX1 = glm::clamp(ImGui::GetMousePos().x, 0.0f, (float)m_framebufferWidth);
				viewportY1 = glm::clamp(ImGui::GetMousePos().y, 0.0f, (float)m_framebufferHeight);
				viewportX2 = viewportX1;
				viewportY2 = viewportY1;
			}
		} 

		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			middleMouse = actions == GLFW_PRESS;
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			rightMouse = actions == GLFW_PRESS;
		}
	}
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_A) {
			reset();
			eventHandled = true;
		}

		if (key == GLFW_KEY_E) {
			mode = TranslateView;
			eventHandled = true;
		}

		if (key == GLFW_KEY_O) {
			mode = RotateView;
			eventHandled = true;
		}

		if (key == GLFW_KEY_P) {
			mode = Perspective;
			eventHandled = true;
		}

		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}

		if (key == GLFW_KEY_R) {
			mode = RotateModel;
			eventHandled = true;
		}

		if (key == GLFW_KEY_S) {
			mode = ScaleModel;
			eventHandled = true;
		}

		if (key == GLFW_KEY_T) {
			mode = TranslateModel;
			eventHandled = true;
		}

		if (key == GLFW_KEY_V) {
			mode = Viewport;
			eventHandled = true;
		}
	}

	return eventHandled;
}
