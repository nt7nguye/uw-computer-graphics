// Termm--Fall 2020

#include <glm/ext.hpp>
#include <thread>
#include <iomanip>
#include <chrono>

#include "A5.hpp"

// Added
#include "Ray.hpp"
#include "HitRecord.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"
#include "ExtraFeature.hpp"

using namespace std;
using namespace glm;

static const float PADDING = 0.0001;
static const int noiseWidth = 512;
static const int noiseHeight = 512;
double noise[noiseWidth][noiseHeight];

struct ThreadData
{
	SceneNode *root;
	Image &image;
	const glm::vec3 &eye;
	const glm::vec3 &ambient;
	const std::list<Light *> &lights;
	const glm::vec3 &BL_corner_direction, &_u, &_v;
	const size_t &h, &w, x_start, x_end, y_start, y_end;
	size_t &done;
};

float random_float()
{
	float x;
	do
	{
		x = (float)rand() / (RAND_MAX);
	} while (x == 1);
	return x;
}

vec3 random_small_vec()
{
	vec3 p;
	do
	{
		p = 2.0 * (vec3(random_float(), random_float(), 0)) - vec3(1, 1, 0);
	} while (length2(p) >= 1.0);
	return p;
}

#ifdef ENABLE_TEXTURE_MAPPING // ================================== End texture mapping
double smoothNoise(double x, double y)
{
   //get fractional part of x and y
   double fractX = x - int(x);
   double fractY = y - int(y);

   //wrap around
   int x1 = (int(x) + noiseWidth) % noiseWidth;
   int y1 = (int(y) + noiseHeight) % noiseHeight;

   //neighbor values
   int x2 = (x1 + noiseWidth - 1) % noiseWidth;
   int y2 = (y1 + noiseHeight - 1) % noiseHeight;

   //smooth the noise with bilinear interpolation
   double value = 0.0;
   value += fractX     * fractY     * noise[y1][x1];
   value += (1 - fractX) * fractY     * noise[y1][x2];
   value += fractX     * (1 - fractY) * noise[y2][x1];
   value += (1 - fractX) * (1 - fractY) * noise[y2][x2];

   return value;
}

double turbulence(double x, double y, double size)
{
  double value = 0.0, initialSize = size;

  while(size >= 1)
  {
    value += smoothNoise(x / size, y / size) * size;
    size /= 2.0;
  }
  return (128.0 * value / initialSize);
}

vec3 HSLtoRGB(float H, float S, float L) {
	float r, g, b, h, s, l; //this function works with floats between 0 and 1
    float temp1, temp2, tempr, tempg, tempb;
    h = H / 256.0;
    s = S/ 256.0;
    l = L / 256.0;
	if(s == 0) {
		r = g = b = l;
	} else
    {
        //Set the temporary values
        if(l < 0.5) temp2 = l * (1 + s);
        else temp2 = (l + s) - (l * s);
        temp1 = 2 * l - temp2;
        tempr = h + 1.0 / 3.0;
        if(tempr > 1) tempr--;
        tempg = h;
        tempb = h - 1.0 / 3.0;
        if(tempb < 0) tempb++;

        //Red
        if(tempr < 1.0 / 6.0) r = temp1 + (temp2 - temp1) * 6.0 * tempr;
        else if(tempr < 0.5) r = temp2;
        else if(tempr < 2.0 / 3.0) r = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempr) * 6.0;
        else r = temp1;

        //Green
        if(tempg < 1.0 / 6.0) g = temp1 + (temp2 - temp1) * 6.0 * tempg;
        else if(tempg < 0.5) g = temp2;
        else if(tempg < 2.0 / 3.0) g = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempg) * 6.0;
        else g = temp1;

        //Blue
        if(tempb < 1.0 / 6.0) b = temp1 + (temp2 - temp1) * 6.0 * tempb;
        else if(tempb < 0.5) b = temp2;
        else if(tempb < 2.0 / 3.0) b = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempb) * 6.0;
        else b = temp1;
    }
	return vec3((float)r, (float)g , (float)b);
}

vec3 cloud_texture_mapping(
	uint x,
	uint y)
{	
    float L = 192 + (uint)(turbulence(x, y, 64)) / 4;
	return HSLtoRGB(169.0, 255.0, L);
}

#endif // Background texture map


vec3 motion_blur_trace_color(Ray &ray,
            SceneNode *root,
            const glm::vec3 & eye,
            const glm::vec3 & ambient,
            const std::list<Light *> & lights) {
	HitRecord record;
    vec3 color(0.0f);
	// cout << "ray from " << to_string(ray.origin) << endl;
	// cout << "ray direction " << to_string(normalize(ray.direction)) << endl;

	if ( root->hit( ray, 0, numeric_limits<float>::max(), record ) ) {
		// hit
		record.normal = normalize(record.normal);
		// cout << "normal " << to_string(record.normal) << endl;
		// cout << "hit point " << to_string(record.hit_point) << endl;
		record.hit_point += record.normal * PADDING;

		PhongMaterial *material = static_cast<PhongMaterial *>(record.material);

		// ambient
		color += material->diffuse() * ambient;

		for (Light * light : lights) {
			Ray shadowRay(record.hit_point, light->position - record.hit_point);
			HitRecord shadowRay_record;
			
			// if light is blocked, skip calculating 
			if (root->hit( shadowRay, 0, numeric_limits<float>::max(), shadowRay_record)) {
			// length(shadowRay_record.hit_point - record.hit_point) <= length(light->position - record.hit_point)) {
				continue;
			}

			float soft_shadow_coef = 1;

			// cout << *light << " light isn't blocked" << endl;

			vec3 L = normalize(shadowRay.Get_direction());
			vec3 N = record.normal;
			vec3 R = normalize(2 * N * dot(N, L) - L);
			vec3 V = normalize(eye - record.hit_point);
			double r = length(shadowRay.Get_direction());

		// cout << "diffuse "<< dot(L, N) << endl;
		// cout << "specular " << dot(R, V) << ", " << material->shininess() << ", " << pow(glm::max(0.0, (double)dot(R, V)), material->shininess()) << endl;
		
			double attenuation = 1.0 / ( light->falloff[0] + light->falloff[1] * r + light->falloff[2] * r * r );

			// diffuse
			color += dot(L, N) * attenuation * material->diffuse() * light->colour * soft_shadow_coef;

			// specular
			color += pow(glm::max(0.0, (double)dot(R, V)), material->shininess()) * attenuation * material->specular() * light->colour * soft_shadow_coef;
		}

	}

	return color; 
}

vec3 trace_color(
	Ray &ray,
	SceneNode *root,
	const glm::vec3 &eye,
	const glm::vec3 &ambient,
	const std::list<Light *> &lights,
	const int maxHits = 5,
	const int refraction_counter = 0)
{
	HitRecord record;
	vec3 color;

	if (root->hit(ray, 0, numeric_limits<float>::max(), record))
	{
		record.normal = normalize(record.normal);
		record.hit_point += record.normal * PADDING;

		PhongMaterial *material = static_cast<PhongMaterial *>(record.material);

		color += material->diffuse() * ambient;

		for (Light *light : lights)
		{
			Ray shadowRay(record.hit_point, light->position - record.hit_point);
			HitRecord shadowRay_record;

			if (root->hit(shadowRay, 0, numeric_limits<float>::max(), shadowRay_record))
			{
				continue;
			}

			vec3 L = normalize(shadowRay.Get_direction());
			vec3 N = normalize(record.normal);
			vec3 R = normalize(2 * N * dot(N, L) - L);
			vec3 V = normalize(eye - record.hit_point);
			double r = length(shadowRay.Get_direction());

			double attenuation = 1.0 / (light->falloff[0] + light->falloff[1] * r + light->falloff[2] * r * r);

			color += dot(L, N) * attenuation * material->diffuse() * light->colour;

			color += pow(glm::max(0.0, (double)dot(R, V)), material->shininess()) * attenuation * material->specular() * light->colour;
		}

		if (maxHits > 0)
		{
			PhongMaterial* pm = dynamic_cast<PhongMaterial*>(record.material);
			if (pm != nullptr) {
				// reflection
				ray.direction = normalize(ray.Get_direction());
				float cos_theta = dot(ray.Get_direction(), record.normal);
				vec3 refleciton_color = vec3(0.0f);
#ifdef ENABLE_REFLECTION
				vec3 reflection_direction = ray.Get_direction() - 2 * record.normal * cos_theta;
				Ray reflection_ray(record.hit_point, reflection_direction);
				// cout << "relection ray " << to_string(reflection_ray.origin) << endl;
				// float reflect_coef = 0.2;
				refleciton_color = trace_color(reflection_ray, root, eye, ambient, lights, maxHits - 1);
#ifdef ENABLE_GLOSSY_REFLECTION
				reflection_direction = normalize(reflection_direction);
				vec3 reflect_orthonormal_u = cross(reflection_direction, record.normal);
				vec3 reflect_orthonormal_v = cross(reflect_orthonormal_u, reflection_direction);
				float reflection_a = 0.3f; // 2D square to blur
				int reflective_rays = 10 + 1;
				// cout << "reflective_rays " << reflective_rays << endl;
				refleciton_color /= reflective_rays;
				for (int i = 0; i < reflective_rays - 1; i++) {
					float u = -reflection_a/2 + random_float() * reflection_a;
					float v = -reflection_a/2 + random_float() * reflection_a;

					vec3 glossy_reflection = reflection_direction + u * reflect_orthonormal_u + v * reflect_orthonormal_v;
					glossy_reflection = normalize(glossy_reflection);
					float cosi = dot(glossy_reflection, reflection_direction);
					Ray glossy_reflection_ray(record.hit_point, glossy_reflection);
					// cout << to_string(glossy_reflection) << " weight " << cosi << endl;
					// cout << i << " is " << to_string(refleciton_color) << endl;
					refleciton_color += cosi / reflective_rays * trace_color(glossy_reflection_ray, root, eye, ambient, lights, 0, refraction_counter);
				}
				// cout << to_string(refleciton_color) << endl;
#endif
#endif
				// refraction 
				vec3 refraciton_color = vec3(0.0f);
#ifdef ENABLE_REFRACTION
				
				// if (refraction_counter % 2 == 1 && direction != vec3(0.0f)) {
				// 	// cout << refraction_counter << " straight back " << to_string(direction) << " \n";
				// 	Ray refraction_ray(record.hit_point - 2 * record.normal * PADDING, direction);
				// 	refraciton_color = trace_color(refraction_ray, root, eye, ambient, lights, maxHits - 1, direction, refraction_counter + 1);
				// } else {
				double eta = pm->refraction_ratio();
				if (refraction_counter % 2 == 1) eta = 1/eta;
				float c1 = -cos_theta;
				float c2 = (1-pow(eta,2)*(1-pow(c1,2)));
				// cout << "c2 is " << c2 << endl;
				if (c2 <= 0) return color;
				vec3 refraction_direction = eta * ray.Get_direction() + (eta * c1 - sqrt(c2)) * record.normal;
				Ray refraction_ray(record.hit_point - 2 * record.normal * PADDING, refraction_direction);
				// cout << refraction_counter << " refraction " << c1 << " " << c2 << " " << to_string(refraction_direction) << " " <<
				// to_string(ray.direction) << " \n";
				refraciton_color = trace_color(refraction_ray, root, eye, ambient, lights, 0, refraction_counter + 1);
				// }

#ifdef ENABLE_GLOSSY_REFRACTION
				refraction_direction = normalize(refraction_direction);
				// cout << to_string(refraction_direction) << endl;
				vec3 refract_orthonormal_u = cross(refraction_direction, -record.normal);
				vec3 refract_orthonormal_v = cross(refract_orthonormal_u, refraction_direction);
				float refract_a = 0.3f;

				int refractive_rays = 5 + 1;
				// cout << "refractive_rays " << refractive_rays << endl;
				refraciton_color /= refractive_rays;
				for (int i = 0; i < refractive_rays - 1; i++) {
					float u = -refract_a/2 + random_float() * refract_a;
					float v = -refract_a/2 + random_float() * refract_a;

					vec3 glossy_refraction = refraction_direction + u * refract_orthonormal_u + v * refract_orthonormal_v;
					glossy_refraction = normalize(glossy_refraction);
					float cosi = dot(glossy_refraction, refraction_direction);
					Ray glossy_refraction_ray(record.hit_point - 2 * record.normal * PADDING, glossy_refraction);
					// cout << to_string(glossy_refraction) << " weight " << cosi << endl;
					// cout << i << " is " << to_string(refraction_direction) << endl;
					refraciton_color += cosi / refractive_rays * trace_color(glossy_refraction_ray, root, eye, ambient, lights, maxHits-1, refraction_counter);
				}
#endif

#endif
				float reflect_coef = pm->reflectiveness();
				float refract_coef = pm->refractiveness();
				// cout << " color is " << to_string(color) << endl;
				color = (1-reflect_coef-refract_coef) * color + reflect_coef * refleciton_color + refract_coef * refraciton_color;
				// cout << " color is " << to_string(color) << endl;
			}
		}

	}
	else {
		vec3 unit_direction = glm::normalize(ray.Get_direction());
		color += cloud_texture_mapping(noiseWidth / 2 + unit_direction.x / 2 * noiseWidth, noiseHeight / 2 + unit_direction.y / 2 * noiseHeight);
	}
	return color;
}

void *A5_Render_Thread(void *data)
{
	ThreadData *d = (ThreadData *)(data);

	for (uint x = d->x_start; x < d->x_end; ++x)
	{
		for (uint y = d->y_start; y < d->y_end; ++y)
		{
			// cout << x << " " << y << endl;
			float _y = (float) y / noiseHeight;
			float _x = (float) x / noiseWidth;
			int _x_chunk = (float) x / 1024 * 8;

			vec3 direction = d->BL_corner_direction + (float)(d->h - y) * d->_v + (float)x * d->_u;

#ifdef ENABLE_DEPTH_OF_FIELD
			vec3 color;

			int random_eye_pos = 10;

			float focal_plane = 800.0f; // relative to eye position
			for (int i = 0; i < random_eye_pos; i++)
			{

				vec3 relative_move = vec3((random_float() - 0.5f) * 15, (random_float() - 0.5f) * 15, 0);
				vec3 eye_pos = d->eye + relative_move;
				float ratio = (direction.z - focal_plane) / direction.z;
				vec3 focal_direction = direction * ratio;

				focal_direction = focal_direction - relative_move;

				Ray ray = Ray(eye_pos, focal_direction);
				color += trace_color(ray, d->root, eye_pos, d->ambient, d->lights) / random_eye_pos;
			}
#else
			// cout << direction.x << " " << direction.y << " " << direction.z << endl;
			Ray ray = Ray(d->eye, direction);

			vec3 color;

			// color += trace_color(ray, root, eye, ambient, lights);
			size_t AA = 1;
#ifdef ENABLE_ANTI_ALIASING
			AA = 10;
#endif

			for (unsigned int i = 0; i < AA; ++i)
			{	// calculate color
				// anti aliasing
#ifdef ENABLE_ANTI_ALIASING
				ray.direction = direction + random_small_vec() * (d->_u + d->_v) * 0.5;
#else
				ray.direction = direction;
#endif
				color += trace_color(ray, d->root, d->eye, d->ambient, d->lights);
			}
			color /= AA;
#endif
			
			// Red:
			d->image(x, y, 0) = (double)color.r;
			// Green:
			d->image(x, y, 1) = (double)color.g;
			// Blue:
			d->image(x, y, 2) = (double)color.b;

			// to show progress bar
			(d->done)++;
		}
	}
}

void A5_Render(
	// What to render
	SceneNode *root,

	// Image to write to, set to a given width and height
	Image &image,

	// Viewing parameters
	const glm::vec3 &eye,
	const glm::vec3 &view,
	const glm::vec3 &up,
	double fovy,

	// Lighting parameters
	const glm::vec3 &ambient,
	const std::list<Light *> &lights,
	
	int t)
{
	srand(0);
	// Fill in raytracing code here...

	std::cout << "F20: Calling A5_Render(\n"
			  << "\t" << *root << "\t"
			  << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
																					 "\t"
			  << "eye:  " << glm::to_string(eye) << std::endl
			  << "\t"
			  << "view: " << glm::to_string(view) << std::endl
			  << "\t"
			  << "up:   " << glm::to_string(up) << std::endl
			  << "\t"
			  << "fovy: " << fovy << std::endl
			  << "\t"
			  << "ambient: " << glm::to_string(ambient) << std::endl
			  << "\t"
			  << "lights{" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

#ifdef ENABLE_TEXTURE_MAPPING
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++)
		{
			noise[y][x] = (double) ((rand() + t) % 32768) / 32768.0;
		}
	}

#endif

	for (const Light *light : lights)
	{
		std::cout << "\t\t" << *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std::cout << ")" << std::endl;

	vec3 _w = normalize(view);			// z-axis
	vec3 _u = normalize(cross(_w, up)); // x-axis
	vec3 _v = cross(_u, _w);			// y-axis
	float d = h / 2 / glm::tan(glm::radians(fovy / 2));
	vec3 BL_corner_direction = _w * d - _u * (float)w / 2 - _v * (float)h / 2;

	int barWidth = 50;

#ifdef ENABLE_MULTITHREAD
	size_t thread_count = 64;
#else
	size_t thread_count = 1;
#endif

	pthread_t threads[thread_count];
	size_t progress[thread_count];
	ThreadData *datas[thread_count];

	size_t x_per_thread = w / (uint)sqrt(thread_count);
	size_t y_per_thread = h / (uint)sqrt(thread_count);

	for (uint i = 0; i < (uint)sqrt(thread_count); i++)
	{
		for (uint j = 0; j < (uint)sqrt(thread_count); j++)
		{
			progress[i * (uint)sqrt(thread_count) + j] = 0;
			datas[i * (uint)sqrt(thread_count) + j] = new ThreadData{root, image, eye, ambient, lights, BL_corner_direction, _u, _v, h, w,
																	 i * x_per_thread, i == (uint)sqrt(thread_count) - 1 ? w : (i + 1) * x_per_thread,
																	 j * y_per_thread, j == (uint)sqrt(thread_count) - 1 ? h : (j + 1) * y_per_thread,
																	 progress[i * (uint)sqrt(thread_count) + j]};
			cout << "creating thread " << i * (uint)sqrt(thread_count) + j << endl;
			int ret = pthread_create(&threads[i * (uint)sqrt(thread_count) + j], NULL, A5_Render_Thread, datas[i * (uint)sqrt(thread_count) + j]);
			if (ret)
			{
				cerr << "Abort: pthread_create failed with error code: " << ret << endl;
				exit(EXIT_FAILURE);
			}
		}
	}

	size_t total = h * w;
	cout << "Progress" << endl;
	for (;;)
	{
		size_t done = 0;
		for (uint i = 0; i < thread_count; ++i)
		{
			done += progress[i];
		}
		if (done == total)
			break;

		cout << "[";
		int pos = (float)done / total * barWidth;
		for (int i = 0; i < barWidth; ++i)
		{
			if (i < pos)
				cerr << "=";
			else if (i == pos)
				cerr << ">";
			else
				cout << " ";
		}
		cout << "] " << done << "/" << total << " | " << (float)done / (float)total * 100.0 << "%\r";
		cout.flush();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	for (uint i = 0; i < thread_count; ++i)
	{
		pthread_join(threads[i], NULL);
		delete datas[i];
	}

#ifdef ENABLE_MOTION_BLUR
	int mix_times = 10; 
		// cout << BL_corner_direction.x << " " << BL_corner_direction.y << " " << BL_corner_direction.z << endl;
	for (int i = 0; i < mix_times; i++) {
		float time = random_float();
		for (uint y = 0; y < h; ++y) {
			for (uint x = 0; x < w; ++x) {
				// cout << x << " " << y << endl;
				const vec3 direction = BL_corner_direction + (float)(h - y) * _v + (float)x * _u;
				// cout << direction.x << " " << direction.y << " " << direction.z << endl;
				
				// cout << "time is " << time << endl;
				Ray ray = Ray(eye, direction, time);
				// cout << "time is " << ray.get_time() << endl;

				vec3 color;
				color.r = image(x, y, 0);
				color.g = image(x, y, 1);
				color.b = image(x, y, 2);


				vec3 motion_blur_color = motion_blur_trace_color(ray, root, eye, ambient, lights);
				if (motion_blur_color != vec3(0.0f)) {
					color = glm::mix(color, motion_blur_color, 0.1f);
				}
				// Red: 
				image(x, y, 0) = (double)color.r;
				// Green: 
				image(x, y, 1) = (double)color.g;
				// Blue: 
				image(x, y, 2) = (double)color.b;
			}
		}
	}
#endif


	cout.flush();
	cout << endl
		 << "Done" << endl;
}