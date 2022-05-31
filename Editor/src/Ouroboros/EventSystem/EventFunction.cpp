/************************************************************************************//*!
\file           EventFunction.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552 | code contribution (100%)
\par            email: l.guanhui\@digipen.edu
\date           October 31, 2021
\brief          Contains Event function classes that are used by the event system

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "EventFunction.h"

namespace oo
{
    void EventFunctionBase::Execute(event::Event* event)
    {
        Invoke(event);
    }
}
