// A C++ program to do topological sorting of a DAG
// http://www.geeksforgeeks.org/topological-sorting/

#pragma once

#include <vector>
#include <map>


template <typename T>
class Graph
{

public:

    typedef T* ItemPtr;
    typedef std::vector<ItemPtr> ItemList;

    void reset();

    void addConnection(ItemPtr edge, ItemPtr connectedTo);

    void topologicalSort();

    // stored in reverse order!
    const ItemList& getSorted() const;

private:

    struct Edge
    {
        ItemList outputs;
        bool visited = 0;
    };

    std::map<ItemPtr, Edge> m_edges;
    ItemList m_sorted;

    // A function used by topologicalSort
    void topologicalSortUtil(ItemPtr item);

};


#include "Graph.inl"
