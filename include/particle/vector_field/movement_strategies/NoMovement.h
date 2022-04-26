#pragma once
#include "MovementStrategy.h"
class NoMovement :
    public MovementStrategy
{
public:

    NoMovement();
    glm::vec3 get_velocity(glm::vec3 position);
    ~NoMovement();
};

