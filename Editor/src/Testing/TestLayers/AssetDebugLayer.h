/************************************************************************************//*!
\file           AssetDebugLayer.h
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Aug 30, 2022
\brief          Describes a Test scene used to test the Asset system.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

// ^ DO LATER

#pragma once

#include <Ouroboros/Asset/AssetManager.h>

/****************************************************************************//*!
 @brief     Describes a Test scene used to test The Input Systems
            Functionality and print out debug messages for all supported inputs.
*//*****************************************************************************/
class AssetDebugLayer final : public oo::Layer
{
public:
    AssetDebugLayer() : Layer("AssetDebugLayer")
    {
    }

    void OnAttach() override final
    {
    }

    void OnUpdate() override final
    {
        if (oo::input::IsAnyKeyPressed())
        {
        }
    }
};
