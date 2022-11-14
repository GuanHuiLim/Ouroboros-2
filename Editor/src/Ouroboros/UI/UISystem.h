/************************************************************************************//*!
\file           UISystem.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Nov 09, 2022
\brief          Defines the UI System that is responsible for recalculating and applying
                RectTransform data to the Transform3D component, and for checking
                for and invoking mouse events for interactable UI elements

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Archetypes_Ecs/src/A_Ecs.h>

namespace oo
{
    // forward declaration
    class RectTransform;
    class Scene;

    class UISystem final : public Ecs::System
    {
    public:

        UISystem(Scene* scene);
        virtual ~UISystem() = default;

        /*********************************************************************************//*!
        \brief      Update function called per frame in the Editor Scene. Used to perform
                    the usual actions needed as in Runtime scene, except for processing mouse events
        *//**********************************************************************************/
        void EditorUpdate();

        /*********************************************************************************//*!
        \brief      Update function called per frame in the Runtime Scene to update all RectTransforms
                    and invoke any triggered mouse events for any interactable UI elements
        *//**********************************************************************************/
        void RuntimeUpdate();

        ///*********************************************************************************//*!
        //\brief      immediately recalculates the positional and size data of a given RectTransform
        //            and its children and applies it to the their Transform3D component

        //\param      rect
        //        the RectTransform to update
        //*//**********************************************************************************/
        //void UpdateRectTransformImmediate(RectTransform& rect);

    private:
        //GameObject selectedButton;

        /*********************************************************************************//*!
        \brief      Determines which RectTransforms need to be updated and calling the
                    UpdateRectTransform for the corresponding RectTransforms
        *//**********************************************************************************/
        void UpdateRectTransformAll();

        ///*********************************************************************************//*!
        //\brief      recalculates a RectTransform's positional and size data and applies it
        //            to the RectTransform's Transform3D component

        //\param      rect
        //        the RectTransform to update
        //*//**********************************************************************************/
        //bool UpdateRectTransform(RectTransform& rect);

        ///*********************************************************************************//*!
        //\brief      For all interactable UI elements, check if any mouse events have been triggered
        //            and, if it has, invoke the corresponding C# functions tied to the mouse event
        //*//**********************************************************************************/
        //void UpdateButtonCallbackAll();

        ///*********************************************************************************//*!
        //\brief      For a specific interactable UI element, check if any mouse events have been triggered
        //            and, if it has, invokes the corresponding C# functions tied to the mouse event

        //\param      button
        //        the interactable UI element component
        //\param      isInside
        //        boolean representing if the mouse is within the UI element
        //\param      isUnderElement
        //        boolean representing if another UI element is over this UI element
        //*//**********************************************************************************/
        //bool UpdateButtonCallback(UIButton& button, bool isInside, bool isUnderElement = false);
    
    private:
        Scene* m_scene = nullptr;
    };
}