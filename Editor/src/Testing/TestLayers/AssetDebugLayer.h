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
            static AssetManager manager;
            try
            {
                //Asset myFile = manager.LoadFile("assets/infile.txt");
                Asset myFile = manager.Load(2509311311874L);
                std::ifstream* ifs = myFile.GetData<std::ifstream>();
                ifs->clear();
                ifs->seekg(0);
                while (ifs->good())
                {
                    std::cout << static_cast<char>(ifs->get());
                }
                std::cout << myFile.GetHeader().id;
            }
            catch (...)
            {
                std::cout << "not found\n";
            }
        }
    }
};
