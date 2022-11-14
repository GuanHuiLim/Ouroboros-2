/************************************************************************************//*!
\file           UISystem.cpp
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
#include "pch.h"
#include "UISystem.h"

#include "Ouroboros/ECS/GameObject.h"
//#include "Ouroboros/Transform/Transform3D.h"
//#include "Ouroboros/ECS/ECS_Manager.h"
//
//#include "Ouroboros/Renderer/Text.h"
//#include "Ouroboros/Renderer/UIRenderingSystem.h"
//#include "Ouroboros/Renderer/Renderer.h"
//
//#include "Ouroboros/Core/Input.h"
//#include "Ouroboros/Renderer/SceneCamera.h"
//#include "Ouroboros/Core/Application.h"
//
//#include "Ouroboros/Geometry/Shapes.h"
//#include "Ouroboros/Geometry/Intersection2D.h"

#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

#include "Ouroboros/Core/Application.h"
#include "Ouroboros/ECS/ECS.h"
#include "RectTransformComponent.h"
#include "UIButtonComponent.h"
#include "UIImageComponent.h"
#include "UICanvasComponent.h"

namespace oo
{
    /*-----------------------------------------------------------------------------*/
    /* Lifecycle Functions                                                         */
    /*-----------------------------------------------------------------------------*/

    UISystem::UISystem(Scene* scene)
        : m_scene{ scene }
    {
    }

    void UISystem::EditorUpdate()
    {
        TRACY_PROFILE_SCOPE_NC(UI_Editor, tracy::Color::Cyan);
        UpdateRectTransformAll();
        TRACY_PROFILE_SCOPE_END();
    }

    void UISystem::RuntimeUpdate()
    {
        TRACY_PROFILE_SCOPE_NC(UI, tracy::Color::DarkCyan);
        UpdateRectTransformAll();
        if (Application::Get().GetWindow().IsFocused())
        {
            //UpdateButtonCallbackAll();
            UpdateRectTransformAll();
        }
        TRACY_PROFILE_SCOPE_END();
    }

    /*void UISystem::UpdateRectTransformImmediate(RectTransform& rect)
    {
        UpdateRectTransform(rect);
        GameObject rectObj{ rect.GetEntity() };
        for (GameObject child : rectObj.GetDirectChilds())
        {
            RectTransform* childRect = child.TryGetComponent<RectTransform>();
            if (childRect != nullptr)
                UpdateRectTransformImmediate(*childRect);
        }
    }*/

    void UISystem::UpdateRectTransformAll()
    {
        // Remember order of update matters!
        // Update canvas here.
        static Ecs::Query canvas_query = Ecs::make_query<UICanvasComponent, RectTransformComponent>();
        m_world->for_each(canvas_query, [&](UICanvasComponent& canvas, RectTransformComponent& rectTransform)
            {
                if (canvas.ScaleWithScreenSize)
                {
                    auto windowSize = Application::Get().GetWindow().GetSize();
                    rectTransform.Size = { windowSize.first, windowSize.second };
                }
            });

        // Update Rect Transform here, order matters : might want to use scenegraph instead.
        /*static Ecs::Query rect_transform_query = Ecs::make_query<RectTransformComponent>();
        m_world->for_each(rect_transform_query, [&](RectTransformComponent& rectTransform)
            {
                UpdateRectTransform(rectTransform);
            });*/

        /*auto canvasView = m_ECS_Manager.GetComponentView<UICanvas>();
        for (auto [canvas] : canvasView)
        {
            if (!canvas.m_dirty)
                continue;

            GameObject canvasObj{ canvas.GetEntity() };
            if (canvas.IsScaledWithScreen())
            {
                auto windowSize = Application::Get().GetWindow().GetSize();
                canvasObj.GetComponent<RectTransform>().SetSize({ windowSize.first, windowSize.second });
            }
            RectTransform* canvasRect = canvasObj.TryGetComponent<RectTransform>();
            if (canvasRect != nullptr)
                UpdateRectTransform(*canvasRect);

            std::vector<Entity> children = canvasObj.GetChildren();
            for (GameObject child : children)
            {
                RectTransform* childRect = child.TryGetComponent<RectTransform>();
                if (childRect != nullptr)
                    UpdateRectTransform(*childRect);
                if (child.HasComponent<UICanvas>())
                {
                    UICanvas& childCanvas = child.GetComponent<UICanvas>();
                    childCanvas.m_dirty = false;
                    childCanvas.m_hasChanged = true;
                    m_ECS_Manager.GetSystem<UIRenderingSystem>()->SignalDirty();
                }
            }
            canvas.m_dirty = false;
            canvas.m_hasChanged = true;
            m_ECS_Manager.GetSystem<UIRenderingSystem>()->SignalDirty();
        }*/
    }

    //bool UISystem::UpdateRectTransform(RectTransform& rect)
    //{
    //    GameObject obj{ rect.GetEntity() };
    //    //LOG_TRACE("RectTransform Update: {0}", obj.Name());

    //    Transform3D& trans = obj.GetComponent<Transform3D>();
    //    // not dirty, no need to update
    //    //if (!rect.m_dirty)
    //    //    return false;

    //    oom::vec3 pos = rect.m_anchoredPosition;

    //    // get parent rectTransform size
    //    GameObject parent{ trans.GetParentId() };
    //    oom::vec2 parentSize{ 0, 0 };
    //    RectTransform* parentRect = parent.TryGetComponent<RectTransform>();
    //    if (parentRect != nullptr)
    //    {
    //        parentSize = parentRect->GetSize();
    //    }

    //    if (parentSize.Length2() > 0)
    //    {
    //        oom::vec2 anchorMin = rect.m_anchorMin;
    //        oom::vec2 anchorMax = rect.m_anchorMax;
    //        oom::vec2 anchorDiff = anchorMax - anchorMin;
    //        oom::vec2 anchor = anchorMin + (0.5f * anchorDiff) - oom::vec2{ 0.5f, 0.5f };
    //        // set size based on anchors
    //        if (fabsf(anchorDiff.x) > 0)
    //        {
    //            rect.m_size.x = anchorDiff.x * parentSize.x;
    //        }
    //        if (fabsf(anchorDiff.y) > 0)
    //        {
    //            rect.m_size.y = anchorDiff.y * parentSize.y;
    //        }
    //        // set position based on anchors
    //        pos += oom::vec3{ anchor.x * parentSize.x, anchor.y * parentSize.y, 0 };
    //    }

    //    // update position based on pivot
    //    oom::vec2 size{ rect.m_size.x * rect.m_scale.x, rect.m_size.y * rect.m_scale.y };
    //    oom::vec2 pivot = rect.m_pivot - oom::vec2{ 0.5f, 0.5f };
    //    oom::vec3 posOffset = oom::vec3{ size.x * pivot.x, size.y * pivot.y, 0 };
    //    float sin = oom::sin(oom::radians(rect.m_angle));
    //    float cos = oom::cos(oom::radians(rect.m_angle));
    //    oom::vec3 rotatedOffset{ (posOffset.x * cos) - (posOffset.y * sin), (posOffset.x * sin) + (posOffset.y * cos), 0 };
    //    pos -= rotatedOffset;

    //    // update transform component
    //    trans.SetPosition(pos);
    //    //trans.SetRotationAngle(rect.GetRotationAngle());
    //    trans.SetScale(rect.GetLocalScale());

    //    // mark rectTransform as no longer dirty
    //    rect.m_dirty = false;
    //    return true;
    //}

    //void UISystem::UpdateButtonCallbackAll()
    //{
    //    if (GameObject::IsValid(selectedButton) && (!selectedButton.ActiveInHierarchy() || !selectedButton.GetComponent<UIButton>().IsInteractable()))
    //    {
    //        UIButton& button = selectedButton.GetComponent<UIButton>();
    //        if (button.IsInteractable())
    //            UpdateButtonCallback(button, false);
    //        selectedButton = GameObject::NOTFOUND;
    //    }

    //    SceneCamera* cam = SceneCamera::MainCamera();
    //    if (cam == nullptr)
    //        return;
    //    oom::vec2 mousePos = cam->MouseToGameScreen(Input::GetMouseX(), Input::GetMouseY());
    //    mousePos.x -= Application::Get().GetWindow().GetWidth() / 2.0f;
    //    mousePos.y -= Application::Get().GetWindow().GetHeight() / 2.0f;
    //    Point2D mouseScreenPoint{ mousePos };

    //    mousePos = cam->ScreenToWorld(Input::GetMouseX(), Input::GetMouseY());
    //    Point2D mouseWorldPoint{ mousePos };

    //    std::vector<std::vector<UICanvas*>> canvasList(OO_MAX_2D_RENDERABLE_LAYERS + 2);
    //    auto& transformDense = m_ECS_Manager.GetComponentDenseArray<Transform3D>();
    //    for (auto iter = transformDense.rbegin(); iter != transformDense.rend(); ++iter)
    //    {
    //        GameObject iterObj{ iter->GetEntity() };
    //        UICanvas* mainCanvas = iter->TryGetComponent<UICanvas>();
    //        if (!iterObj.ActiveInHierarchy() || mainCanvas == nullptr)
    //            continue;
    //        switch (mainCanvas->GetRenderMode())
    //        {
    //        case UICanvas::RenderMode::Overlay:
    //            canvasList[0].push_back(mainCanvas);
    //            break;
    //        case UICanvas::RenderMode::WorldSpace:
    //            canvasList[mainCanvas->GetLayer() + 1UL].push_back(mainCanvas);
    //            break;
    //        }
    //    }

    //    for (std::vector<UICanvas*>& list : canvasList)
    //    {
    //        for (UICanvas* canvas : list)
    //        {
    //            GameObject canvasObject{ canvas->GetEntity() };
    //            std::vector<Entity> childList = canvasObject.GetChildren(true);
    //            for (auto iter = childList.rbegin(); iter != childList.rend(); ++iter)
    //            {
    //                GameObject child = *iter;
    //                if (!child.ActiveInHierarchy())
    //                    continue;

    //                bool hasImage = child.HasComponent<UIImage>() && child.GetComponent<UIImage>().IsActive();
    //                bool hasText = child.HasComponent<Text>() && child.GetComponent<Text>().IsActive();
    //                if (!hasImage && !hasText)
    //                    continue;
    //                if (hasImage && !child.GetComponent<UIImage>().GetRaycastTarget())
    //                    continue;
    //                if (hasText && !child.GetComponent<Text>().GetRaycastTarget())
    //                    continue;

    //                bool mouseOutside = true;
    //                switch (canvas->GetRenderMode())
    //                {
    //                case UICanvas::RenderMode::Overlay:
    //                    mouseOutside = !Intersection2D::PointAABB(mouseScreenPoint, child.GetComponent<RectTransform>().GetAABB2D());
    //                    break;
    //                case UICanvas::RenderMode::WorldSpace:
    //                    mouseOutside = !Intersection2D::PointAABB(mouseWorldPoint, child.GetComponent<RectTransform>().GetAABB2D());
    //                    break;
    //                }
    //                if (mouseOutside)
    //                    continue;

    //                GameObject selected = child.GetEntity();
    //                while (!selected.HasComponent<UICanvas>() && !selected.HasComponent<UIButton>())
    //                {
    //                    selected = selected.GetParent();
    //                }

    //                if (selected == selectedButton)
    //                {
    //                    UpdateButtonCallback(selectedButton.GetComponent<UIButton>(), true);
    //                    return;
    //                }

    //                if (GameObject::IsValid(selectedButton))
    //                {
    //                    UpdateButtonCallback(selectedButton.GetComponent<UIButton>(), false);
    //                    selectedButton = GameObject::NOTFOUND;
    //                }
    //                UIButton* selectedButtonPointer = selected.TryGetComponent<UIButton>();
    //                if (selectedButtonPointer != nullptr && selectedButtonPointer->IsInteractable())
    //                {
    //                    UpdateButtonCallback(*selectedButtonPointer, true);
    //                    selectedButton = selected.GetEntity();
    //                }
    //                return;
    //            }
    //        }
    //    }

    //    if (GameObject::IsValid(selectedButton))
    //    {
    //        UpdateButtonCallback(selectedButton.GetComponent<UIButton>(), false);
    //        selectedButton = GameObject::NOTFOUND;
    //    }
    //}

    //bool UISystem::UpdateButtonCallback(UIButton& button, bool isInside, bool isUnderElement)
    //{
    //    // mouse was previously not in button, and also currently not in button
    //    if (!button.hasEntered && !isInside)
    //        return false;

    //    // mouse was previously not in button, but is now in button
    //    if (!button.hasEntered && isInside)
    //    {
    //        button.hasEntered = true;
    //        button.InvokeButtonEvent("OnPointerEnter");
    //        if (!button.IsInteractable()) // for if OnPointerEnter sets interactable false
    //            return false;
    //        return true;
    //    }
    //    // mouse was previously in button, but is not in button anymore
    //    if (button.hasEntered && !isInside)
    //    {
    //        if (button.isPressed)
    //        {
    //            button.isPressed = false;
    //            button.InvokeButtonEvent("OnRelease");
    //            if (!button.IsInteractable()) // for if OnRelease sets interactable false
    //                return false;
    //        }
    //        button.hasEntered = false;
    //        button.InvokeButtonEvent("OnPointerExit");
    //        return false;
    //    }

    //    // mouse was previously in button, and is still in button

    //    if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft))
    //    {
    //        button.isPressed = true;
    //        button.InvokeButtonEvent("OnPress");
    //        if (!button.IsInteractable()) // for if OnRelease sets interactable false
    //            return false;
    //    }
    //    if (button.isPressed && Input::IsMouseButtonReleased(MouseCode::ButtonLeft))
    //    {
    //        button.InvokeButtonEvent("OnClick");
    //        // isInteractable check since OnClick could disable the button,
    //        //if so OnRelease already invoked, shouldn't be invoked again after OnInteractDisabled
    //        if (!button.IsInteractable())
    //            return false;

    //        button.isPressed = false;
    //        button.InvokeButtonEvent("OnRelease");
    //        if (!button.IsInteractable()) // for if OnRelease sets interactable false
    //            return false;
    //    }
    //    return true;
    //}
}