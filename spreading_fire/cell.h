#ifndef CELL_H_
#define CELL_H_

#include <cstdint>
#include <cstdlib>
#include <iosfwd>

struct TreeT {
  int DurationCut;
  int DurationFire;
  int Value;
};

struct HouseT {
  int DurationCut;
  int DurationFire;
  int Value;
};

extern TreeT Tree;
extern HouseT House;
/**
 * Store the Type and Status of a cell by combining both enums
 * into one unsigned 8 bit integers with a bitwise OR operation.
 */
class Cell {
public:
  enum class Type : uint8_t {
  Safe = 1,
  Tree = 2,
  House = 4
};
  enum class Status : uint8_t {
    NoFire = 1,
    OnFire = 2,
    Burnt = 4,
    Cut = 8,
    Cutting = 16
  };

  Cell(Type type, Status status, const size_t n);

  /** Return the Type part of the cell's state. */
  Status status() const { return m_status; }

  /** Return the Status part of the cell's state. */
  Type type() const { return m_type; }

  /** Change the type to safe (cell is burned or cut) */
  void start_cutting();

  /** Set the cell on fire */
  void set_on_fire();

  /** Set the cell to type safe */
  void set_safe();

  /** The time before the cell is burnt, or -1 if no fire. */
  int fire_countdown() const { return m_fire_countdown; }

  /** The time before the cell is cut, or -1 if not cutting. */
  int cutting_countdown() const { return m_cutting_countdown; }

  /**
   * Decrement any necessary countdown, set type and status if burnt
   *  or cut.
   * In case one of the two countdowns gets to 0, return true.
   */
  bool update();

  /** Return the value, or 0 if the cell is consumed. */
  int value() const;


    size_t index() const { return m_index; }

private:
  /** Both the type and status are encoded in an 8-bit integer */
  Type m_type;

  /** Whether the cell is on fire or not */
  Status m_status;

  /** The cell's index */
  const size_t m_index;

  /** The cell's countdown until it gets Burnt status */
  int m_fire_countdown{-1};

  /** The cell's countdown until it gets Cut status */
  int m_cutting_countdown{-1};
};

extern std::ostream& operator<<(std::ostream& out, Cell::Type type);
extern std::ostream& operator<<(std::ostream& out, Cell::Status status);


#endif // CELL_H_
