#include "Graph.hpp"

template <typename T>
void Graph<T>::topologicalSortUtil(ItemPtr item)
{
    // Mark the current node as visited.
    m_edges[item].visited = true;

    for(ItemPtr nextItem : m_edges[item].outputs)
        if(!m_edges[nextItem].visited)
            topologicalSortUtil(nextItem);

    // Push current vertex to stack which stores result
    m_sorted.push_back(item);
}

template <typename T>
void Graph<T>::reset()
{
    m_edges.clear(); m_sorted.clear();
}

template <typename T>
void Graph<T>::addConnection(ItemPtr edge, ItemPtr connectedTo)
{
    /// [TODO] check existing connections

    m_edges[edge].outputs.push_back(connectedTo);
}

template <typename T>
void Graph<T>::topologicalSort()
{
    for(std::pair<ItemPtr const, Edge>& edge : m_edges)
        edge.second.visited = false;

    m_sorted.clear();

    // Call the recursive helper function to store Topological. Sort starting from all vertices one by one
    typename std::map<ItemPtr, Edge>::iterator it;
    for(it = m_edges.begin(); it != m_edges.end(); ++it)
      if(it->second.visited == false)
        topologicalSortUtil(it->first);
}

template <typename T>
const typename Graph<T>::ItemList& Graph<T>::getSorted() const {return m_sorted;}
