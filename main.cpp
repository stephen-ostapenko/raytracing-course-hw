#include <bits/stdc++.h>
using namespace std;

using std::size_t;
using std::uint8_t;

//#include <glm/vec3.hpp> // glm::vec3
//#include <glm/vec4.hpp> // glm::vec4
//#include <glm/mat4x4.hpp> // glm::mat4
//#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
//#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
//#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

const float EPS = 1e-12;

namespace glm {

float scalar_square(glm::vec3 &vec) {
	return glm::dot(vec, vec);
}

float scalar_square(glm::vec3 vec) {
	return glm::dot(vec, vec);
}

}

struct Ray {
	glm::vec3 o, d;

	Ray operator - (const glm::vec3 &vec) const {
		return { o - vec, d };
	}

	Ray rotate(const glm::quat &rot) const {
		return { rot * o, rot * d };
	}
};

std::optional<float> least_positive_from_two(float a, float b) {
	if (a - b > 0) {
		std::swap(a, b);
	}

	if (a > 0) {
		return a;
	}
	
	if (b > 0) {
		return b;
	}

	return {};
}

std::optional<float> least_positive_root_of_square_equation(float a, float b, float c) {
	float d = b * b - 4 * a * c;
	if (d < 0) {
		return {};
	}

	float sd = sqrt(d);
	float x1 = (-b + sd) / a / 2;
	float x2 = (-b - sd) / a / 2;

	return least_positive_from_two(x1, x2);
}

struct Primitive {
	glm::vec3 position = {0.f, 0.f, 0.f};
	glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
	glm::vec3 color;
	
	virtual std::optional<float> intersection_t(const Ray &ray) const = 0;

	Primitive() {}

	std::optional<std::pair<float, glm::vec3>> intersect(const Ray &ray) const {
		glm::quat conj_rotation = glm::conjugate(rotation);
		Ray transformed_ray = (ray - position).rotate(conj_rotation);
    	
    	auto t = intersection_t(transformed_ray);
	    if (!t.has_value()) {
	    	return {};
	    }
		
		return std::make_pair(t.value(), color);
	}
};

struct Plane : Primitive {
	glm::vec3 normal;
	
	Plane() {}
	
	Plane(glm::vec3 _normal) {
		normal = glm::normalize(_normal);
	}

	std::optional<float> intersection_t(const Ray &ray) const override {
		float d_normal = glm::dot(ray.d, normal);
		if (std::abs(d_normal) < EPS) {
			return {};
		}
		
		float t = -glm::dot(ray.o, normal) / d_normal;
		if (t <= 0) {
			return {};
		}
		
		return t;
	}
};

struct Ellipsoid : Primitive {
	glm::vec3 axes;
	
	Ellipsoid() {}
	
	Ellipsoid(glm::vec3 _axes) {
		axes = _axes;
	}
	
	std::optional<float> intersection_t(const Ray &ray) const override {
		float a = glm::scalar_square(ray.d / axes);
		float b = 2 * glm::dot(ray.o / axes, ray.d / axes);
		float c = glm::scalar_square(ray.o / axes) - 1;
		
		return least_positive_root_of_square_equation(a, b, c);
	}
};

struct Box : Primitive {
	glm::vec3 semi_axes;

	Box() {}
	
	Box(glm::vec3 _semi_axes) {
		semi_axes = _semi_axes;
	}

	std::optional<float> intersection_t(const Ray &ray) const override {
		glm::vec3 ts1 = (semi_axes - ray.o) / ray.d;
		glm::vec3 ts2 = (-semi_axes - ray.o) / ray.d;
		
		float t1x = std::min(ts1.x, ts2.x), t2x = std::max(ts1.x, ts2.x);
		float t1y = std::min(ts1.y, ts2.y), t2y = std::max(ts1.y, ts2.y);
		float t1z = std::min(ts1.z, ts2.z), t2z = std::max(ts1.z, ts2.z);
		
		float t1 = std::max({ t1x, t1y, t1z });
		float t2 = std::min({ t2x, t2y, t2z });
		
		if (t1 > t2) {
		    return {};
		}

		return least_positive_from_two(t1, t2);
	}
};

struct Scene {
	size_t width, height;
	glm::vec3 bg_color;
	
	glm::vec3 camera_position, camera_right, camera_up, camera_forward;
	glm::vec2 tan_fov;

	std::vector<std::variant<Plane, Ellipsoid, Box>> primitives;

	Ray generate_ray_to_pixel(size_t x, size_t y) const {
		float xc = tan_fov.x * (2 * (x + 0.5) / width - 1);
		float yc = tan_fov.y * (2 * (y + 0.5) / height - 1);

		return { camera_position, xc * camera_right - yc * camera_up + camera_forward };
	}

	glm::vec3 get_pixel_color(size_t x, size_t y) const {
		std::optional<std::pair<float, glm::vec3>> ans;
		
		for (const auto &pr : primitives) {
			Ray ray = generate_ray_to_pixel(x, y);
			std::optional<std::pair<float, glm::vec3>> intersection;
			
			switch (pr.index()) {
			case 0:
				intersection = std::get<0>(pr).intersect(ray);
				break;

			case 1:
				intersection = std::get<1>(pr).intersect(ray);
				break;

			case 2:
				intersection = std::get<2>(pr).intersect(ray);
				break;

			default:
				assert(false);
			}
			
			if (!intersection.has_value()) {
				continue;
			}

			if (!ans.has_value() || ans.value().first > intersection.value().first) {
				ans = intersection;
			}
		}

		if (!ans.has_value()) {
			return bg_color;
		}

		return ans.value().second;
	}
};

Scene read_scene(std::istream &in) {
	std::unordered_map <string, int> command_id;
	
	static const int UNKNOWN_COMMAND = 0;
	static const int FIN             = -1;
	
	static const int DIMENSIONS      = 1;
	static const int BG_COLOR        = 2;
	static const int CAMERA_POSITION = 3;
	static const int CAMERA_RIGHT    = 4;
	static const int CAMERA_UP       = 5;
	static const int CAMERA_FORWARD  = 6;
	static const int CAMERA_FOV_X    = 7;

	static const int NEW_PRIMITIVE   = 8;
	static const int POSITION        = 9;
	static const int ROTATION        = 10;
	static const int COLOR           = 11;
	static const int PLANE           = 12;
	static const int ELLIPSOID       = 13;
	static const int BOX             = 14;
	
	command_id["FIN"]             = FIN;

	command_id["DIMENSIONS"]      = DIMENSIONS;
	command_id["BG_COLOR"]        = BG_COLOR;
	command_id["CAMERA_POSITION"] = CAMERA_POSITION;
	command_id["CAMERA_RIGHT"]    = CAMERA_RIGHT;
	command_id["CAMERA_UP"]       = CAMERA_UP;
	command_id["CAMERA_FORWARD"]  = CAMERA_FORWARD;
	command_id["CAMERA_FOV_X"]    = CAMERA_FOV_X;

	command_id["NEW_PRIMITIVE"]   = NEW_PRIMITIVE;
	command_id["POSITION"]        = POSITION;
	command_id["ROTATION"]        = ROTATION;
	command_id["COLOR"]           = COLOR;
	command_id["PLANE"]           = PLANE;
	command_id["ELLIPSOID"]       = ELLIPSOID;
	command_id["BOX"]             = BOX;

	Scene scene;

	enum { none, plane, ellipsoid, box } cur_primitive_type = none;
	std::unique_ptr<Primitive> cur_primitive;
	glm::vec3 cur_position;
	glm::quat cur_rotation;
	glm::vec3 cur_color;

	auto emit_primitive = [&]() {
		switch (cur_primitive_type) {
		case plane: {
			Plane *ptr = static_cast<Plane*>(cur_primitive.get());
			scene.primitives.push_back(*ptr);
			break;
		}

		case ellipsoid: {
			Ellipsoid *ptr = static_cast<Ellipsoid*>(cur_primitive.get());
			scene.primitives.push_back(*ptr);
			break;
		}

		case box: {
			Box *ptr = static_cast<Box*>(cur_primitive.get());
			scene.primitives.push_back(*ptr);
			break;
		}
		
		case none:
			assert(false);
		}

		cur_primitive_type = none;
		cur_primitive.reset();
	};

	string s;
	bool finish = false;
	while (!finish && in >> s) {
		switch (command_id[s]) {
		case UNKNOWN_COMMAND:
			std::cerr << "w: unknown command " << s << std::endl;
			break;

		case FIN:
			finish = true;
			break;

		case DIMENSIONS:
			in >> scene.width >> scene.height;
			break;

		case BG_COLOR: {
			glm::vec3& v = scene.bg_color;
			in >> v.x >> v.y >> v.z;
			break;
		}	

		case CAMERA_POSITION: {
			glm::vec3& v = scene.camera_position;
			in >> v.x >> v.y >> v.z;
			break;
		}
		
		case CAMERA_RIGHT: {
			glm::vec3& v = scene.camera_right;
			in >> v.x >> v.y >> v.z;
			break;
		}
		
		case CAMERA_UP: {
			glm::vec3& v = scene.camera_up;
			in >> v.x >> v.y >> v.z;
			break;
		}
		
		case CAMERA_FORWARD: {
			glm::vec3& v = scene.camera_forward;
			in >> v.x >> v.y >> v.z;
			break;
		}
		
		case CAMERA_FOV_X:
			float camera_fov_x;
			in >> camera_fov_x;

			scene.tan_fov.x = tan(camera_fov_x / 2);
			scene.tan_fov.y = scene.tan_fov.x * scene.height / scene.width;

			break;

		case NEW_PRIMITIVE:
			if (cur_primitive) {
				emit_primitive();
			}

			cur_position = glm::vec3(0.f, 0.f, 0.f);
			cur_rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
			cur_color = glm::vec3(0.f, 0.f, 0.f);
			
			break;

		case POSITION:
			in >> cur_position.x >> cur_position.y >> cur_position.z;
			if (cur_primitive) {
				cur_primitive->position = cur_position;
			}
			
			break;

		case ROTATION:
			in >> cur_rotation.x >> cur_rotation.y >> cur_rotation.z >> cur_rotation.w;
			if (cur_primitive) {
				cur_primitive->rotation = cur_rotation;
			}
			
			break;

		case COLOR:
			in >> cur_color.x >> cur_color.y >> cur_color.z;
			if (cur_primitive) {
				cur_primitive->color = cur_color;
			}
			
			break;

		case PLANE: {
			glm::vec3 normal;
			in >> normal.x >> normal.y >> normal.z;
			
			cur_primitive_type = plane;
			cur_primitive = std::make_unique<Plane>(normal);

			cur_primitive->position = cur_position;
			cur_primitive->rotation = cur_rotation;
			cur_primitive->color = cur_color;
			
			break;
		}

		case ELLIPSOID: {
			glm::vec3 axes;
			in >> axes.x >> axes.y >> axes.z;
			
			cur_primitive_type = ellipsoid;
			cur_primitive = std::make_unique<Ellipsoid>(axes);

			cur_primitive->position = cur_position;
			cur_primitive->rotation = cur_rotation;
			cur_primitive->color = cur_color;

			break;
		}

		case BOX: {
			glm::vec3 semi_axes;
			in >> semi_axes.x >> semi_axes.y >> semi_axes.z;
			
			cur_primitive_type = box;
			cur_primitive = std::make_unique<Box>(semi_axes);

			cur_primitive->position = cur_position;
			cur_primitive->rotation = cur_rotation;
			cur_primitive->color = cur_color;
			
			break;
		}

		default:
			assert(false);
		}
	}

	if (cur_primitive) {
		emit_primitive();
	}

	return scene;
}

struct Image {
	size_t width, height;
	glm::vec3 **data, *_raw_data;

	Image(size_t _width, size_t _height) {
		width = _width; height = _height;

		_raw_data = new glm::vec3[width * height];
		data = new glm::vec3*[height];

		for (size_t i = 0; i < height; i++) {
			data[i] = _raw_data + i * width;
		}
	}

	~Image() {
		delete[] _raw_data;
		delete[] data;
	}
};

Image render_scene(const Scene &scene) {
	Image result(scene.width, scene.height);
	
	for (size_t i = 0; i < scene.height; i++) {
		for (size_t j = 0; j < scene.width; j++) {
			result.data[i][j] = scene.get_pixel_color(j, i);
		}
	}

	return result;
}

void write_image(const Image &img, std::ostream &out) {
	vector<uint8_t> img_data; img_data.reserve(img.width * img.height * 3);

	for (size_t i = 0; i < img.height; i++) {
		for (size_t j = 0; j < img.width; j++) {
			//img_data.push_back((byte)(int)round(img.data[i][j][0] * 255));
			//img_data.push_back((byte)(int)round(img.data[i][j][1] * 255));
			//img_data.push_back((byte)(int)round(img.data[i][j][2] * 255));

			//img_data.push_back((uint8_t)round(0.5 * 255));
			//img_data.push_back((uint8_t)round(0.1 * 255));
			//img_data.push_back((uint8_t)round(0.3 * 255));

			img_data.push_back((uint8_t)round(img.data[i][j].x * 255));
			img_data.push_back((uint8_t)round(img.data[i][j].y * 255));
			img_data.push_back((uint8_t)round(img.data[i][j].z * 255));
		}
	}

	out << "P6" << std::endl;
	out << img.width << " " << img.height << std::endl;
	out << 255 << std::endl;
	out.write(reinterpret_cast<const char*>(img_data.data()), img_data.size() * sizeof(uint8_t));
	out.flush();
}

int main(int argc, char **argv) {
	std::ifstream in(argv[1]);
	std::ofstream out(argv[2]);
	
	Scene scene = read_scene(in);
	Image result = render_scene(scene);
	write_image(result, out);

	return 0;
}