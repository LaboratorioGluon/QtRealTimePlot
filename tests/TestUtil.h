#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>

// Minimal assertion helpers: count failures and keep going so one run reports everything.
inline int& testFailures()
{
    static int failures = 0;
    return failures;
}

#define CHECK(cond)                                                     \
    do                                                                  \
    {                                                                   \
        if (!(cond))                                                    \
        {                                                               \
            std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
            ++testFailures();                                           \
        }                                                               \
    } while (0)

inline bool nearlyEqual(double a, double b, double rel = 1e-9)
{
    return std::abs(a - b) <= rel * std::max(1.0, std::abs(b));
}

inline int testResult(const char* name)
{
    if (testFailures() == 0)
        std::printf("%s: all checks passed\n", name);
    else
        std::printf("%s: %d check(s) failed\n", name, testFailures());
    return testFailures() == 0 ? 0 : 1;
}
