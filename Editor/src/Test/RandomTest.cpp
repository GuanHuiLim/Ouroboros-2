/************************************************************************************//*!
\file           RandomTest.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2021
\brief          Describes a function that is used to test the utility random
                functionality

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Utility/Random.h"

void RandomTest()
{
    {
        auto test1 = random::generate();
        auto test2 = random::generate(10.f);
        auto test3 = random::generate(-10.f, 10.f);
        auto test4 = random::generate(-10.f, 10.0);
    }

    {
        float t1 = -10.f, t2 = 10.f;
        double d1 = t1, d2 = t2;

        auto test1 = random::generate();
        auto test2 = random::generate(t1);
        auto test3 = random::generate(t1, t2);
        auto test4 = random::generate(t1, d2);
    }

    {
        struct Wrapper
        {
            float t1 = -10.f, t2 = 10.f;
            double d1 = t1, d2 = t2;
        };

        Wrapper m_wrapper;

        auto test1 = random::generate();
        auto test2 = random::generate(m_wrapper.t1);
        auto test3 = random::generate(m_wrapper.t1, m_wrapper.t2);
        auto test4 = random::generate(m_wrapper.t1, m_wrapper.d2);
    }
}
