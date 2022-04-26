#pragma once
#include "MovementStrategy.h"
#include <random>
#include <stdexcept>

class Explosion :
    public MovementStrategy
{
public:

    Explosion();
    Explosion(float influence_angle_theta, float influence_angle_rho);
    glm::vec3 get_velocity(glm::vec3 position);
    ~Explosion();

private:

    float influence_angle_theta, influence_angle_rho;

    std::random_device dev;

};

