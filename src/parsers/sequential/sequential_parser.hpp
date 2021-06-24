/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Interface with serial parser.
 */

#pragma once

#include "../../components/parser.hpp"

class SequentialParser : public Parser
{
public:
    descriptor_set_t worklist;
    descriptor_set_t descriptor_set;
    epn_set_t epn_set;
    int num_descriptors;
    int num_derivations;
    int num_match;
    int num_descend;
    int num_ascend;
    int num_skip;
public:
    SequentialParser(Grammar g) : Parser(g) { num_descriptors = 0; };
public:
    std::tuple<descriptor_set_t, epn_set_t> parse(std::vector<std::string> input_sequence);
private:
    std::tuple<descriptor_set_t, epn_set_t> loop() override;
    void print_data() override;
    void process_descriptor(Descriptor descriptor);
    void match(Descriptor descriptor);
    void descend(std::string symbol, unsigned int pivot);
    void skip(Descriptor descriptor, std::unordered_set<unsigned int> right_extents);
    void ascend(descriptor_set_t descriptors, unsigned int right_extent);
    void add_to_worklist(Descriptor descriptor);
    void extend_worklist(
        std::vector<production_rule_t> rules,
        int left_extent = 0,
        int right_extent = 0
    );
};