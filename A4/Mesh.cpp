// Fall 2020

#include <iostream>
#include <fstream>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

using namespace glm;
using namespace std;


// https://github.com/stharlan/oglwalker/blob/768c3fb65ebf831f20dae0747b72d9f959f1f121/OglWalker/GeometryOperations.cpp
bool triangleIntersection(Ray &ray, vec3 vertex0, vec3 vertex1, vec3 vertex2, float &res) {
	const float EPSILON = 0.0000001;
	glm::vec3 edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0; 

	h = glm::cross(ray.Get_direction(), edge2);
	a = glm::dot(edge1, h);
	if (a > -EPSILON && a < EPSILON)
		return false;
	f = 1 / a;
	s = ray.Get_origin() - vertex0;
	u = f * (glm::dot(s, h));
	if (u < 0.0 || u > 1.0)
		return false;
	q = glm::cross(s, edge1);
	v = f * glm::dot(ray.Get_direction(), q);
	if (v < 0.0 || u + v > 1.0)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.

	float t = f * glm::dot(edge2, q);
	if (t > EPSILON) // ray intersection
	{
		res = t;
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
    return false;
}


Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	// cout << fname << " Mesh constructor" << endl;
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}

bool Mesh::hit(Ray &ray, float t_min, float t_max, HitRecord &record) {	
	if (RENDER_BOUNDING_VOLUMES == 1) {
		HitRecord tmpRec = record;
		Ray tmpRay = ray;
		vec3 dis_oc = tmpRay.Get_origin() - m_pos;

		double A = dot(tmpRay.Get_direction(), tmpRay.Get_direction());
		double B = 2 * dot(tmpRay.Get_direction(), dis_oc);
		double C = dot(dis_oc, dis_oc) - m_radius * m_radius;

		double roots[2];
		size_t n_roots =  quadraticRoots(A, B, C, roots);

		float t = 0;
		if (n_roots == 0) {
			// return false;
		} else if (n_roots == 1) {
			t = roots[0];
		} else {
			t = glm::min(roots[0], roots[1]);
		}

		if ( t <= t_min || t >= t_max ) return false;
			
			record.t = t;
			record.hit_point = ray.At_t(t);
			record.normal = record.hit_point - m_pos;
			record.material = nullptr;

			return true;
	}
	
	float cur_min_t = t_max;
	vec3 normal = vec3();
    bool hit = false;

	for ( auto triangle : m_faces ) {
        float tmp_t;
        if ( triangleIntersection(ray, m_vertices[triangle.v1], m_vertices[triangle.v2], m_vertices[triangle.v3], tmp_t) ) {
            if ( tmp_t < cur_min_t && tmp_t > t_min ) {
                hit = true;
                cur_min_t = tmp_t;
                normal = cross(m_vertices[triangle.v1] - m_vertices[triangle.v2], m_vertices[triangle.v2] - m_vertices[triangle.v3]);
            }
        }
    }
	if ( !hit ) return false;

	if ( dot( ray.Get_direction(), normal ) > 0 ) normal = -normal;

	record.t = cur_min_t;
	record.normal = normal;
	record.hit_point = ray.At_t(record.t);
	record.material = nullptr;

	return hit;
}

Mesh::Mesh( std::vector<glm::vec3>& all_vertices, const std::vector<glm::vec3> &faces) :
	m_vertices(all_vertices) {
	
	float Infinity = std::numeric_limits<float>::infinity();

	float minX = Infinity;
	float minY = Infinity;
	float minZ = Infinity;

	float maxX = -Infinity;
	float maxY = -Infinity;
	float maxZ = -Infinity;

	m_faces.resize(faces.size());
	for (size_t i = 0; i < faces.size(); i++) {
		Triangle tri((size_t)faces[i].x, (size_t)faces[i].y, (size_t)faces[i].z);

		m_faces[i] = tri;


	}


	for (const glm::vec3& v: m_vertices) {
		minX = std::min(minX, v.x);
		minY = std::min(minY, v.y);
		minZ = std::min(minZ, v.z);

		maxX = std::max(maxX, v.x);
		maxY = std::max(maxY, v.y);
		maxZ = std::max(maxZ, v.z);
	}
	
	const glm::vec3 min{minX, minY, minZ};
	const glm::vec3 max{maxX, maxY, maxZ};

	m_pos = (max + min) / 2;
	m_radius =  glm::length(max - min)/2;
	// cout << m_pos.x << m_pos.y << m_pos.z;
	// cout << m_radius;
}