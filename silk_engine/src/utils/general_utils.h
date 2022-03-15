#pragma once


class GeneralUtils
{
public:
    template <typename T>
    static std::vector<std::vector<T>> makeClusters(const std::vector<T>& v)
    {
        std::vector<std::vector<T>> clusters;
    
        auto cluster_begin = v.begin();
        while (cluster_begin != v.end())
        {
            T elem = *cluster_begin;
    
            auto cluster_end = std::find_if(cluster_begin, v.end(),
                [&](const T& e) { return e != elem; });
            clusters.emplace_back(std::distance(cluster_begin, cluster_end), elem);
    
            cluster_begin = cluster_end;
        }
    
        return clusters;
    }

    template <typename T>
    static std::vector<std::vector<T>> groupDuplicates(std::vector<T> v)
    {
        std::vector<std::vector<T>> duplicates;

        size_t processed = 0;
        const size_t s = v.size();
        while (processed < s)
        {
            T to_find = v.back();
            duplicates.emplace_back();
            for (int i = v.size() - 1; i >= 0; --i)
            {
                if (v[i] == to_find)
                {
                    std::swap(v[i], v.back());
                    duplicates.back().emplace_back(v.back());
                    v.pop_back();
                    ++processed;
                }
            }
        }
        return duplicates;
    }
};