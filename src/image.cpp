#include "image.h"

#include <iostream>
#include <vector>

using std::size_t;
using std::uint8_t;

Image::Image(size_t _width, size_t _height) {
	width = _width; height = _height;

	_raw_data = new glm::vec3[width * height];
	data = new glm::vec3*[height];

	for (size_t i = 0; i < height; i++) {
		data[i] = _raw_data + i * width;
	}
}

Image::~Image() {
	delete[] _raw_data;
	delete[] data;
}

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
	std::vector<uint8_t> img_data; img_data.reserve(img.width * img.height * 3);

	for (size_t i = 0; i < img.height; i++) {
		for (size_t j = 0; j < img.width; j++) {
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
