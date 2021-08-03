// Termm--Fall 2020

#include "GeometryNode.hpp"

using namespace std;
using namespace glm;

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
	const string & name, Primitive *prim, Material *mat )
	: SceneNode( name )
	, m_material( mat )
	, m_primitive( prim )
{
	m_nodeType = NodeType::GeometryNode;
}

void GeometryNode::setMaterial( Material *mat )
{
	// Obviously, there's a potential memory leak here.  A good solution
	// would be to use some kind of reference counting, as in the 
	// C++ shared_ptr.  But I'm going to punt on that problem here.
	// Why?  Two reasons:
	// (a) In practice we expect the scene to be constructed exactly
	//     once.  There's no reason to believe that materials will be
	//     repeatedly overwritten in a GeometryNode.
	// (b) A ray tracer is a program in which you compute once, and 
	//     throw away all your data.  A memory leak won't build up and
	//     crash the program.

	m_material = mat;
}

// Hit logic
bool GeometryNode::hit(
	Ray &ray, 
	float t_min, 
	float t_max, 
	HitRecord &record
) {
	Ray transformedRay(vec3(invtrans * vec4(ray.origin, 1.0)), vec3(invtrans * vec4(ray.direction, 0.0)));

	HitRecord tmp_record;
	float closest = t_max;
	tmp_record.material = nullptr;
	bool result = false;


	if (m_primitive->hit(transformedRay, t_min, t_max, tmp_record)) {
		if ( tmp_record.material == nullptr ) 
			tmp_record.material = m_material;
		result = true;
		closest = tmp_record.t;
		record = tmp_record;
	}

	if ( SceneNode::hit(ray, t_min, closest, tmp_record) ) {
		result = true;
		record = tmp_record;
		closest = tmp_record.t;
	}

	if (result) {
		record.normal = mat3(transpose(invtrans)) * record.normal;
		record.hit_point = vec3(trans * vec4(record.hit_point, 1.0));
	}

	return result;
}