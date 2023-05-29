#pragma once

class FreeList
{
public:
    using size_type = size_t;
    using pair_type = std::pair<size_type, size_type>;
    using list_type = std::list<pair_type>;

public:
    pair_type add(size_type size)
    {
        for (auto it = free_list.begin(); it != free_list.end(); ++it)
        {
            auto& free = *it;
            size_type free_size = free.second - free.first;
            if (free_size >= size)
            {
                if (free_size == size)
                {
                    pair_type range(free.first, free.second);
                    last = std::max(last, range.second);
                    free_list.erase(it);
                    return range;
                }
                pair_type range(free.first, free.first + size);
                last = std::max(last, range.second);
                free.first += size;
                return range;
            }
        }
        last += size;
        return { last - size, last };
    }

    void erase(const pair_type& range)
    {
        erase(range.first, range.second);
    }

    void erase(size_type first, size_type second)
    {
        list_type::iterator iter = free_list.end();
        std::vector<list_type::iterator> potential_overlaps;
        for (auto it = free_list.begin(); it != free_list.end(); ++it)
        {
            auto& free = *it;
            // Range is already free, so return
            if (free.first <= first && free.second >= second)
                return;
            // Detect if touching (NOTE: Doesn't work on overlapping nodes)
            if (free.second == first)
            {
                first = free.first;
                free.second = second;
                iter = it;
            }
            else if (free.first == second)
            {
                second = free.second;
                free.first = first;
                potential_overlaps.emplace_back(it);
            }
        }
        // Remove all overlaps
        for (auto it : potential_overlaps)
        {
            auto& free = *it;
            if (free.second >= first && free.first <= second)
                free_list.erase(it);
        }
        if (second == last)
            last = first;
        else free_list.emplace(iter, first, second);
    }

    list_type::iterator begin() { return free_list.begin(); }
    list_type::iterator end() { return free_list.end(); }
    list_type::const_iterator begin() const { return free_list.begin(); }
    list_type::const_iterator end() const { return free_list.end(); }

    size_t size() const { return last; }
    bool empty() const { return free_list.empty(); }

private:
    list_type free_list = {};
    size_type last = 0;
};