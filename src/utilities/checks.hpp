/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Interface to correctness checker.
 */
#pragma once

#include "types.hpp"
#include "../components/grammar.hpp"

bool check_correctness(
    descriptor_set_t descriptors,
    epn_set_t epns,
    Grammar grammar,
    std::vector<std::string> input
);