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

#include "Ouroboros/ECS/ArchtypeECS/A_Ecs.h"
#include "Utility/UUID.h"
#include "Ouroboros/ECS/GameObject.h"
#include "Ouroboros/Geometry/Shapes.h"

class PreviewWindowImageResizeEvent;

namespace oo
{
    // forward declaration
    class TransformComponent;
    class RectTransformComponent;
    class UIRaycastComponent;
    class Scene;
    class GameObject;
    class UIComponent;
    /*class UITextComponent;
    class UIImageComponent;*/


    class UISystem final : public Ecs::System
    {
    public:

        UISystem(GraphicsWorld* graphicsWorld, Scene* scene);
        virtual ~UISystem();

        virtual void Run(Ecs::ECSWorld* world) override {};

        void Init();

        void PostSceneLoadInit();
        void UpdateJustCreated();
        void UpdateDuplicated();
        void UpdateExisting();

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

        inline static bool UIDebugDraw = true;
        inline static bool UIDebugRaycast = false;
        inline static bool UIDebugPrint = false;
    private:
        //GameObject selectedButton;

        /*********************************************************************************//*!
        \brief      Determines which RectTransforms need to be updated and calling the
                    UpdateRectTransform for the corresponding RectTransforms
        *//**********************************************************************************/
        void UpdateRectTransformAll();


        void UpdateIndividualRectTransform(TransformComponent* tf, RectTransformComponent* rect);

        void DebugDrawUI();

        ///*********************************************************************************//*!
        //\brief      For all interactable UI elements, check if any mouse events have been triggered
        //            and, if it has, invoke the corresponding C# functions tied to the mouse event
        //*//**********************************************************************************/
        void UpdateButtonCallbackAll();

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
        bool UpdateButtonCallback(UUID uuid, UIRaycastComponent* raycaster, bool isInside);

        Ray ScreenToWorld(Camera camera, TransformComponent* cameraTf, int32_t mouse_x, int32_t mouse_y);
        

    private:

        void OnUIAssign(Ecs::ComponentEvent<UIComponent>* evnt);
        void OnUIRemove(Ecs::ComponentEvent<UIComponent>* evnt);
        void InitializeUI(UIComponent& uiComp, TransformComponent& tfComp, GameObjectComponent& goComp);

        void OnPreviewWindowImageResize(PreviewWindowImageResizeEvent* e);

        /*void OnTextAssign(Ecs::ComponentEvent<UITextComponent>* evnt);
        void OnTextRemove(Ecs::ComponentEvent<UITextComponent>* evnt);
        void InitializeText(UITextComponent& uiTextComp, TransformComponent& tfComp);
        void OnImageAssign(Ecs::ComponentEvent<UIImageComponent>* evnt);
        void OnImageRemove(Ecs::ComponentEvent<UIImageComponent>* evnt);
        void InitializeImage(UIImageComponent& uiImgComp, TransformComponent& tfComp);*/

    private:
        GraphicsWorld* m_graphicsWorld{ nullptr };

        Scene* m_scene = nullptr;
        GameObject m_prevSelectedUI;

        glm::vec2 m_previewImgStartPos;
        float m_previewImgWidth;
        float m_previewImgHeight;
    };
}