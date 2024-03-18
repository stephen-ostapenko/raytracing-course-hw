#include "primitives.h"

#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
// utils

const float EPS = 1e-12;

namespace glm {

float scalar_square(glm::vec3 &vec) {
	return glm::dot(vec, vec);
}

float scalar_square(glm::vec3 vec) {
	return glm::dot(vec, vec);
}

}

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

///////////////////////////////////////////////////////////////////////////////
// ray

Ray Ray::operator - (const glm::vec3 &vec) const {
	return { o - vec, d };
}

Ray Ray::rotate(const glm::quat &rot) const {
	return { rot * o, rot * d };
}

///////////////////////////////////////////////////////////////////////////////
// primitive

Primitive::Primitive() {}

std::optional<std::pair<float, glm::vec3>> Primitive::intersect(const Ray &ray) const {
	glm::quat conj_rotation = glm::conjugate(rotation);
	Ray transformed_ray = (ray - position).rotate(conj_rotation);

	auto t = intersection_t(transformed_ray);
    if (!t.has_value()) {
    	return {};
    }

	return std::make_pair(t.value(), color);
}

///////////////////////////////////////////////////////////////////////////////
// plane

Plane::Plane() {}

Plane::Plane(glm::vec3 _normal) {
	normal = glm::normalize(_normal);
}

std::optional<float> Plane::intersection_t(const Ray &ray) const {
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

///////////////////////////////////////////////////////////////////////////////
// ellipsoid

Ellipsoid::Ellipsoid() {}

Ellipsoid::Ellipsoid(glm::vec3 _axes) {
	axes = _axes;
}

std::optional<float> Ellipsoid::intersection_t(const Ray &ray) const {
	float a = glm::scalar_square(ray.d / axes);
	float b = 2 * glm::dot(ray.o / axes, ray.d / axes);
	float c = glm::scalar_square(ray.o / axes) - 1;

	return least_positive_root_of_square_equation(a, b, c);
}

///////////////////////////////////////////////////////////////////////////////
// box

Box::Box() {}

Box::Box(glm::vec3 _semi_axes) {
	semi_axes = _semi_axes;
}

std::optional<float> Box::intersection_t(const Ray &ray) const {
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
