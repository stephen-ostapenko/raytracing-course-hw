#pragma once

#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Ray {
	glm::vec3 o, d;

	Ray operator - (const glm::vec3 &vec) const;

	Ray rotate(const glm::quat &rot) const;
};

struct Primitive {
	glm::vec3 position = {0.f, 0.f, 0.f};
	glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
	glm::vec3 color;
	
	virtual std::optional<float> intersection_t(const Ray &ray) const = 0;

	Primitive();

	std::optional<std::pair<float, glm::vec3>> intersect(const Ray &ray) const;
};

struct Plane : Primitive {
	glm::vec3 normal;
	
	Plane();
	
	Plane(glm::vec3 _normal);

	std::optional<float> intersection_t(const Ray &ray) const override;
};

struct Ellipsoid : Primitive {
	glm::vec3 axes;
	
	Ellipsoid();
	
	Ellipsoid(glm::vec3 _axes);
	
	std::optional<float> intersection_t(const Ray &ray) const override;
};

struct Box : Primitive {
	glm::vec3 semi_axes;

	Box();
	
	Box(glm::vec3 _semi_axes);

	std::optional<float> intersection_t(const Ray &ray) const override;
};
