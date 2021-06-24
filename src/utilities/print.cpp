/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Defines print and tostring functions for various structures.
 */

#include <algorithm>
#include <numeric>
#include <iostream>
#include <sstream>
#include "print.hpp"

/**
 * @brief Returns a production rule in string forme. If the right hand side
 * is empty, print an epsilon.
 *
 * @param rule Production rule to convert.
 * @param dot_index Index in the right-hand side of where to print the dot.
 *                  Default: -1.
 *
 * @return Production rule as a string.
 */
std::string production_rule_to_string(production_rule_t rule, std::size_t dot_index)
{
    std::string lhs = rule.first;
    std::vector<std::string> rhs = rule.second;
    std::stringstream ss;

    ss << lhs << " ::= ";

    if (rhs.size() == 0 && dot_index == (std::size_t)-1)
    {
        ss << "ϵ";
    }

    for (size_t i = 0; i <= rhs.size(); i++)
    {
        if (i == dot_index)
        {
            ss << "·";
        }

        if (i < rhs.size())
        {
            ss << rhs.at(i);
        }
    }

    return ss.str();
}

/**
 * @brief Prints a descriptor set to standard output.
 *
 * @param descriptors Descriptor set to print.
 */
void print_descriptors(descriptor_set_t descriptors)
{
    for (auto d : descriptors)
    {
        std::cout << d << std::endl;
    }
}

/**
 * @brief Prints an EPN set to standard output.
 *
 * @param epns EPN set to print.
 */
void print_epns(epn_set_t epns)
{
    for (auto e : epns)
    {
        std::cout << e << std::endl;
    }
}