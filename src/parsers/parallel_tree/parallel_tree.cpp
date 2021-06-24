/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Parallel implemenation of a CDS descriptor-processing parser.
 *   Uses a tree of threads, where each node has one parent and can have
 *   multiple or no children.
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <algorithm>
#include "parallel_tree.hpp"

#define WORKLIST_SIZE_THRESHOLD 32
#ifndef OPTIMISATION_TREE_GRANULAR_GLOBAL
// #define CORRECTNESS_FIX
// thread_local bool force = false;
#endif

thread_local descriptor_set_t worklist;
thread_local descriptor_set_t descriptor_set;
thread_local std::vector<std::thread> threads;
#ifdef OPTIMISATION_TREE_FUTURE
thread_local std::vector<std::promise<descriptor_set_t>> promises;
thread_local std::vector<std::future<descriptor_set_t>> futures;
#endif
std::mutex epn_set_mutex;
#ifndef OPTIMISATION_TREE_FUTURE
std::shared_mutex descriptor_set_mutex;
descriptor_set_t descriptor_set_global;
#endif
#ifdef OPTIMISATION_TREE_GRANULAR_GLOBAL
std::shared_mutex descended_set_mutex;
std::vector<Descriptor> descended_descriptors;
std::shared_mutex ascended_set_mutex;
std::vector<Descriptor> ascended_descriptors;
#endif
#ifdef OPTIMISATION_TREE_BETTER_LOCAL_SET
thread_local size_t global_set_index = 0;
std::shared_mutex global_set_mutex;
std::vector<Descriptor> global_descriptors;
#endif

/**
 * @brief Call the parse method of the base class.
 */
std::tuple<descriptor_set_t, epn_set_t> ThreadTreeParser::parse(std::vector<std::string> input_sequence)
{
    return Parser::parse(input_sequence);
}

/**
 * @brief Populate the worklist and descriptor set, then add one thread for each
 * descriptor in the set. Add all promised descriptor sets to the current set
 * and return it in the output.
 */
std::tuple<descriptor_set_t, epn_set_t> ThreadTreeParser::loop()
{
    extend_worklist(
        grammar.get_production_rules(grammar.start_symbol)
    );

    for (auto descriptor : worklist)
    {
        descriptor_set.insert(descriptor);
    }

    for (auto descriptor : descriptor_set)
    {
        add_thread(descriptor);
    }

#ifdef OPTIMISATION_TREE_FUTURE
    for (auto& future : futures)
    {
        for (auto item : future.get())
        {
            descriptor_set.insert(item);
        }
    }
#endif

    for (size_t i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }

#ifdef OPTIMISATION_TREE_FUTURE
    return std::make_tuple(descriptor_set, epn_set);
#else
    return std::make_tuple(descriptor_set_global, epn_set);
#endif
}

/**
 * @brief Print data for experiments.
 */
void ThreadTreeParser::print_data()
{
    std::cout << input.size()
              << "," << timer.elapsedMilliseconds()
              << "," << num_descriptors
              << "," << num_threads
#ifdef OPTIMISATION_TREE_FUTURE
              << ","<< descriptor_set.size()
#else
              << ","<< descriptor_set_global.size()
#endif
              << "," << epn_set.size()
              << std::endl;
}

/**
 * @brief Function that is used to spawn threads. Loops until there is no more
 * work to be done. Creates a new thread for all but one new descriptor once the
 * size of the worklist hits a certain threshold. The one descriptor that does not
 * get a new thread is processed by the current thread.
 * Once all items have been processed, the promises of the child threads are
 * are collected and sent to the parent thread.
 */
#ifdef OPTIMISATION_TREE_FUTURE
void ThreadTreeParser::thread_function(std::promise<descriptor_set_t>&& promise, Descriptor descriptor, descriptor_set_t descriptors_parent)
#else
void ThreadTreeParser::thread_function(Descriptor descriptor, descriptor_set_t descriptors_parent)
#endif
{
    Descriptor d;

    worklist.insert(descriptor);
    descriptor_set = descriptors_parent;

    while (!worklist.empty())
    {
        auto worklist_begin = worklist.begin();
#ifdef CORRECTNESS_FIX
        force = false;
#endif
#ifdef OPTIMISATION_TREE_BETTER_LOCAL_SET
        {
            std::shared_lock<std::shared_mutex> lock(global_set_mutex);

            for (size_t i = global_set_index; i < global_descriptors.size(); i++)
            {
                descriptor_set.insert(global_descriptors[i]);
            }

            std::cout << "SIZE: " << global_descriptors.size() << std::endl;

            global_set_index = global_descriptors.size() - 1L;
        }
#endif

        if (worklist.size() >= WORKLIST_SIZE_THRESHOLD)
        {
#ifdef OPTIMISATION_TREE_COST_REDUCTION_LOCAL_DESCRIPTORS
            for (auto item : worklist)
            {
                descriptor_set.insert(item);
            }
#endif

            for (size_t i = 0; i < worklist.size() - WORKLIST_SIZE_THRESHOLD + 1; i++)
            {
                d = *worklist_begin++;
                add_thread(d);
                worklist.erase(d);
            }
        }

        d = *worklist_begin;
        descriptor_set.insert(d);

#ifdef OPTIMISATION_TREE_BETTER_LOCAL_SET
        {
            std::unique_lock<std::shared_mutex> lock(global_set_mutex);
            global_descriptors.push_back(d);
        }
#endif

#ifndef OPTIMISATION_TREE_FUTURE
        /* Necessary because a write/write conflict can occur. */
        {
            std::unique_lock<std::shared_mutex> lock(descriptor_set_mutex);
#ifdef OPTIMISATION_TREE_COST_REDUCTION_GLOBAL_DESCRIPTORS
            bool success = descriptor_set_global.insert(d).second;

            if (!success
#ifdef CORRECTNESS_FIX
                && !d.force_process
#endif
            )
            {
                worklist.erase(d);
                continue;
            }
#else
            descriptor_set_global.insert(d);
#endif
        }
#endif
        process_descriptor(d);

        num_descriptors.fetch_add(1);
        worklist.erase(d);
    }

#ifdef OPTIMISATION_TREE_GRANULAR_GLOBAL
    working_threads.fetch_sub(1);
#endif

#ifdef OPTIMISATION_TREE_FUTURE
    for (auto& future : futures)
    {
        for (auto item : future.get())
        {
            descriptor_set.insert(item);
        }
    }
#endif

    for (size_t i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }

#ifdef OPTIMISATION_TREE_FUTURE
    promise.set_value(descriptor_set);
#endif
}

/**
 * @brief Processes a descriptor. Chooses one of 'match', 'ascend', 'descend',
 * 'skip' and calls the function for the chosen operation.
 *
 * @param descriptor Descriptor to be processed.
 */
void ThreadTreeParser::process_descriptor(Descriptor descriptor)
{
#ifdef CORRECTNESS_FIX
    if (descriptor.force_process)
    {
        force = true;
    }
#endif

    if (!descriptor.is_completed())
    {
        std::unordered_set<unsigned int> right_extents;
#ifdef CORRECTNESS_FIX
        std::vector<production_rule_t> skipped_rules;
#endif
        std::string symbol = descriptor.get_next_symbol();

        if (grammar.terminals.count(symbol))
        {
            match(descriptor);
        }
        else
        {
#ifdef OPTIMISATION_TREE_GLOBAL_DESCRIPTORS
            {
                std::shared_lock<std::shared_mutex> lock(descriptor_set_mutex);
                for (auto d : descriptor_set_global)
                {
                    if (d.lhs == symbol && d.left_extent == descriptor.right_extent && d.is_completed())
                    {
                        right_extents.insert(d.right_extent);
                    }
                }
            }
#else
#ifdef OPTIMISATION_TREE_GRANULAR_GLOBAL
            {
                std::shared_lock<std::shared_mutex> lock(ascended_set_mutex);
                for (auto d : ascended_descriptors)
                {
                    if (d.lhs == symbol && d.left_extent == descriptor.right_extent)
                    {
                        right_extents.insert(d.right_extent);
                    }
                }
            }

#else
            for (auto d : descriptor_set)
            {
                if (d.lhs == symbol && d.left_extent == descriptor.right_extent && d.is_completed())
                {
                    right_extents.insert(d.right_extent);
#ifdef CORRECTNESS_FIX
                    skipped_rules.push_back(std::make_pair(symbol, d.rhs));
#endif
                }
            }
#endif
#endif

#ifdef OPTIMISATION_TREE_GRANULAR_GLOBAL
            {
                std::unique_lock<std::shared_mutex> lock(descended_set_mutex);
                descended_descriptors.push_back(descriptor);
            }
#endif
            if (right_extents.size() == 0)
            {
                descend(symbol, descriptor.right_extent);
            }
            else
            {
#ifdef CORRECTNESS_FIX
                std::vector<production_rule_t> rules = grammar.get_production_rules(symbol);
                std::vector<production_rule_t> remaining_rules;

                std::set_difference(rules.begin(), rules.end(), skipped_rules.begin(), skipped_rules.end(), std::inserter(remaining_rules, remaining_rules.begin()));

                for(auto rule : remaining_rules)
                {
                    Descriptor d(rule.first, rule.second, 0, descriptor.right_extent, descriptor.right_extent, true);
                    worklist.insert(d);
                    descriptor_set.erase(d);
                }
#endif
                skip(descriptor.copy_and_advance(), right_extents);
            }
        }
    }
    else
    {
        descriptor_set_t descriptors;

#ifdef OPTIMISATION_TREE_GLOBAL_DESCRIPTORS
        {
            std::shared_lock<std::shared_mutex> lock(descriptor_set_mutex);

            for (auto d : descriptor_set_global)
            {
                if (!d.is_completed() && d.get_next_symbol() == descriptor.lhs && d.right_extent == descriptor.left_extent)
                {
                    descriptors.insert(d.copy_and_advance());
                }
            }
        }
#else
#ifdef OPTIMISATION_TREE_GRANULAR_GLOBAL
        {
            std::shared_lock<std::shared_mutex> lock(descended_set_mutex);

            for (auto d : descended_descriptors)
            {
                if (d.get_next_symbol() == descriptor.lhs && d.right_extent == descriptor.left_extent)
                {
                    descriptors.insert(d.copy_and_advance());
                }
            }
        }

        {
            std::unique_lock<std::shared_mutex> lock(ascended_set_mutex);
            ascended_descriptors.push_back(descriptor);
        }
#else
        for (auto d : descriptor_set)
        {
            if (!d.is_completed() && d.get_next_symbol() == descriptor.lhs && d.right_extent == descriptor.left_extent)
            {
                descriptors.insert(d.copy_and_advance());
            }
        }
#endif
#endif
        ascend(descriptors, descriptor.right_extent);

        if (descriptor.is_empty())
        {
            {
                std::lock_guard<std::mutex> lock(epn_set_mutex);
                epn_set.insert(EPN(descriptor));
            }
        }
    }
}

/**
 * @brief Implements the 'match' operation: add a new descriptor and EPN if the
 * current descriptor matches the correct terminal in the input string.
 *
 * @param descriptor Descriptor that is being processed.
 */
void ThreadTreeParser::match(Descriptor descriptor)
{
    std::string terminal = descriptor.get_next_symbol();

    if (input.size() > 0 && terminal == input[descriptor.right_extent])
    {
        Descriptor d = descriptor.copy_and_advance();
        d.right_extent++;

        add_to_worklist(d);

        {
            std::lock_guard<std::mutex> lock(epn_set_mutex);
            epn_set.insert(EPN(d, descriptor.right_extent));
        }
    }
}

/**
 * @brief Implements the 'match' operation: add a new descriptor for every valid
 * alternative of the given nonterminal symbol.
 *
 * @param symbol Nonterminal symbol to find alternatives of.
 * @param pivot Pivot of the currently processed descriptor.
 */
void ThreadTreeParser::descend(
    std::string symbol,
    unsigned int pivot
)
{
    extend_worklist(
        grammar.get_production_rules(symbol),
        pivot,
        pivot
    );
}

/**
 * @brief Implements the 'skip' operation: skip over the nonterminal symbol
 * and find the valid descriptors.
 *
 * @param descriptor Descriptor currently being processed, with nonterminal skip.
 * @param right_extents Set of valid right extents for new descriptors.
 */
void ThreadTreeParser::skip(
    Descriptor descriptor,
    std::unordered_set<unsigned int> right_extents
)
{
    for (int right_extent : right_extents)
    {
        Descriptor new_descriptor(descriptor);
        new_descriptor.right_extent = right_extent;

        add_to_worklist(new_descriptor);

        {
            std::lock_guard<std::mutex> lock(epn_set_mutex);
            epn_set.insert(EPN(new_descriptor, descriptor.right_extent));
        }
    }
}

/**
 * @brief Implements the 'ascend' operation: a production rules has been parsed
 * and the next valid descriptors are added to the worklist.
 *
 * @param descriptor_set Set of valid descriptors to be added, with incorrect
 *                       right extent.
 * @param right_extent Right extent for the new descriptors.
 */
void ThreadTreeParser::ascend(
    descriptor_set_t descriptors,
    unsigned int right_extent
)
{
    for (auto descriptor : descriptors)
    {
        Descriptor new_descriptor(descriptor);
        new_descriptor.right_extent = right_extent;

        add_to_worklist(new_descriptor);


        {
            std::lock_guard<std::mutex> lock(epn_set_mutex);
            epn_set.insert(EPN(new_descriptor, descriptor.right_extent));
        }
    }
}

void ThreadTreeParser::add_to_worklist(Descriptor descriptor)
{
    size_t count;
#ifdef OPTIMISATION_TREE_GLOBAL_DESCRIPTORS
    {
        std::shared_lock<std::shared_mutex> lock(descriptor_set_mutex);
        count = descriptor_set_global.count(descriptor);
    }
#else
    count = descriptor_set.count(descriptor);
#endif
    if (!count)
    {
        worklist.insert(descriptor.copy_and_force());
    }
    /* The descriptor might not be in the local descriptor set, so it is added. */
#if defined(OPTIMISATION_TREE_GLOBAL_DESCRIPTORS) || defined(OPTIMISATION_TREE_BETTER_LOCAL_SET)
    else
    {
        descriptor_set.insert(descriptor);
    }
#endif
}

/**
 * @brief Extends the worklist with the provided rules, using the provided left
 * and right extents. Does not add a new descriptor if it is already in the
 * descriptors set.
 *
 * @param rules Rules to extend the worklist with.
 * @param left_extent Left extent of the new descriptors.
 * @param right_extent Right extent of the new descriptors.
 */
void ThreadTreeParser::extend_worklist(
    std::vector<production_rule_t> rules,
    int left_extent,
    int right_extent
)
{
    for(auto rule : rules)
    {
        Descriptor descriptor = Descriptor(rule.first, rule.second, 0, left_extent, right_extent);

        add_to_worklist(descriptor);
    }
}

/**
 * @brief Add a new thread to the tree. Each thread gets its own promise that it
 * needs to fulfill.
 *
 * @param descriptor Descriptor to pass to the new thread.
 */
void ThreadTreeParser::add_thread(Descriptor descriptor)
{
#ifdef OPTIMISATION_TREE_GRANULAR_GLOBAL
    working_threads.fetch_add(1);
#endif
#ifdef OPTIMISATION_TREE_FUTURE
    promises.push_back(std::promise<descriptor_set_t>());
    futures.push_back(promises[promises.size() - 1].get_future());
    threads.push_back(std::thread(&ThreadTreeParser::thread_function, this, std::move(promises[promises.size() - 1]), descriptor, descriptor_set));
#else
    threads.push_back(std::thread(&ThreadTreeParser::thread_function, this, descriptor, descriptor_set));
#endif
    num_threads.fetch_add(1);
}