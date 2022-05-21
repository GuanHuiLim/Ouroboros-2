/************************************************************************************//*!
\file           Random.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Nov 8, 2021
\brief          Utility Random class that helps wrap C++ random into one simple to use
                class.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <random>
#include <numeric>

namespace random
{
    // Generates a random number 
    // from 0 - 1 for floats
    // from 0 - max for ints
    template<typename T = float>
    static constexpr T generate();

    // Generates a random number 
    // from -range - range
    template<typename T = float>
    static constexpr T generate(T const& range);
    
    // Generates a random number 
    // from lower - upper
    template<typename T = float>
    static constexpr T generate(T const& lower, T const& upper);
    
    // Overload that takese 2 different types that are convertible to each other
    // Generates a random number 
    // from lower - upper
    template <typename T1, typename T2>
    static constexpr T1 generate(T1 const& lower, T2 const& upper);
};

#include "Random.hpp"
