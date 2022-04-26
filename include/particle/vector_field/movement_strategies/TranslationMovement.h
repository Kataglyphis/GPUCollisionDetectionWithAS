#pragma once
#include "MovementStrategy.h"
class TranslationMovement :
    public MovementStrategy
{
public:

    TranslationMovement(glm::vec3 translationDir);

    glm::vec3 get_velocity(glm::vec3 position);

    ~TranslationMovement();


private:

    glm::vec3 translationDirection;

};

