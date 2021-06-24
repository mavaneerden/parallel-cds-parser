/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Contains the descriptor class, used for representing descriptors.
 */

#pragma once

#include <string>
#include <vector>

/**
 * Represents a descriptor.
 */
class Descriptor
{
public:
    /* Left-hand side of the grammar slot. */
    std::string lhs;
    /* Right-hand side of the grammar slot. */
    std::vector<std::string> rhs;
    /* Position of the dot in the grammar slot. */
    unsigned int dot_position;
    unsigned int left_extent;
    unsigned int right_extent;
    /* Whether the descriptor must be forcibly processed, even if it already has been processed. */
    bool force_process = false;
public:
    Descriptor();
    Descriptor(const Descriptor& d);
    Descriptor(std::string k, std::vector<std::string> v, unsigned int d, unsigned int l, unsigned int r);
    Descriptor(std::string k, std::vector<std::string> v, unsigned int d, unsigned int l, unsigned int r, bool f);
public:
    bool is_completed();
    bool is_empty();
    std::string get_next_symbol();
    void advance();
    Descriptor copy_and_advance();
    Descriptor copy_and_force();
    size_t hash() const;
    Descriptor& operator=(const Descriptor& descriptor);
};

bool operator==(const Descriptor& first, const Descriptor& second);
std::ostream& operator<<(std::ostream& out, const Descriptor& descriptor);