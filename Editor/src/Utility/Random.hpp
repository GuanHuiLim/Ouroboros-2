/************************************************************************************//*!
\file           Random.hpp
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

#include "pch.h"

namespace random
{
    static std::random_device s_RandomDevice;
    static std::mt19937_64 s_randomEngine;

    template<typename T>
    static constexpr T generate()
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            // 0 - 1
            std::uniform_real_distribution<T> s_realDistribution{ };
            return s_realDistribution(s_randomEngine);
        }
        else
        {
            // 0 - max
            std::uniform_int_distribution<T> s_intDistribution{ };
            return s_intDistribution(s_randomEngine);
        }
    }

    template<typename T>
    static constexpr T generate(T const& range)
    {
        T min = -range, max = range;
        if (min > max)
        {
            std::swap(min, max);
        }

        if constexpr (std::is_floating_point_v<T>)
        {
            // -range - range
            std::uniform_real_distribution<T> s_realDistribution{ min, max };
            return s_realDistribution(s_randomEngine);
        }
        else
        {
            // -range - range
            std::uniform_int_distribution<T> s_intDistribution{ min, max };
            return s_intDistribution(s_randomEngine);
        }
    }

    template<typename T>
    static constexpr T generate(T const& lower, T const& upper)
    {
        T min = lower, max = upper;
        if (min > max)
        {
            std::swap(min, max);
        }

        if constexpr (std::is_floating_point_v<T>)
        {
            // -range - range
            std::uniform_real_distribution<T> s_realDistribution{ min, max };
            return s_realDistribution(s_randomEngine);
        }
        else
        {
            // -range - range
            std::uniform_int_distribution<T> s_intDistribution{ min, max };
            return s_intDistribution(s_randomEngine);
        }
    }

    template <typename T1, typename T2>
    static constexpr T1 generate(T1 const& lower, T2 const& upper)
    {
        static_assert(std::is_convertible_v<T1, T2>, "second argument can't be converted to first");

        T1 min = lower, max = static_cast<T1>(upper);
        if (min > max)
        {
            std::swap(min, max);
        }

        if constexpr (std::is_floating_point_v<T1>)
        {
            // -range - range
            std::uniform_real_distribution<T1> s_realDistribution{ min, max };
            return s_realDistribution(s_randomEngine);
        }
        else
        {
            // -range - range
            std::uniform_int_distribution<T1> s_intDistribution{ min, max };
            return s_intDistribution(s_randomEngine);
        }
    }

    //std::random_device s_RandomDevice;
    //std::mt19937_64 s_randomEngine{ s_RandomDevice() };

}

