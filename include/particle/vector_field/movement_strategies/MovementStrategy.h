#pragma once
#include <functional>
#include <glm/glm.hpp>

// serves as C++ interface
class MovementStrategy
{
public:
	virtual ~MovementStrategy() {}
	virtual glm::vec3 get_velocity(glm::vec3 position) = 0;
};

