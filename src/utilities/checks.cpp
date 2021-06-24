/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Checks the correctness of the output of a parser for some grammar and
 *   input sequence. Correctness is checked using the requirements R(1)-R(4)
 *   and P(1)-P(3) from Van Binsbergen (2018).
 */

#include <iostream>
#include "checks.hpp"

bool is_correct = true;

/**
 * @brief Check if an EPN is in a set of EPNs. Prints a message
 * if this is not the case.
 *
 * @param epn Descriptor to check.
 * @param epns Set of descriptors.
 *
 * @return True if the EPN is in the set, false otherwise.
 */
bool check_if_exists(EPN epn, epn_set_t epns)
{
    if (!epns.count(epn))
    {
        std::cout << "Missing EPN " << epn << std::endl;

        is_correct = false;
        return false;
    }

    return true;
}

/**
 * @brief Check if a descriptor is in a set of descriptors. Prints a message
 * if this is not the case.
 *
 * @param descriptor Descriptor to check.
 * @param descriptors Set of descriptors.
 *
 * @return True if the descriptor is in the set, false otherwise.
 */
bool check_if_exists(Descriptor descriptor, descriptor_set_t descriptors)
{
    if (!descriptors.count(descriptor))
    {
        std::cout << "Missing descriptor " << descriptor << std::endl;

        is_correct = false;
        return false;
    }

    return true;
}

/**
 * @brief Check correctness of the parsing output for some input.
 *
 * @param descriptors Output descriptor set.
 * @param epns Output EPN set.
 * @param grammar Input grammar.
 * @param input Input sequence.
 */
bool check_correctness(
    descriptor_set_t descriptors,
    epn_set_t epns,
    Grammar grammar,
    std::vector<std::string> input
)
{
    auto start_symbol_rules = grammar.get_production_rules(grammar.start_symbol);

    for (auto rule : start_symbol_rules)
    {
        /* Check requirement R(1). */
        check_if_exists(Descriptor(grammar.start_symbol, rule.second, 0, 0, 0), descriptors);
    }

    for (auto descriptor : descriptors)
    {
        if (!descriptor.is_completed())
        {
            auto symbol = descriptor.get_next_symbol();

            if (grammar.terminals.count(symbol) && symbol == input[descriptor.right_extent])
            {
                Descriptor d = descriptor.copy_and_advance();
                d.right_extent++;

                /* Check requirement R(2). */
                bool result = check_if_exists(d, descriptors);

                if (result)
                {
                    /* Check requirement P(1). */
                    check_if_exists(EPN(d, descriptor.right_extent), epns);
                }
            }
            else
            {
                auto rules = grammar.get_production_rules(symbol);

                for (auto rule : rules)
                {
                    /* Check requirement R(3). */
                    check_if_exists(Descriptor(symbol, rule.second, 0, descriptor.right_extent, descriptor.right_extent), descriptors);
                }

                for (auto d : descriptors)
                {
                    if (d.lhs == symbol && d.is_completed() && d.left_extent == descriptor.right_extent)
                    {
                        Descriptor d_new = descriptor.copy_and_advance();
                        d_new.right_extent = d.right_extent;

                        /* Check requirement R(4). */
                        check_if_exists(d_new, descriptors);
                        /* Check requirement P(2). */
                        check_if_exists(EPN(d_new, descriptor.right_extent), epns);
                    }
                }
            }
        }
        else if (descriptor.is_empty())
        {
            /* Check requirement P(3). */
            check_if_exists(EPN(descriptor), epns);
        }
    }

    return is_correct;
}