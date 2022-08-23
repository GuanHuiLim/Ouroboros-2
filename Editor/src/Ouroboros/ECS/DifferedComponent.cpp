/************************************************************************************//*!
\file           DifferedComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Aug 23, 2022
\brief          All Components holding onto this component will need to be considered
                differed and will not be updated for the current frame.
                This Component will be removed by the end of this frame by the system.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"
#include "DifferedComponent.h"

#include <rttr/registration>
namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<oo::DifferedComponent>("This is a Differed Object")
            .property("Entity ID", &DifferedComponent::entityID);
    }
    
    DifferedComponent::DifferedComponent() {};

}
