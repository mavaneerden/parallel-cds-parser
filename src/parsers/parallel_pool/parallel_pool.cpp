/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Parallel implemenation of a CDS descriptor-processing parser.
 *   Uses a pool of threads that take work from the worklist.
 */

#include <iostream>
#include <thread>
#include "../../utilities/print.hpp"
#include "parallel_pool.hpp"

#include <algorithm>

/* Number of threads to spawn. */
#define NUM_THREADS 16
/* Define when getting thread usage data. */
#define WORKING_THREADS_DATA
/* Define when getting action usage data. */
// #define ACTIONS_DATA

namespace
{
#ifdef WORKING_THREADS_DATA
    std::array<unsigned long, NUM_THREADS + 1> working_treads_data;
#endif
#ifdef ACTIONS_DATA
    std::array<int, 4> actions_data;
#endif
    std::mutex epn_set_mutex;
#ifndef OPTIMISATION_POOL_QUEUES
    descriptor_set_t worklist;
    std::mutex worklist_mutex;
#endif
    descriptor_set_t descriptor_set;
#ifdef OPTIMISATION_POOL_SHARED_LOCKS
    std::shared_mutex descriptor_set_mutex;
#else
    std::mutex descriptor_set_mutex;
#endif
    epn_set_t epn_set;
    std::atomic<int> num_descriptors;
    std::atomic<int> working_threads;
    std::atomic<bool> stop_threads;
    std::condition_variable thread_cv;
    std::mutex thread_cv_mutex;
    std::condition_variable main_cv;
    std::mutex main_cv_mutex;
#ifdef OPTIMISATION_POOL_GLL_P
    std::unordered_map<
        std::string,
        std::pair<
            std::unordered_map<unsigned int, std::pair<std::unordered_set<unsigned int>, std::unique_ptr<std::shared_mutex>>>,
            std::unique_ptr<std::shared_mutex>
        >
    > right_extents_map;
#endif
#ifdef OPTIMISATION_POOL_QUEUES
    std::vector<std::mutex> worklist_mutexes;
    std::vector<descriptor_set_t> worklists;
    std::mutex global_worklist_mutex;
    descriptor_set_t global_worklist;
    size_t rr_thread_id;
#endif
}

/**
 * @brief Call the parse method of the base class.
 */
std::tuple<descriptor_set_t, epn_set_t> ThreadPoolParser::parse(std::vector<std::string> input_sequence)
{
    return Parser::parse(input_sequence);
}

/**
 * @brief Spawn threads at the start, then add descriptors for the start
 * symbol to the worklist. The main thread blocks until its condition variable
 * is notified, then it joins all threads and returns the output of the parser.
 */
std::tuple<descriptor_set_t, epn_set_t> ThreadPoolParser::loop()
{
    std::vector<std::thread> threads;
    unsigned int num_threads = NUM_THREADS;

    num_descriptors = 0;
    working_threads = 0;
    stop_threads = false;

#ifdef OPTIMISATION_POOL_QUEUES
    rr_thread_id = 0;
    std::vector<std::mutex> tmp(num_threads);
    worklist_mutexes.swap(tmp);
#endif

    for (unsigned int i = 0; i < num_threads; i++)
    {
#ifdef OPTIMISATION_POOL_QUEUES
        worklists.push_back(descriptor_set_t());
        threads.push_back(std::thread(&ThreadPoolParser::thread_function, this, i));
#else
        threads.push_back(std::thread(&ThreadPoolParser::thread_function, this));
#endif
    }

    extend_worklist(
        grammar.get_production_rules(grammar.start_symbol)
    );


#ifndef OPTIMISATION_POOL_QUEUES
    {
        std::unique_lock<std::mutex> lock(main_cv_mutex);
        main_cv.wait(lock);
    }
#else
    while(!(all_worklists_empty() && working_threads.load() == 0 && global_worklist.empty()))
    {
#ifdef WORKING_THREADS_DATA
        working_treads_data[working_threads.load()]++;
#endif

        if (global_worklist.empty())
        {
            continue;
        }

        std::lock_guard<std::mutex> lock(global_worklist_mutex);
        {
            for (auto item : global_worklist)
            {
                {
                    std::lock_guard<std::mutex> lock_local(worklist_mutexes[rr_thread_id]);
                    worklists[rr_thread_id++].insert(item);
                }

                rr_thread_id %= num_threads;
            }

            global_worklist.clear();
        }

        thread_cv.notify_all();
    }

    stop_threads.store(true);
    thread_cv.notify_all();
#endif

    for (auto& thread : threads)
    {
        thread.join();
    }

    return std::make_tuple(descriptor_set, epn_set);
}

/**
 * @brief Print data for experiments.
 */
void ThreadPoolParser::print_data()
{
#ifdef WORKING_THREADS_DATA
    std::cout << input.size();
    for (auto element : working_treads_data)
    {
        std::cout << "," << element;
    }

    std::cout << std::endl;
#else
#ifdef ACTIONS_DATA
    std::cout << input.size();
    for (auto element : actions_data)
    {
        std::cout << "," << element;
    }

    std::cout << std::endl;
#else
    std::cout << input.size()
              << "," << timer.elapsedMilliseconds()
              << "," << num_descriptors
              << "," << NUM_THREADS
              << "," << descriptor_set.size()
              << "," << epn_set.size()
              << std::endl;
#endif
#endif
}

/**
 * @brief Function that is used to spawn threads. Loops until stop_threads is
 * true. Each loop the thread blocks until its condition variable is notified,
 * unless signalled to stop or the worklist is not empty.
 * It processed a descriptor from the worklist and at the end of each iteration,
 * it signals all threads to stop when the conditions are right.
 */
#ifdef OPTIMISATION_POOL_QUEUES
void ThreadPoolParser::thread_function(unsigned int thread_id)
#else
void ThreadPoolParser::thread_function()
#endif
{
    Descriptor d;
    bool process;
#ifndef OPTIMISATION_POOL_QUEUES
    int working_threads_count;
#endif

    while (true)
    {
        process = false;

        {
#ifndef OPTIMISATION_POOL_QUEUES
            std::unique_lock<std::mutex> lock(thread_cv_mutex);
#endif

            /* Wait for notification that new item was added to the work list,
               or that all threads need to stop. */
#ifdef OPTIMISATION_POOL_QUEUES
            while (worklists[thread_id].size() == 0 && !stop_threads.load())
#else
            if (worklist.size() == 0 && !stop_threads.load())
#endif
            {
#ifndef OPTIMISATION_POOL_QUEUES
                thread_cv.wait(lock);
#endif
            }
        }

        /* Break out of the loop if signalled to stop. */
        if (stop_threads.load())
        {
            break;
        }

        {
#ifdef OPTIMISATION_POOL_QUEUES
            std::lock_guard<std::mutex> lock(worklist_mutexes[thread_id]);
#else
            std::lock_guard<std::mutex> lock(worklist_mutex);
#endif

            /* Necessary because other threads could have emptied the worklist. */
#ifdef OPTIMISATION_POOL_QUEUES
            if (worklists[thread_id].size() == 0)
#else
            if (worklist.size() == 0)
#endif
            {
                continue;
            }

            working_threads.fetch_add(1);

            /* Get the first item from the worklist and remove it. */
#ifdef OPTIMISATION_POOL_QUEUES
            d = *worklists[thread_id].begin();
            worklists[thread_id].erase(d);
#else
            d = *worklist.begin();
            worklist.erase(d);
#endif
        }

        {
#ifdef OPTIMISATION_POOL_SHARED_LOCKS
            std::unique_lock<std::shared_mutex> lock(descriptor_set_mutex);
#else
            std::lock_guard<std::mutex> lock(descriptor_set_mutex);
#endif

            /* Necessary because some other threads may have added the descriptor
               to the set already. */
            if (!descriptor_set.count(d))
            {
                descriptor_set.insert(d);
                process = true;
            }
        }

        if (process)
        {
            process_descriptor(d);
            num_descriptors.fetch_add(1);
        }

        /* Notify the main thread if all threads are idle and the worklist is empty. */
#ifdef OPTIMISATION_POOL_QUEUES
        working_threads.fetch_sub(1);
#else
        working_threads_count = working_threads.fetch_sub(1);

        if (working_threads_count - 1 == 0 && worklist.empty())
        {
            stop_threads.store(true);
            main_cv.notify_one();
            thread_cv.notify_all();
            break;
        }
#endif
    }
}

/**
 * @brief Processes a descriptor. Implementation is identical to that of the
 * sequential parser, except locks are added around critical blocks.
 *
 * @param descriptor Descriptor to process.
 */
void ThreadPoolParser::process_descriptor(Descriptor descriptor)
{
    if (!descriptor.is_completed())
    {
        std::unordered_set<unsigned int> right_extents;
        std::string symbol = descriptor.get_next_symbol();

        if (grammar.terminals.count(symbol))
        {
            match(descriptor);
        }
        else
        {
#ifdef OPTIMISATION_POOL_GLL_P
            try
            {
                auto& outer = right_extents_map.at(symbol);
                {
                    std::shared_lock<std::shared_mutex> lock_outer(*outer.second.get());
                    auto& inner = outer.first.at(descriptor.right_extent);

                    {
                        std::shared_lock<std::shared_mutex> lock_inner(*inner.second.get());
                        right_extents = inner.first;
                    }
                }
            }
            catch(const std::out_of_range& e) {}
#else
            {
#ifdef OPTIMISATION_POOL_SHARED_LOCKS
                std::shared_lock<std::shared_mutex> lock(descriptor_set_mutex);
#else
                std::lock_guard<std::mutex> lock(descriptor_set_mutex);
#endif

                for (auto d : descriptor_set)
                {
                    if (d.lhs == symbol && d.left_extent == descriptor.right_extent && d.is_completed())
                    {
                        right_extents.insert(d.right_extent);
                    }
                }
            }
#endif

            if (right_extents.size() == 0)
            {
                descend(symbol, descriptor.right_extent);
            }
            else
            {
                skip(descriptor.copy_and_advance(), right_extents);
            }
        }
    }
    else
    {
        descriptor_set_t descriptors;

        {
#ifdef OPTIMISATION_POOL_SHARED_LOCKS
            std::shared_lock<std::shared_mutex> lock(descriptor_set_mutex);
#else
            std::lock_guard<std::mutex> lock(descriptor_set_mutex);
#endif

            for (auto d : descriptor_set)
            {
                if (!d.is_completed() && d.get_next_symbol() == descriptor.lhs && d.right_extent == descriptor.left_extent)
                {
                    descriptors.insert(d.copy_and_advance());
                }
            }
        }

#ifdef OPTIMISATION_POOL_GLL_P
        if (!right_extents_map.count(descriptor.lhs))
        {
            right_extents_map.insert(
                std::make_pair(
                    descriptor.lhs, std::make_pair(
                        std::unordered_map<unsigned int, std::pair<std::unordered_set<unsigned int>, std::unique_ptr<std::shared_mutex>>>(),
                        std::make_unique<std::shared_mutex>()
                    )
                )
            );
        }

        {
            std::unique_lock<std::shared_mutex> lock(*right_extents_map[descriptor.lhs].second.get());

            if (!right_extents_map[descriptor.lhs].first.count(descriptor.left_extent))
            {
                right_extents_map[descriptor.lhs].first.insert(
                    std::make_pair(
                        descriptor.left_extent, std::make_pair(
                            std::unordered_set<unsigned int>(),
                            std::make_unique<std::shared_mutex>()
                        )
                    )
                );
            }
        }

        {
            std::unique_lock<std::shared_mutex> lock(*right_extents_map[descriptor.lhs].first[descriptor.left_extent].second.get());
            right_extents_map[descriptor.lhs].first[descriptor.left_extent].first.insert(descriptor.right_extent);
        }
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
 * @brief Applies the match action to a descriptor. Implementation identical to
 * sequential implementation.
 *
 * @param descriptor Descriptor to process.
 */
void ThreadPoolParser::match(Descriptor descriptor)
{
    std::string terminal = descriptor.get_next_symbol();

#ifdef ACTIONS_DATA
    actions_data[0]++;
#endif

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
 * @brief Applies the descend action to a descriptor. Implementation identical to
 * sequential implementation.
 *
 * @param symbol Symbol use to get production rules from the grammar.
 * @param pivot Pivot of the processed descriptor.
 */
void ThreadPoolParser::descend(std::string symbol, unsigned int pivot)
{
#ifdef ACTIONS_DATA
    actions_data[1]++;
#endif

    extend_worklist(
        grammar.get_production_rules(symbol),
        pivot,
        pivot
    );
}

/**
 * @brief Applies the skip action to a descriptor. Implementation identical to
 * sequential implementation.
 *
 * @param descriptor Descriptor to process.
 * @param right_extents Right extents to apply to the new descriptors.
 */
void ThreadPoolParser::skip(Descriptor descriptor, std::unordered_set<unsigned int> right_extents)
{
#ifdef ACTIONS_DATA
    actions_data[3]++;
#endif

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
 * @brief Applies the ascend action to a descriptor. Implementation identical to
 * sequential implementation.
 *
 * @param descriptors Descriptors to process.
 * @param right_extent Right extent to apply to the new descriptors.
 */
void ThreadPoolParser::ascend(descriptor_set_t descriptors, unsigned int right_extent)
{
#ifdef ACTIONS_DATA
    actions_data[2]++;
#endif

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

/**
 * @brief Adds new descriptors to the worklist.
 *
 * @param rules Production rules used to make new descriptors.
 * @param left_extent Left extent.
 * @param right_extent Right extent.
 */
void ThreadPoolParser::extend_worklist(
    std::vector<production_rule_t> rules,
    int left_extent,
    int right_extent
)
{
    for (auto rule : rules)
    {
        add_to_worklist(Descriptor(rule.first, rule.second, 0, left_extent, right_extent));
    }
}

/**
 * @brief Adds a new descriptor to the worklist. Critical code blocks are
 * locked.
 *
 * @param descriptor Descriptor to add.
 */
void ThreadPoolParser::add_to_worklist(Descriptor descriptor)
{
    size_t count;

    {
#ifdef OPTIMISATION_POOL_SHARED_LOCKS
        std::shared_lock<std::shared_mutex> lock(descriptor_set_mutex);
#else
        std::lock_guard<std::mutex> lock(descriptor_set_mutex);
#endif
        count = descriptor_set.count(descriptor);
    }

    if (!count)
    {
        {
#ifdef OPTIMISATION_POOL_QUEUES
            std::lock_guard<std::mutex> lock(global_worklist_mutex);
            global_worklist.insert(descriptor);
#else
            std::lock_guard<std::mutex> lock(worklist_mutex);
            worklist.insert(descriptor);
            thread_cv.notify_one();
#endif
        }
    }
}

#ifdef OPTIMISATION_POOL_QUEUES
bool ThreadPoolParser::all_worklists_empty()
{
    for (size_t i = 0; i < worklists.size(); i++)
    {
        if (!worklists[i].empty())
        {
            return false;
        }
    }

    return true;
}
#endif