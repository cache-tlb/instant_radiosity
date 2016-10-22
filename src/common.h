#ifndef COMMON_H
#define COMMON_H


#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <map>
#include <functional>
#include <tuple>
#include <sstream>
#include <regex>
#include <climits>
#include <ctime>
#include <cassert>
#include <random>

typedef std::map<std::string, std::string> Option;

template<typename _Tp>
_Tp parseTo(const std::string &str) {
    std::istringstream iss(str);
    _Tp ret;
    iss >> ret;
    return ret;
}

template<typename _Tp>
std::string toString(const _Tp &x) {
    std::ostringstream oss;
    oss << x;
    return oss.str();
}

template<typename _Tp>
_Tp clamp(const _Tp x, const _Tp low, const _Tp high) {
    return x < low ? low : (x > high ? high : x);
}

template<typename T, typename Tt>
T lerp(const T &a, const T &b, const Tt t) {
    return a*(1 - t) + b*t;
}

template<typename T>
T sqr(const T x) {
    return x*x;
}

#define PI M_PI

#include <QDebug>
#include "OpenGLVersion.h"

#endif // COMMON_H

