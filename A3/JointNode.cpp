// Termm-Fall 2020

#include "JointNode.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "cs488-framework/MathUtils.hpp"
#include <iostream>
using namespace std;
using namespace glm;
//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name)
{
	m_nodeType = NodeType::JointNode;
	angleX = 0.0f;
	angleY = 0.0f;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}

float clamp(float x, float min, float max) {
	if (x > max) return max;
	if (x < min) return min;
	return x;
}

void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;
	rotate('x', m_joint_x.init);
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;

	rotate('y', m_joint_y.init);
}

//---------------------------------------------------------------------------------------
void JointNode::rotate(char axis, float angle) {
	if (angle == 0.0f) return ;
	vec3 rot_axis;
	if (axis == 'x') {
		rot_axis = vec3(1,0,0);
		angle = clamp(angle, m_joint_x.min - angleX, m_joint_x.max - angleX);
		angleX += angle;
	} else if (axis == 'y') {
		rot_axis = vec3(0,1,0);
		angle = clamp(angle, m_joint_y.min - angleY, m_joint_y.max - angleY);
		angleY += angle;
	}
	
	mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);
	trans = rot_matrix * trans;
}