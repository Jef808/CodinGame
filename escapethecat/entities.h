#ifndef ENTITIES_H_
#define ENTITIES_H_


#include <cmath>
#include <complex>
#include <iosfwd>

namespace EscapeTheCat {

constexpr double POOL_RADIUS = 500.0;
constexpr double CRITICAL_DISTANCE = 80.0;

/* Arc-length corresponding to the boundary length of a chord of length 80.0. */
extern const double CRITICAL_ARC_DISTANCE;

struct Entity {
  Entity(double speed);

  virtual ~Entity() = default;

  std::istream& input(std::istream& is);

  virtual void update();

  const double speed;
  std::complex<double> pos;
  bool updated { false };
};

inline std::istream& operator>>(std::istream& is, Entity& entity) { return entity.input(is); }

struct Cat : public Entity {
  Cat(double speed);

  void update() override;

  std::complex<double> opposite;
};

struct Mouse : public Entity {
  static constexpr double SPEED = 10.0;
  Mouse();

  void update() override;

  std::complex<double> closest_on_boundary;
};

} // namespace EscapeTheCat


#endif // ENTITIES_H_
