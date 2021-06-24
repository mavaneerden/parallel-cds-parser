/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Contains the implementation of the EPN class.
 */

#include "epn.hpp"
#include "../utilities/print.hpp"
#include "../utilities/hash_custom.hpp"

/**
 * @brief Crates an EPN from a descriptor without a provided pivot.
 * Used for empty production rules.
 *
 * @param d Descriptor.
 */
EPN::EPN(const Descriptor& d)
{
    lhs = d.lhs;
    rhs = d.rhs;
    dot_position = d.dot_position;
    left_extent = d.left_extent;
    pivot = d.right_extent;
    right_extent = d.right_extent;
}

/**
 * @brief Crates an EPN from a descriptor with a provided pivot.
 *
 * @param d Descriptor.
 * @param p Pivot.
 */
EPN::EPN(const Descriptor& d, unsigned int p)
{
    lhs = d.lhs;
    rhs = d.rhs;
    dot_position = d.dot_position;
    left_extent = d.left_extent;
    pivot = p;
    right_extent = d.right_extent;
}

/**
 * @brief Creates descriptor from parameters.
 *
 * @param k Left hand side of the grammar slot.
 * @param v Right hand side fo the grammar slot.
 * @param d Position of the dot in the grammar slot.
 * @param l Left extent.
 * @param p Pivot.
 * @param r Right extent.
 */
EPN::EPN(
    std::string k,
    std::vector<std::string> v,
    unsigned int d,
    unsigned int l,
    unsigned int p,
    unsigned int r
) : lhs(k), rhs(v), dot_position(d), left_extent(l), pivot(p), right_extent(r) { }

/**
 * @return Hash of this object.
 */
size_t EPN::hash() const
{
    size_t seed = 17;
    hash_custom::hash_combine(seed, lhs);
    hash_custom::hash_combine(seed, rhs);
    hash_custom::hash_combine(seed, dot_position);
    hash_custom::hash_combine(seed, left_extent);
    hash_custom::hash_combine(seed, pivot);
    hash_custom::hash_combine(seed, right_extent);

    return seed;
}

bool operator==(const EPN& first, const EPN& second)
{
    bool result = first.lhs == second.lhs
                  && first.rhs.size() == second.rhs.size()
                  && first.dot_position == second.dot_position
                  && first.left_extent == second.left_extent
                  && first.pivot == second.pivot
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

std::ostream& operator<<(std::ostream& out, const EPN& epn)
{
    out << std::string("[")
        << production_rule_to_string(std::make_pair(epn.lhs, epn.rhs), epn.dot_position)
        << std::string(", ")
        << std::to_string(epn.left_extent)
        << std::string(", ")
        << std::to_string(epn.pivot)
        << std::string(", ")
        << std::to_string(epn.right_extent)
        << std::string("]");

    return out;
}