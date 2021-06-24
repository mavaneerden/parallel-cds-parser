/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Contains the definition of the Thread Tree parallel parser.
 */

#pragma once

#include "optimisations.hpp"
#include <atomic>
#ifdef OPTIMISATION_TREE_FUTURE
#include <future>
#endif
#include "../../components/parser.hpp"

/**
 * Represents the Thread Tree parser. Derived from the Parser class.
 */
class ThreadTreeParser : public Parser
{
public:
    epn_set_t epn_set;
#ifdef OPTIMISATION_TREE_GRANULAR_GLOBAL
    std::atomic<int> working_threads;
#endif
    std::atomic<int> num_descriptors;
    std::atomic<int> num_threads;
public:
    ThreadTreeParser(Grammar g) : Parser(g) {
        num_descriptors = 0;
#ifdef OPTIMISATION_TREE_GRANULAR_GLOBAL
        working_threads = 0;
#endif
    };
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
    void extend_worklist(
        std::vector<production_rule_t> rules,
        int left_extent = 0,
        int right_extent = 0
    );
    void add_to_worklist(Descriptor descriptor);
#ifdef OPTIMISATION_TREE_FUTURE
    void thread_function(std::promise<descriptor_set_t>&& promise, Descriptor descriptor, descriptor_set_t descriptors);
#else
    void thread_function(Descriptor descriptor, descriptor_set_t descriptors);
#endif
    void add_thread(Descriptor descriptor);
};