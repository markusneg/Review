#include "util.hpp"

#include <functional>
#include <algorithm>

class _Guard
{
public:
    _Guard(std::function<void(void)> on_leave) : m_on_leave(on_leave) {}
    ~_Guard() {m_on_leave();}

private:
    std::function<void(void)> m_on_leave;
};


template <typename T, typename U>
void removeFromVector(T& vector, const U& element)
{
    vector.erase(std::remove(vector.begin(), vector.end(), element), vector.end());
}

template <typename T, typename U>
bool isInContainer(const T& container, const U& element)
{
    return std::find(container.begin(), container.end(), element) != container.end();
}

