/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Contains the base class for all parser implementations.
 */

#pragma once

#include "descriptor.hpp"
#include "epn.hpp"
#include "grammar.hpp"
#include "../utilities/timer.hpp"

/**
 * Base class for all parsers.
 */
class Parser
{
public:
    /* Input sequence. */
    std::vector<std::string> input;
    /* Input grammar. */
    Grammar grammar;
    /* Timer used for experiments. */
    Timer timer;
public:
    Parser(Grammar g);
public:
    std::tuple<descriptor_set_t, epn_set_t> parse(std::vector<std::string> input_sequence);
private:
    virtual std::tuple<descriptor_set_t, epn_set_t> loop() = 0;
    virtual void print_data() = 0;
};