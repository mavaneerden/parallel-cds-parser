/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Defines types for use in the CDS algorithm.
 */

#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <unordered_set>
#include "hash_custom.hpp"

typedef std::unordered_set<Descriptor, hash_custom::hash<Descriptor>> descriptor_set_t;
typedef std::unordered_set<EPN, hash_custom::hash<EPN>> epn_set_t;
typedef std::pair<std::string, std::vector<std::string>> production_rule_t;