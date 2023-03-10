#include "entities.h"

#include <iostream>

namespace EscapeTheCat {

const double CRITICAL_ARC_DISTANCE = POOL_RADIUS * 2.0 * std::asin(CRITICAL_DISTANCE / POOL_RADIUS);

Entity::Entity(double speed)
      : speed{ speed }
      , pos{ 0.0, 0.0 }
  {
  }

void Entity::update() {
  updated = true;
}

std::istream& Entity::input(std::istream& is) {
  int X, Y;
  is >> X >> Y;
  pos.real(X);
  pos.imag(Y);
  update();
  return is;
}

Cat::Cat(double speed)
      : Entity{ speed }
  {
  }

void Cat::update() {
  opposite = -pos;

  auto* as_entity = dynamic_cast<Entity*>(this);
  as_entity->update();
}

Mouse::Mouse()
    : Entity{ Mouse::SPEED }
{
}

void Mouse::update()  {
  if (pos.imag() != 0 || pos.real() != 0)
    closest_on_boundary = POOL_RADIUS * pos / abs(pos);
  else
    closest_on_boundary = { 0.0, 0.0 };
}

} // namespace EscapeTheCat
