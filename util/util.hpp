#pragma once

// used to connect lambdas with overloaded Qt-signals
#define T_SIGNAL(type, fn, arg) static_cast<void (type::*)(arg)>(&type::fn)

// send a message
#define T_COUT(message) std::cout << message << std::endl << std::flush

// run a function on leaving scope
#define ON_LEAVE(fn)    _Guard _guard1([&](){fn});
#define ON_LEAVE2(fn)   _Guard _guard2([&](){fn});

template <typename T, typename U>
void removeFromVector(T& vector, const U& element);

template <typename T, typename U>
bool isInContainer(const T& container, const U& element);

#include "util.inl"
