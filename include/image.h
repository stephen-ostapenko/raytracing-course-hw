#pragma once

#include <cstddef>

#include <glm/glm.hpp>

#include "scene.h"

using std::size_t;

struct Image {
	size_t width, height;
	glm::vec3 **data, *_raw_data;

	Image(size_t _width, size_t _height);

	~Image();
};

Image render_scene(const Scene &scene);

void write_image(const Image &img, std::ostream &out);
