#include "TranslationMovement.h"

TranslationMovement::TranslationMovement(glm::vec3 translationDir)
{
	this->translationDirection = translationDir;
}

glm::vec3 TranslationMovement::get_velocity(glm::vec3 position)
{
	return translationDirection;
}



TranslationMovement::~TranslationMovement()
{
}
