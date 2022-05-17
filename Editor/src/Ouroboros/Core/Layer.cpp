/************************************************************************************//*!
\file           Layer.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 24, 2021
\brief          Describes a single layer in the application. Think of each layer like a
                general scene where u can fill up. The idea behind having layers
                is to allow control of rendering and updating as one can selectively
                turn on and off a layer.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Layer.h"

namespace oo
{
    Layer::Layer(const std::string& name)
        : m_debugName(name)
    {
    }
}