/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Declares print and tostring functions for various structures.
 */

#pragma once

#include "types.hpp"

std::string production_rule_to_string(production_rule_t rule, std::size_t dot_index = -1);
void print_descriptors(descriptor_set_t descriptors);
void print_epns(epn_set_t epns);