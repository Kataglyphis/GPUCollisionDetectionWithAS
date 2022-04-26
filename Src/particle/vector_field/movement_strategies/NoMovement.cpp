#include "NoMovement.h"

NoMovement::NoMovement()
{
}

glm::vec3 NoMovement::get_velocity(glm::vec3 position)
{
	return glm::vec3(0.0f);
}

NoMovement::~NoMovement()
{
}
