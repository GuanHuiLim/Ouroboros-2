/************************************************************************************//*!
\file           InputDebugLayer.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2021
\brief          Describes a Test scene used to test The Input Systems
                Functionality and print out debug messages for all supported inputs.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/Core/Layer.h"
#include "Ouroboros/Core/Input.h"
#include "Ouroboros/Core/Assert.h"
#include "Ouroboros/Core/Log.h"
#include "Ouroboros/Core/KeyCode.h"

/****************************************************************************//*!
 @brief     Describes a Test scene used to test The Input Systems
            Functionality and print out debug messages for all supported inputs.
*//*****************************************************************************/
class InputDebugLayer final : public oo::Layer
{
public:
    InputDebugLayer() : Layer("InputDebugLayer")
    {
    }

    void OnUpdate() override final
    {
        //LOG_INFO("ExampleLayer::Update {0}s {1}ms", dt.GetSeconds(), dt.GetMilliSeconds());
        // Commenting this out for now until input::GetMouseDelta() no longer consumes the information
        /*input::GetMouseDelta();
        std::pair<int, int> pos = input::GetMouseDelta();
        LOG_INFO("{0}, {1}", pos.first , pos.second);*/


        // New way to do check keys.
        if (input::IsKeyPressed(KEY_0))
        {
            LOG_TRACE("key 0 Pressed ");
        }

        if (input::IsAnyKeyHeld())
        {
            for (input::KeyCode key : input::GetKeysHeld())
            {
                LOG_TRACE("input::Key {0} down", key);
            }
        }

        if (input::IsAnyKeyPressed())
        {
            for (input::KeyCode key : input::GetKeysPressed())
            {
                LOG_TRACE("input::Key {0} pressed", key);
            }
        }

        if (input::IsAnyKeyReleased())
        {
            for (input::KeyCode key : input::GetKeysReleased())
            {
                LOG_TRACE("input::Key {0} released", key);
            }
        }

        if (input::IsAnyMouseButtonHeld())
        {
            for (input::MouseCode mousecode : input::GetMouseButtonsHeld())
            {
                LOG_TRACE("input::Mouse Button {0} Down", mousecode);
            }
        }

        if (input::IsAnyMouseButtonPressed())
        {
            for (input::MouseCode mousecode : input::GetMouseButtonsPressed())
            {
                LOG_TRACE("input::Mouse Button {0} Pressed", mousecode);
            }
        }

        if (input::IsAnyMouseButtonReleased())
        {
            for (input::MouseCode mousecode : input::GetMouseButtonsReleased())
            {
                LOG_TRACE("input::Mouse Button {0} Released", mousecode);
            }
        }

        if (input::IsAnyControllerButtonHeld())
        {
            for (input::ControllerButtonCode controllerBtnCode : input::GetControllerButtonsHeld())
            {
                LOG_TRACE("Controller Button {0} Down", controllerBtnCode);
            }
        }

        if (input::IsAnyControllerButtonPressed())
        {
            for (input::ControllerButtonCode controllerBtnCode : input::GetControllerButtonsPressed())
            {
                LOG_TRACE("Controller Button {0} Pressed", controllerBtnCode);
            }
        }

        if (input::IsAnyControllerButtonReleased())
        {
            for (input::ControllerButtonCode controllerBtnCode : input::GetControllerButtonsReleased())
            {
                LOG_TRACE("Controller Button {0} Released", controllerBtnCode);
            }
        }

        if (input::IsAnyControllerAxis())
        {
            for (auto&& [controllerAxis, value] : input::GetControllerAxis())
            {
                LOG_TRACE("Active Controller Axis {0} value {1}", controllerAxis, value);
            }
        }


        if (input::IsKeyHeld(input::Key::W))
        {
            LOG_TRACE("key W down!");
        }
        if (input::IsKeyPressed(input::Key::W))
        {
            LOG_TRACE("key W pressed!");
        }
        if (input::IsKeyReleased(input::Key::W))
        {
            LOG_TRACE("key W released!");
        }

        if (input::IsMouseButtonHeld(input::Mouse::ButtonLeft))
        {
            LOG_TRACE("mouse button Left is Down!");
        }
        if (input::IsMouseButtonPressed(input::Mouse::ButtonLeft))
        {
            LOG_TRACE("mouse button Left is Pressed!");
        }
        if (input::IsMouseButtonReleased(input::Mouse::ButtonLeft))
        {
            LOG_TRACE("mouse button Left is Released!");
        }

        if (input::IsMouseButtonHeld(input::Mouse::ButtonRight))
        {
            LOG_TRACE("mouse button Right is Down!");
        }
        if (input::IsMouseButtonPressed(input::Mouse::ButtonRight))
        {
            LOG_TRACE("mouse button Right is Pressed!");
        }
        if (input::IsMouseButtonReleased(input::Mouse::ButtonRight))
        {
            LOG_TRACE("mouse button Right is Released!");
        }


        if (input::IsMouseButtonHeld(input::Mouse::ButtonMiddle))
        {
            LOG_TRACE("mouse button Middle is Down!");
        }
        if (input::IsMouseButtonPressed(input::Mouse::ButtonMiddle))
        {
            LOG_TRACE("mouse button Middle is Pressed!");
        }
        if (input::IsMouseButtonReleased(input::Mouse::ButtonMiddle))
        {
            LOG_TRACE("mouse button Middle is Released!");
        }

        if (input::IsMouseButtonHeld(input::Mouse::ButtonLast))
        {
            LOG_TRACE("mouse button Last is Down!");
        }
        if (input::IsMouseButtonPressed(input::Mouse::ButtonLast))
        {
            LOG_TRACE("mouse button Last is Pressed!");
        }
        if (input::IsMouseButtonReleased(input::Mouse::ButtonLast))
        {
            LOG_TRACE("mouse button Last is Released!");
        }

        //LOG_TRACE("{0}, {1}", input::GetMousePosition().first, input::GetMousePosition().second);
        //LOG_TRACE("{0}, {1}", input::GetMouseX(), input::GetMouseY());

    }

    //void OnEvent(Event& e) override final
    //{
    //    //LOG_TRACE("{0}", e);
    //    if (e.GetEventType() == EVENT_TYPE::MOUSEMOVED)
    //    {
    //        //LOG_TRACE("{0}", e);
    //    }
    //    /*if (e.GetEventType() == EVENT_TYPE::CONTROLLERMOVED)
    //    {
    //        LOG_TRACE(e.ToString());
    //    }*/
    //}
};