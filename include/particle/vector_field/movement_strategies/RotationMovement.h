#include <functional>
#include <MovementStrategy.h>
#include <math.h>
#include <random>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.14159265

class RotationMovement : public MovementStrategy {
public:
	RotationMovement(int rotation_axis_selection);
	glm::vec3 get_velocity(glm::vec3 position);
	~RotationMovement();

	int rotation_axis_selection;
};