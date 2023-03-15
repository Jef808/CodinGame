#ifndef CATSANDMOUSE_H_
#define CATSANDMOUSE_H_

#include <complex>
#include <iosfwd>

namespace EscapeTheCat {

constexpr double POOL_RADIUS = 500.0;
constexpr double MOUSE_SPEED = 10.0;

double distance_after(const std::complex<double>& target);

class CatAndMouse {
  public:
    explicit CatAndMouse(double cat_speed);

    void input(std::istream&);

    /**
     * \brief Compute a number between \f$0\f$ and \f$1\f$ indicating the
     * distance from the cat to the mouse's closest point on the boundary.
     *
     * More precisely, with \f$C, M\f$ denoting the cat's (resp. mouse's)
     * position, \f$\partial M\f$ the point closest to the mouse on the
     * boundary, and \f$d_{\partial}(\cdot, \cdot)\f$ the distance restricted to
     * the boundary of the pool, compute \f[ \frac{1}{\pi R}d_{\partial}(C,
     * \partial M) + \frac{1}{R}d(0, M) \f] where \f$R\f$ is the radius of the
     * pool.
     *
     * \return A number between 0 and 1.
     */
    double score() const;

    /**
     * Update the mouse's and cat's position after mouse moves towards \p
     * target.
     */
    void step(const std::complex<double>& target);

    /**
     * Revert to the previous state.
     *
     * After being called once, this will have no effect before a call to
     * step(const std::complex<double>&) const.
     */
    void undo();

    [[nodiscard]] const std::complex<double>& mouse() const { return m_mouse; }

    [[nodiscard]] const std::complex<double>& cat() const { return m_cat; }

    [[nodiscard]] const double cat_speed() const { return m_cat_speed; }

    double distance_after(const std::complex<double>& target);

    friend std::istream& operator>>(std::istream& is, CatAndMouse& cm) {
        cm.input(is);
        return is;
    }

  private:
    const double m_cat_speed{0.0};
    std::complex<double> m_cat{POOL_RADIUS, 0.0};
    std::complex<double> m_cat_prev{POOL_RADIUS, 0.0};
    std::complex<double> m_mouse{0.0, 0.0};
    std::complex<double> m_mouse_prev{0.0, 0.0};
    mutable std::complex<double> m_mouse_boundary{0.0 - POOL_RADIUS};
};

extern CatAndMouse initialize(std::istream&);

} // namespace EscapeTheCat

#endif // CATSANDMOUSE_H_
