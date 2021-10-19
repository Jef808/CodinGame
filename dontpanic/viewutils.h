#ifndef __VIEWUTILS_H_
#define __VIEWUTILS_H_

#include <algorithm>
#include <string_view>
#include <type_traits>

#include <fmt/core.h>
#include <fmt/format.h>

#include "dp.h"

namespace {

template<typename Enum>
std::underlying_type_t<Enum>
constexpr inline to_int(Enum e) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(e);
}

}  // namespace

namespace dp {

enum class Cell {
    Empty=0,     // empty
    Elevator=1,  // elevator
    Wall=2,      // wall
    EntryI=3,    // entry
    EntryN=4,    // entry
    ExitO=5,     // exit
    ExitU=6,     // exit
    Clone=7,     // clone
    Nb=8
};

struct Square {};

constexpr int NbCells = to_int(Cell::Nb);

constexpr std::array<const char, NbCells> literals =
{
    's',         // empty
    'e',         // elevator
    'w',         // wall
    'i',         // entry
    'n',         // entry
    'o',         // exit
    'u',         // exit
    'c'          // clone
};

constexpr std::array<const char8_t*, NbCells> Ucodes =
{
    u8"\u2500",  // empty
    u8"\u2191",  // elevator
    u8"\u2503",  // wall
    u8"I",       // entry
    u8"N",       // entry
    u8"O",       // exit
    u8"U",       // exit
    u8"C"        // clone
};

struct Formatter {
    const char literal;
    const Cell type;
    const char8_t* unicode;
};

constexpr std::array<Formatter, NbCells> Encodings =
{
    Formatter{literals[0], Cell(0), Ucodes[0]},  // empty
    {literals[1], Cell(1), Ucodes[1]},           // elevator
    {literals[2], Cell(2), Ucodes[2]},           // wall
    {literals[3], Cell(3), Ucodes[3]},           // entry
    {literals[4], Cell(4), Ucodes[4]},           // entry
    {literals[5], Cell(5), Ucodes[5]},           // exit
    {literals[6], Cell(6), Ucodes[6]},           // exit
    {literals[7], Cell(7), Ucodes[7]}            // clone
};

}  // namespace dp

constexpr const char8_t* get_unicode(const dp::Square& dp, char presentation) noexcept
{
    return std::find_if(dp::Encodings.begin(), dp::Encodings.end(), [p=presentation](const auto& enc){
        return enc.literal == p;
    })
        ->unicode;
}

template<>
struct fmt::formatter<dp::Square>
    : formatter<std::string_view>
{
    char presentation = 's';

    constexpr auto parse(format_parse_context& ctx) ->decltype(ctx.begin())
    {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it=='s' || *it=='e' || *it=='w' ||
                          *it=='i' || *it=='n' || *it=='c' ||
                          *it=='o' || *it=='u'))

            presentation = *it++;

        if (it != end && *it != '}')
            throw format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    auto format(const dp::Square& sq, FormatContext& ctx) -> decltype(ctx.out())
    {
        std::string_view sv((const char*)get_unicode(sq, presentation));
        return formatter<std::string_view>::format(sv, ctx);
    }
};

#endif // VIEWUTILS_H_
