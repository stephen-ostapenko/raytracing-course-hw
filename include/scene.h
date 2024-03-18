#pragma once

#include <cstddef>
#include <iosfwd>
#include <optional>
#include <variant>
#include <vector>

#include <glm/glm.hpp>

#include "primitives.h"

using std::size_t;

struct Scene {
	size_t width, height;
	glm::vec3 bg_color;

	glm::vec3 camera_position, camera_right, camera_up, camera_forward;
	glm::vec2 tan_fov;

	std::vector<std::variant<Plane, Ellipsoid, Box>> primitives;

	Ray generate_ray_to_pixel(size_t x, size_t y) const;

	glm::vec3 get_pixel_color(size_t x, size_t y) const;
};

Scene read_scene(std::istream &in);
