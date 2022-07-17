/************************************************************************************//*!
\file           InputDebugLayer.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2021
\brief          Describes a Test scene used to test The Input Systems
                Functionality and print out debug messages for all supported inputs.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Ouroboros/Core/Layer.h>
#include <Ouroboros/Core/Input.h>
#include <Ouroboros/Core/Assert.h>
#include <Ouroboros/Core/Log.h>
#include <Ouroboros/Core/KeyCode.h>

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
        // Commenting this out for now until oo::input::GetMouseDelta() no longer consumes the information
        /*oo::input::GetMouseDelta();
        std::pair<int, int> pos = oo::input::GetMouseDelta();
        LOG_INFO("{0}, {1}", pos.first , pos.second);*/


        // New way to do check keys.
        if (oo::input::IsKeyPressed(KEY_0))
        {
            LOG_TRACE("key 0 Pressed ");
        }

        if (oo::input::IsAnyKeyHeld())
        {
            for (oo::input::KeyCode key : oo::input::GetKeysHeld())
            {
                LOG_TRACE("oo::input::Key {0} down", key);
            }
        }

        if (oo::input::IsAnyKeyPressed())
        {
            for (oo::input::KeyCode key : oo::input::GetKeysPressed())
            {
                LOG_TRACE("oo::input::Key {0} pressed", key);
            }
        }

        if (oo::input::IsAnyKeyReleased())
        {
            for (oo::input::KeyCode key : oo::input::GetKeysReleased())
            {
                LOG_TRACE("oo::input::Key {0} released", key);
            }
        }

        if (oo::input::IsAnyMouseButtonHeld())
        {
            for (oo::input::MouseCode mousecode : oo::input::GetMouseButtonsHeld())
            {
                LOG_TRACE("oo::input::Mouse Button {0} Down", mousecode);
            }
        }

        if (oo::input::IsAnyMouseButtonPressed())
        {
            for (oo::input::MouseCode mousecode : oo::input::GetMouseButtonsPressed())
            {
                LOG_TRACE("oo::input::Mouse Button {0} Pressed", mousecode);
            }
        }

        if (oo::input::IsAnyMouseButtonReleased())
        {
            for (oo::input::MouseCode mousecode : oo::input::GetMouseButtonsReleased())
            {
                LOG_TRACE("oo::input::Mouse Button {0} Released", mousecode);
            }
        }

        if (oo::input::IsAnyControllerButtonHeld())
        {
            for (oo::input::ControllerButtonCode controllerBtnCode : oo::input::GetControllerButtonsHeld())
            {
                LOG_TRACE("Controller Button {0} Down", controllerBtnCode);
            }
        }

        if (oo::input::IsAnyControllerButtonPressed())
        {
            for (oo::input::ControllerButtonCode controllerBtnCode : oo::input::GetControllerButtonsPressed())
            {
                LOG_TRACE("Controller Button {0} Pressed", controllerBtnCode);
            }
        }

        if (oo::input::IsAnyControllerButtonReleased())
        {
            for (oo::input::ControllerButtonCode controllerBtnCode : oo::input::GetControllerButtonsReleased())
            {
                LOG_TRACE("Controller Button {0} Released", controllerBtnCode);
            }
        }

        if (oo::input::IsAnyControllerAxis())
        {
            for (auto&& [controllerAxis, value] : oo::input::GetControllerAxis())
            {
                LOG_TRACE("Active Controller Axis {0} value {1}", controllerAxis, value);
            }
        }


        if (oo::input::IsKeyHeld(oo::input::Key::W))
        {
            LOG_TRACE("key W down!");
        }
        if (oo::input::IsKeyPressed(oo::input::Key::W))
        {
            LOG_TRACE("key W pressed!");
        }
        if (oo::input::IsKeyReleased(oo::input::Key::W))
        {
            LOG_TRACE("key W released!");
        }

        if (oo::input::IsMouseButtonHeld(oo::input::Mouse::ButtonLeft))
        {
            LOG_TRACE("mouse button Left is Down!");
        }
        if (oo::input::IsMouseButtonPressed(oo::input::Mouse::ButtonLeft))
        {
            LOG_TRACE("mouse button Left is Pressed!");
        }
        if (oo::input::IsMouseButtonReleased(oo::input::Mouse::ButtonLeft))
        {
            LOG_TRACE("mouse button Left is Released!");
        }

        if (oo::input::IsMouseButtonHeld(oo::input::Mouse::ButtonRight))
        {
            LOG_TRACE("mouse button Right is Down!");
        }
        if (oo::input::IsMouseButtonPressed(oo::input::Mouse::ButtonRight))
        {
            LOG_TRACE("mouse button Right is Pressed!");
        }
        if (oo::input::IsMouseButtonReleased(oo::input::Mouse::ButtonRight))
        {
            LOG_TRACE("mouse button Right is Released!");
        }


        if (oo::input::IsMouseButtonHeld(oo::input::Mouse::ButtonMiddle))
        {
            LOG_TRACE("mouse button Middle is Down!");
        }
        if (oo::input::IsMouseButtonPressed(oo::input::Mouse::ButtonMiddle))
        {
            LOG_TRACE("mouse button Middle is Pressed!");
        }
        if (oo::input::IsMouseButtonReleased(oo::input::Mouse::ButtonMiddle))
        {
            LOG_TRACE("mouse button Middle is Released!");
        }

        if (oo::input::IsMouseButtonHeld(oo::input::Mouse::ButtonLast))
        {
            LOG_TRACE("mouse button Last is Down!");
        }
        if (oo::input::IsMouseButtonPressed(oo::input::Mouse::ButtonLast))
        {
            LOG_TRACE("mouse button Last is Pressed!");
        }
        if (oo::input::IsMouseButtonReleased(oo::input::Mouse::ButtonLast))
        {
            LOG_TRACE("mouse button Last is Released!");
        }

        //LOG_TRACE("{0}, {1}", oo::input::GetMousePosition().first, oo::input::GetMousePosition().second);
        //LOG_TRACE("{0}, {1}", oo::input::GetMouseX(), oo::input::GetMouseY());

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