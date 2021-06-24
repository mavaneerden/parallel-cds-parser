/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Implemenation of a serial CDS descriptor-processing parser.
 */

#include "sequential_parser.hpp"
#include <iostream>

// #define COLLECT_NUM_DERIVATIONS
#define COLLECT_NUM_ACTIONS

/**
 * @brief Adds a single descriptor to the worklist if it doesn't already exist
 * in the descriptor set
 *
 * @param descriptor Descriptor to add to the worklist.
 */
void SequentialParser::add_to_worklist(Descriptor descriptor)
{
#ifdef COLLECT_NUM_DERIVATIONS
    if (descriptor.lhs == grammar.start_symbol
        && descriptor.is_completed()
        && descriptor.left_extent == 0
        && descriptor.right_extent == input.size())
    {
        num_derivations++;
    }
#endif

    if (!descriptor_set.count(descriptor))
    {
        worklist.insert(descriptor);
    }
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
void SequentialParser::extend_worklist(
    std::vector<production_rule_t> rules,
    int left_extent,
    int right_extent
)
{
    for (auto rule : rules)
    {
        Descriptor d(rule.first, rule.second, 0, left_extent, right_extent);

        add_to_worklist(d);
    }
}

/**
 * @brief Implements the 'match' operation: add a new descriptor and EPN if the
 * current descriptor matches the correct terminal in the input string.
 *
 * @param descriptor Descriptor that is being processed.
 */
void SequentialParser::match(Descriptor descriptor)
{
#ifdef COLLECT_NUM_ACTIONS
    num_match++;
#endif
    std::string terminal = descriptor.get_next_symbol();

    if (input.size() > 0 && terminal == input[descriptor.right_extent])
    {
        Descriptor d = descriptor.copy_and_advance();
        d.right_extent++;

        add_to_worklist(d);

        epn_set.insert(EPN(d, descriptor.right_extent));
    }
}

/**
 * @brief Implements the 'match' operation: add a new descriptor for every valid
 * alternative of the given nonterminal symbol.
 *
 * @param symbol Nonterminal symbol to find alternatives of.
 * @param pivot Pivot of the currently processed descriptor.
 */
void SequentialParser::descend(
    std::string symbol,
    unsigned int pivot
)
{
#ifdef COLLECT_NUM_ACTIONS
    num_descend++;
#endif
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
void SequentialParser::skip(
    Descriptor descriptor,
    std::unordered_set<unsigned int> right_extents
)
{
#ifdef COLLECT_NUM_ACTIONS
    num_skip++;
#endif
    for (int right_extent : right_extents)
    {
        Descriptor new_descriptor(descriptor);
        new_descriptor.right_extent = right_extent;

        add_to_worklist(new_descriptor);

        epn_set.insert(EPN(new_descriptor, descriptor.right_extent));
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
void SequentialParser::ascend(
    descriptor_set_t descriptors,
    unsigned int right_extent
)
{
#ifdef COLLECT_NUM_ACTIONS
    num_ascend++;
#endif
    for (auto descriptor : descriptors)
    {
        Descriptor new_descriptor(descriptor);
        new_descriptor.right_extent = right_extent;

        add_to_worklist(new_descriptor);

        epn_set.insert(EPN(new_descriptor, descriptor.right_extent));
    }
}

/**
 * @brief Processes a descriptor. Chooses one of 'match', 'ascend', 'descend',
 * 'skip' and calls the function for the chosen operation.
 *
 * @param descriptor Descriptor to be processed.
 */
void SequentialParser::process_descriptor(Descriptor descriptor)
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
            for (auto d : descriptor_set)
            {
                if (d.lhs == symbol && d.left_extent == descriptor.right_extent && d.is_completed())
                {
                    right_extents.insert(d.right_extent);
                }
            }

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

        for (auto d : descriptor_set)
        {
            if (!d.is_completed() && d.get_next_symbol() == descriptor.lhs && d.right_extent == descriptor.left_extent)
            {
                descriptors.insert(d.copy_and_advance());
            }
        }

        ascend(descriptors, descriptor.right_extent);

        if (descriptor.is_empty())
        {
            epn_set.insert(EPN(descriptor));
        }
    }
}

/**
 * @brief Call the parse method of the base class.
 */
std::tuple<descriptor_set_t, epn_set_t>
SequentialParser::parse(std::vector<std::string> input_sequence)
{
    return Parser::parse(input_sequence);
}

/**
 * @brief Processes descriptors one by one, taking them from the beginning of
 * the set.
 */
std::tuple<descriptor_set_t, epn_set_t> SequentialParser::loop()
{
#ifdef COLLECT_NUM_ACTIONS
    num_ascend = 0;
    num_descend = 0;
    num_match = 0;
    num_skip = 0;
#endif
    extend_worklist(
        grammar.get_production_rules(grammar.start_symbol)
    );

    while (!worklist.empty())
    {
        Descriptor d = *worklist.begin();
        descriptor_set.insert(d);

        process_descriptor(d);

        num_descriptors++;
        worklist.erase(d);
    }

    return std::make_tuple(descriptor_set, epn_set);
}

/**
 * @brief Print data for experiments.
 */
void SequentialParser::print_data()
{
    std::cout << input.size()
#ifdef COLLECT_NUM_ACTIONS
              << "," << num_match
              << "," << num_descend
              << "," << num_skip
              << "," << num_ascend
#else
              << "," << timer.elapsedMilliseconds()
              << "," << num_descriptors
              << "," << 1
              << "," << descriptor_set.size()
              << "," << epn_set.size()
#ifdef COLLECT_NUM_DERIVATIONS
              << "," << num_derivations
#endif
#endif
              << std::endl;
}
