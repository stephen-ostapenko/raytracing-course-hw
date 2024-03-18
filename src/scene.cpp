#include "scene.h"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

using std::size_t;

Ray Scene::generate_ray_to_pixel(size_t x, size_t y) const {
	float xc = tan_fov.x * (2 * (x + 0.5) / width - 1);
	float yc = tan_fov.y * (2 * (y + 0.5) / height - 1);

	return { camera_position, xc * camera_right - yc * camera_up + camera_forward };
}

glm::vec3 Scene::get_pixel_color(size_t x, size_t y) const {
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

Scene read_scene(std::istream &in) {
	std::unordered_map <std::string, int> command_id;
	
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

	std::string s;
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
