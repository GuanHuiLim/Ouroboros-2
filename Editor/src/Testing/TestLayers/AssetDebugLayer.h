/************************************************************************************//*!
\file           AssetDebugLayer.h
\project        Ouroboros
\author         Chuu Tack Li, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Describes a Test scene used to test The Input Systems
                Functionality and print out debug messages for all supported inputs.

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
        AssetManager manager;
        /*Asset myFile = manager.LoadFile("infile.txt");*/
        Asset myFile = manager.Load(1331783729154L);
        std::ifstream* ifs = myFile.GetData<std::ifstream>();
        while (ifs->good())
        {
            std::cout << static_cast<char>(ifs->get());
        }
        std::cout << myFile.GetHeader().id;
    }

    void OnUpdate() override final
    {

    }
};
