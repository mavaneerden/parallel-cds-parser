/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Interface with command line argument parser.
 */

#pragma once

#include "../components/grammar.hpp"

std::tuple<Grammar, std::vector<std::string>, bool> parse_arguments(int argc, char const *argv[]);