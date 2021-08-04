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
static const int noiseWidth = 1024;
static const int noiseHeight = 1024;
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
	// HSL = (169, 255, L)
	return HSLtoRGB(169.0, 255.0, L);
}

vec3 trace_color(
	Ray &ray,
	SceneNode *root,
	const glm::vec3 &eye,
	const glm::vec3 &ambient,
	const std::list<Light *> &lights,
	const int maxHits = 5)
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
			vec3 reflection_direction = ray.Get_direction() - 2 * record.normal * dot(ray.Get_direction(), record.normal);
			Ray reflection_ray(record.hit_point, reflection_direction);
			float reflect_coef = 0.2;
			color = glm::mix(color, trace_color(reflection_ray, root, eye, ambient, lights, maxHits - 1), reflect_coef);
		}
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
			float _y = (float) y / 1024;
			float _x = (float) x / 1024;
			int _x_chunk = (float) x / 1024 * 8;

			vec3 direction = d->BL_corner_direction + (float)(d->h - y) * d->_v + (float)x * d->_u;

#ifdef ENABLE_DEPTH_OF_FIELD
			vec3 color;

			int random_eye_pos = 10 + 1;

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
			
			if (color == vec3(0, 0, 0)) {
				color = cloud_texture_mapping(x, y);
			}

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
	const std::list<Light *> &lights)
{

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
			noise[y][x] = (double) (rand() % 32768) / 32768.0;
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
	cout.flush();
	cout << endl
		 << "Done" << endl;
}