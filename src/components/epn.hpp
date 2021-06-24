/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Contains the descriptor class, used for representing extended packed nodes.
 */

#pragma once

#include <vector>
#include "descriptor.hpp"

/**
 * Represents an extended packed node.
 */
class EPN
{
public:
    /* Left-hand side of the grammar slot. */
    std::string lhs;
    /* Right-hand side of the grammar slot. */
    std::vector<std::string> rhs;
    /* Position of the dot in the grammar slot. */
    unsigned int dot_position;
    unsigned int left_extent;
    unsigned int pivot;
    unsigned int right_extent;
public:
    EPN(const Descriptor& d);
    EPN(const Descriptor& d, unsigned int p);
    EPN(std::string k, std::vector<std::string> v, unsigned int d, unsigned int l, unsigned int p, unsigned int r);
public:
    size_t hash() const;
};

bool operator==(const EPN& first, const EPN& second);
std::ostream& operator<<(std::ostream& out, const EPN& epn);