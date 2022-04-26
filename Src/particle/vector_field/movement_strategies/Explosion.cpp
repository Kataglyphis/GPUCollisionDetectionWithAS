#include "Explosion.h"

Explosion::Explosion()
{

	influence_angle_theta	= 360;
	influence_angle_rho		= 360;
}

Explosion::Explosion(float influence_angle_theta, float influence_angle_rho) {

	if (influence_angle_theta < 0 || influence_angle_theta > 180 ||
		influence_angle_rho < -180 || influence_angle_rho > 180 ) {
		throw std::runtime_error("Wrong angles when creating explosion!");
	}

	this->influence_angle_theta		= influence_angle_theta;
	this->influence_angle_rho		= influence_angle_rho;

}

glm::vec3 Explosion::get_velocity(glm::vec3 position)
{
	std::mt19937 rng(dev());
	std::uniform_real_distribution<> dist_x(-1.0, 1.0);
	std::uniform_real_distribution<> dist_y(-1.0, 1.0);
	std::uniform_real_distribution<> dist_z(-1.0, 1.0);

	return glm::vec3(dist_x(rng), dist_y(rng), dist_z(rng));
}

Explosion::~Explosion()
{
}
