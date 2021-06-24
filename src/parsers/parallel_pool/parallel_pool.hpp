/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Contains the definition of the Thread Pool parallel parser.
 */

#pragma once

#include "optimisations.hpp"
#include <atomic>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include "../../components/parser.hpp"

/**
 * Represents the Thread Pool parser. Derived from the Parser class.
 */
class ThreadPoolParser : public Parser
{
public:
// #ifndef OPTIMISATION_POOL_QUEUES
//     descriptor_set_t worklist;
//     std::mutex worklist_mutex;
// #endif
//     descriptor_set_t descriptor_set;
// #ifdef OPTIMISATION_POOL_SHARED_LOCKS
//     std::shared_mutex descriptor_set_mutex;
// #else
//     std::mutex descriptor_set_mutex;
// #endif
//     epn_set_t epn_set;
//     std::atomic<int> num_descriptors;
//     std::atomic<int> working_threads;
//     std::atomic<bool> stop_threads;
//     std::condition_variable thread_cv;
//     std::mutex thread_cv_mutex;
//     std::condition_variable main_cv;
//     std::mutex main_cv_mutex;
// #ifdef OPTIMISATION_POOL_GLL_P
//     std::unordered_map<
//         std::string,
//         std::pair<
//             std::unordered_map<unsigned int, std::pair<std::unordered_set<unsigned int>, std::unique_ptr<std::shared_mutex>>>,
//             std::unique_ptr<std::shared_mutex>
//         >
//     > right_extents_map;
// #endif
// #ifdef OPTIMISATION_POOL_QUEUES
//     std::vector<std::mutex> worklist_mutexes;
//     std::vector<descriptor_set_t> worklists;
//     std::mutex global_worklist_mutex;
//     descriptor_set_t global_worklist;
//     size_t rr_thread_id;
// #endif
public:
    ThreadPoolParser(Grammar g) : Parser(g) { /*num_descriptors = 0;*/ };
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
#ifdef OPTIMISATION_POOL_QUEUES
    void thread_function(unsigned int thread_id);
    bool all_worklists_empty();
#else
    void thread_function();
#endif
    void add_to_worklist(Descriptor descriptor);
};