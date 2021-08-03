// Termm--Fall 2020

#include <glm/ext.hpp>

#include "A4.hpp"

// Added
#include "Ray.hpp"
#include "HitRecord.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"
#include "ExtraFeature.hpp"

using namespace std;
using namespace glm;

static const float PADDING = 0.0001;

float random_float() {
    float x;
    do {
        x = (float) rand() / (RAND_MAX);
    } while (x == 1);
    return x;
}

vec3 random_small_vec() {
	vec3 p;
	do {
        p = 2.0 * (vec3(random_float(), random_float(), 0)) - vec3(1, 1, 0);
    } while (length2(p) >= 1.0);
    return p;
}

vec3 trace_color(
	Ray &ray,
	SceneNode *root,
	const glm::vec3 & eye,
	const glm::vec3 & ambient,
	const std::list<Light *> & lights,
	const int maxHits = 5
) {
	HitRecord record;
    vec3 color;

	if ( root->hit( ray, 0, numeric_limits<float>::max(), record ) ) {
		record.normal = normalize(record.normal);
		record.hit_point += record.normal * PADDING;

		PhongMaterial *material = static_cast<PhongMaterial *>(record.material);

		color += material->diffuse() * ambient;

		for (Light * light : lights) {
			Ray shadowRay(record.hit_point, light->position - record.hit_point);
			HitRecord shadowRay_record;
			
			if (root->hit( shadowRay, 0, numeric_limits<float>::max(), shadowRay_record)) {
				continue;
			}

			vec3 L = normalize(shadowRay.Get_direction());
			vec3 N = normalize(record.normal);
			vec3 R = normalize(2 * N * dot(N, L) - L);
			vec3 V = normalize(eye - record.hit_point);
			double r = length(shadowRay.Get_direction());

			double attenuation = 1.0 / ( light->falloff[0] + light->falloff[1] * r + light->falloff[2] * r * r );

			color += dot(L, N) * attenuation * material->diffuse() * light->colour;

			color += pow(glm::max(0.0, (double)dot(R, V)), material->shininess()) * attenuation * material->specular() * light->colour;
		}

		if (maxHits > 0) {
			vec3 reflection_direction = ray.Get_direction() - 2 * record.normal * dot(ray.Get_direction(), record.normal);
            Ray reflection_ray(record.hit_point, reflection_direction);
			float reflect_coef = 0.2;
			color = glm::mix(color, trace_color(reflection_ray, root, eye, ambient, lights, maxHits - 1), reflect_coef);
		}
	}
	return color; 
}

void A4_Render(
		// What to render  
		SceneNode * root,

		// Image to write to, set to a given width and height  
		Image & image,

		// Viewing parameters  
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters  
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

  // Fill in raytracing code here...  

  std::cout << "F20: Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	vec3 _w = normalize(view); // z-axis
	vec3 _u = normalize(cross(_w, up)); // x-axis
	vec3 _v = cross(_u, _w); // y-axis
	float d = h / 2 / glm::tan(glm::radians(fovy / 2));
	vec3 BL_corner_direction = _w * d - _u * (float)w / 2 - _v * (float)h / 2;

	int barWidth = 70;

	for (uint y = 0; y < h; ++y) {
		float _y = (float) y / h;
		for (uint x = 0; x < w; ++x) {
			float _x = (float) x / w;
			int _x_chunk = (float) x / w * 8;

			
			const vec3 direction = BL_corner_direction + (float)(h - y) * _v + (float)x * _u;
			Ray ray = Ray(eye, direction);

			vec3 color = vec3(0.0, 0.0, 0.0);

			size_t AA = 1;
			if (ENABLE_ANTI_ALIASING == 1) {
				size_t AA = 10;
			}

            for ( unsigned int i = 0; i < AA; ++i ) {
				if (ENABLE_ANTI_ALIASING == 1) {
                    ray.direction = direction + random_small_vec() * ( _u + _v ) * 0.5;
				} else {
                    ray.direction = direction;
                }

				if (MIRROR_REFLECTIONS == 1) {
                	color += trace_color(ray, root, eye, ambient, lights, 6);
				} else {
					color += trace_color(ray, root, eye, ambient, lights, 0);
				}
            }
            color /= AA;

			// No signal background base image
			// https://www.google.com/imgres?imgurl=https%3A%2F%2Fst3.depositphotos.com%2F9215172%2F19476%2Fv%2F600%2Fdepositphotos_194766548-stock-illustration-tv-no-signal-static-screen.jpg&imgrefurl=https%3A%2F%2Fdepositphotos.com%2Fvector-images%2Ftv-no-signal-design.html&tbnid=WG16nQ4fJzUK2M&vet=12ahUKEwiq3IWZzPXxAhUHQa0KHbKwBYoQMygKegUIARDsAQ..i&docid=-O_UH2L7Uh8dFM&w=600&h=400&q=tv%20no%20signal&client=ubuntu&ved=2ahUKEwiq3IWZzPXxAhUHQa0KHbKwBYoQMygKegUIARDsAQ
			if (color.r == 0 && color.g == 0 && color.b == 0) {
				if (_y <= 0.7) {
					switch (_x_chunk) {
						case 0:
							color += vec3(1.0, 1.0, 1.0);
							break;
						case 1:
							color += vec3(1.0, 1.0, 0.0);
							break;
						case 2:
							color += vec3(0.0, 1.0, 1.0);
							break;
						case 3:
							color += vec3(0.0, 1.0, 0.0);
							break;
						case 4:
							color += vec3(1.0, 0.0, 1.0);
							break;
						case 5:
							color += vec3(1.0, 0.0, 0.0);
							break;
						case 6:
							color += vec3(0.0, 0.0, 1.0);
							break;
					};
				} else if (_y <= 0.85) {
					// Gradient
					color += (1 - _x) * vec3(1.0, 1.0, 1.0);
				} else {
					color += (8 - _x_chunk) / 8.0 * vec3(1.0, 1.0, 1.0);
				}
			}
			// Red: 
			image(x, y, 0) = (double)color.r;
			// Green: 
			image(x, y, 1) = (double)color.g;
			// Blue: 
			image(x, y, 2) = (double)color.b;
		}

		// Progress bar
		// https://stackoverflow.com/questions/14539867/how-to-display-a-progress-indicator-in-pure-c-c-cout-printf

		cout << "[";
		int pos = barWidth * _y;
		for (int i = 0; i < barWidth; ++i) {
			if (i < pos) std::cout << "=";
			else if (i == pos) std::cout << ">";
			else std::cout << " ";
		}
		cout << "] " << int(_y * 100.0) << " %\r";
		cout.flush();
	}
	cout << endl;
	cout << "Done" << endl;
}
