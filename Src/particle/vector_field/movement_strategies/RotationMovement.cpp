#include <RotationMovement.h>

RotationMovement::RotationMovement(int rotation_axis_selection)
{
	this->rotation_axis_selection = rotation_axis_selection;
}

glm::vec3 RotationMovement::get_velocity(glm::vec3 position)
{
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_real_distribution<> dist(0.0, 100.0);
	glm::vec3 eps = { dist(rng), dist(rng), dist(rng) };


	switch (rotation_axis_selection) {
	case 0:
		return glm::vec3(0.0, -position.z, position.y);
		break;
	case 1:
		return glm::vec3(-position.z, 0.0, position.x);
		break;
	case 2:
		return glm::vec3(-position.y, position.x,0.0);
	default:
		return glm::vec3(-position.y, position.x, 0.0);
	}
}



RotationMovement::~RotationMovement()
{
}
