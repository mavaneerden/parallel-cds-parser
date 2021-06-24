/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Contains the implementation of the descriptor class.
 */

#include "descriptor.hpp"
#include "../utilities/print.hpp"
#include "../utilities/hash_custom.hpp"

/**
 * @brief Default constructor.
 */
Descriptor::Descriptor() { }

/**
 * @brief Copy constructor.
 *
 * @param d Descriptor to copy.
 */
Descriptor::Descriptor(const Descriptor& d)
{
    lhs = d.lhs;
    rhs = d.rhs;
    dot_position = d.dot_position;
    left_extent = d.left_extent;
    right_extent = d.right_extent;
    force_process = d.force_process;
}

/**
 * @brief Creates descriptor from parameters.
 *
 * @param k Left hand side of the grammar slot.
 * @param v Right hand side fo the grammar slot.
 * @param d Position of the dot in the grammar slot.
 * @param l Left extent.
 * @param r Right extent or pivot.
 */
Descriptor::Descriptor(
    std::string k,
    std::vector<std::string> v,
    unsigned int d,
    unsigned int l,
    unsigned int r
// ) : lhs(k), rhs(v), dot_position(d), left_extent(l), right_extent(r) { }
) : lhs(k), rhs(v), dot_position(d), left_extent(l), right_extent(r), force_process(false) { }

/**
 * @brief Creates descriptor from parameters.
 *
 * @param k Left hand side of the grammar slot.
 * @param v Right hand side fo the grammar slot.
 * @param d Position of the dot in the grammar slot.
 * @param l Left extent.
 * @param r Right extent or pivot.
 * @param f Whether to force descriptor to be processed.
 */
Descriptor::Descriptor(
    std::string k,
    std::vector<std::string> v,
    unsigned int d,
    unsigned int l,
    unsigned int r,
    bool f
) : lhs(k), rhs(v), dot_position(d), left_extent(l), right_extent(r), force_process(f) { }

/**
 * @return True if the production rule is fully processed, false otherwise.
 */
bool Descriptor::is_completed()
{
    return dot_position == rhs.size();
}

/**
 * @return True if the right hand side of the production rule is empty.
 */
bool Descriptor::is_empty()
{
    return rhs.size() == 0;
}

/**
 * @return The next symbol to be processed.
 */
std::string Descriptor::get_next_symbol()
{
    return rhs.at(dot_position);
}

/**
 * @brief Advance the dot position by 1.
 */
void Descriptor::advance()
{
    dot_position++;
}

/**
 * @brief Copy this descriptor and advance the copy.
 *
 * @return Advanced copy of this descriptor.
 */
Descriptor Descriptor::copy_and_advance()
{
    Descriptor copy = Descriptor(*this);
    copy.advance();

    return copy;
}

/**
 * @brief Copy this descriptor and set force_process to true.
 *
 * @return Advanced copy of this descriptor.
 */
Descriptor Descriptor::copy_and_force()
{
    Descriptor copy = Descriptor(*this);
    copy.force_process = true;

    return copy;
}

/**
 * @return Hash of this object.
 */
size_t Descriptor::hash() const
{
    size_t seed = 17;
    hash_custom::hash_combine(seed, lhs);
    hash_custom::hash_combine(seed, rhs);
    hash_custom::hash_combine(seed, dot_position);
    hash_custom::hash_combine(seed, left_extent);
    hash_custom::hash_combine(seed, right_extent);

    return seed;
}

Descriptor& Descriptor::operator=(const Descriptor& d)
{
    if (this == &d)
    {
        return *this;
    }

    lhs = d.lhs;
    rhs = d.rhs;
    dot_position = d.dot_position;
    left_extent = d.left_extent;
    right_extent = d.right_extent;
    force_process = d.force_process;

    return *this;
}

bool operator==(const Descriptor& first, const Descriptor& second)
{
    bool result = first.lhs == second.lhs
                  && first.rhs.size() == second.rhs.size()
                  && first.dot_position == second.dot_position
                  && first.left_extent == second.left_extent
                  && first.right_extent == second.right_extent;

    if (result)
    {
        for (size_t i = 0; i < first.rhs.size(); i++)
        {
            result = result && first.rhs[i] == second.rhs[i];
        }
    }

    return result;
}

std::ostream& operator<<(std::ostream& out, const Descriptor& descriptor)
{
    out << std::string("[")
        << production_rule_to_string(std::make_pair(descriptor.lhs, descriptor.rhs), descriptor.dot_position)
        << std::string(", ")
        << std::to_string(descriptor.left_extent)
        << std::string(", ")
        << std::to_string(descriptor.right_extent)
        << std::string("]")
        << std::string(" ") << std::to_string(descriptor.force_process);

    return out;
}