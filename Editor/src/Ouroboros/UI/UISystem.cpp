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

#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

#include "Ouroboros/Core/Application.h"
#include "Ouroboros/ECS/ECS.h"
#include "RectTransformComponent.h"
#include "UIRaycastComponent.h"
#include "UIImageComponent.h"
#include "UICanvasComponent.h"
#include "GraphicsRaycasterComponent.h"
#include "UIComponent.h"
#include "UITextComponent.h"

#include <OO_Vulkan/src/DebugDraw.h>
#include "Ouroboros/Core/Input.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "Ouroboros/Vulkan/CameraComponent.h"

#include "Ouroboros/Geometry/Algorithms.h"
#include "Ouroboros/EventSystem/EventTypes.h"

namespace oo
{
    /*-----------------------------------------------------------------------------*/
    /* Lifecycle Functions                                                         */
    /*-----------------------------------------------------------------------------*/

    UISystem::UISystem(GraphicsWorld* graphicsWorld, Scene* scene)
        : m_graphicsWorld { graphicsWorld }
        , m_scene{ scene }
    {
        assert(graphicsWorld != nullptr);	// it should never be nullptr, who's calling this?
    }

    UISystem::~UISystem()
    {
        EventManager::Unsubscribe<UISystem, PreviewWindowImageResizeEvent>(this, &UISystem::OnPreviewWindowImageResize);
    }

    void UISystem::Init()
    {
        // UI Component
        m_world->SubscribeOnAddComponent<UISystem, UIComponent>(
            this, &UISystem::OnUIAssign);
        m_world->SubscribeOnRemoveComponent<UISystem, UIComponent>(
            this, &UISystem::OnUIRemove);

        // we initilize preview window size once.
        /*{
            GetPreviewWindowSizeEvent e;
            EventManager::Broadcast<GetPreviewWindowSizeEvent>(&e);
            m_previewImgStartPos = e.StartPosition;
            m_previewImgWidth = e.Width;
            m_previewImgHeight = e.Height;
        }*/

        // we make sure to keep preview window size updated
        EventManager::Subscribe<UISystem, PreviewWindowImageResizeEvent>(this, &UISystem::OnPreviewWindowImageResize);

        //// UI Component
        //m_world->SubscribeOnAddComponent<UISystem, UITextComponent>(
        //    this, &UISystem::OnTextAssign);
        //m_world->SubscribeOnRemoveComponent<UISystem, UITextComponent>(
        //    this, &UISystem::OnTextRemove);


        //// UI Component
        //m_world->SubscribeOnAddComponent<UISystem, UIImageComponent>(
        //    this, &UISystem::OnImageAssign);
        //m_world->SubscribeOnRemoveComponent<UISystem, UIImageComponent>(
        //    this, &UISystem::OnImageRemove);


    }

    void UISystem::PostSceneLoadInit()
    {
        // Update UI positions immediately!
        static Ecs::Query ui_query = Ecs::make_raw_query<UIComponent, TransformComponent>();
        m_world->parallel_for_each(ui_query, [&](UIComponent& uiComp, TransformComponent& tfComp)
            {
                auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);
                ui.entityID = uiComp.PickingID;
                ui.localToWorld = tfComp.GetGlobalMatrix();
            });

        // Update UIText 
        static Ecs::Query ui_text_query = Ecs::make_raw_query<UIComponent, TransformComponent, UITextComponent>();
        m_world->parallel_for_each(ui_text_query, [&](UIComponent& uiComp, TransformComponent& tfComp, UITextComponent& uiTextComp)
            {
                auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);
                ui.SetText(true);
            });

        // Update UIImage 
        static Ecs::Query ui_img_query = Ecs::make_raw_query<UIComponent, TransformComponent, UIImageComponent>();
        m_world->parallel_for_each(ui_img_query, [&](UIComponent& uiComp, TransformComponent& tfComp, UIImageComponent& uiImgComp)
            {
                auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);
                ui.SetText(false);
            });
    }

    void UISystem::UpdateJustCreated()
    {
        // Update UI
        static Ecs::Query ui_query = Ecs::make_query<UIComponent, TransformComponent, RectTransformComponent, JustCreatedComponent>();
        m_world->parallel_for_each(ui_query, [&](UIComponent& uiComp, TransformComponent& transformComp, RectTransformComponent& rectTfComp, JustCreatedComponent& jcComp)
            {
                auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);

                ui.entityID = uiComp.PickingID;
                ui.localToWorld = transformComp.GetGlobalMatrix();
            });

        // Update Text UI
        static Ecs::Query text_ui_query = Ecs::make_query<UIComponent, TransformComponent, UITextComponent, RectTransformComponent, JustCreatedComponent>();
        m_world->parallel_for_each(text_ui_query, [&](UIComponent& uiComp, TransformComponent& transformComp, UITextComponent& uiTextComp, RectTransformComponent& rectTfComp, JustCreatedComponent& jcComp)
            {
                auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);
                ui.SetText(true);
            });

        // Update Image UI
        static Ecs::Query image_ui_query = Ecs::make_query<UIComponent, TransformComponent, UIImageComponent, RectTransformComponent, JustCreatedComponent>();
        m_world->parallel_for_each(image_ui_query, [&](UIComponent& uiComp, TransformComponent& transformComp, UIImageComponent& uiImageComp, RectTransformComponent& rectTfComp, JustCreatedComponent& jcComp)
            {
                auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);
                ui.SetText(false);
            });
    }

    void UISystem::UpdateDuplicated()
    {
        // Update Newly Duplicated UI
        static Ecs::Query duplicated_ui_query = Ecs::make_raw_query<UIComponent, TransformComponent, GameObjectComponent, DuplicatedComponent>();
        m_world->for_each(duplicated_ui_query, [&](UIComponent& uiComp, TransformComponent& transformComp, GameObjectComponent& goComp, DuplicatedComponent& dupComp)
            {
                InitializeUI(uiComp, transformComp, goComp);
            });

        // Update Newly Duplicated Text UI
        static Ecs::Query text_ui_query = Ecs::make_query<UIComponent, TransformComponent, UITextComponent, RectTransformComponent, DuplicatedComponent>();
        m_world->parallel_for_each(text_ui_query, [&](UIComponent& uiComp, TransformComponent& transformComp, UITextComponent& uiTextComp, RectTransformComponent& rectTfComp, DuplicatedComponent& dupComp)
            {
                auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);
                ui.SetText(true);
            });

        // Update Newly Duplicated Image UI
        static Ecs::Query image_ui_query = Ecs::make_query<UIComponent, TransformComponent, UIImageComponent, RectTransformComponent, DuplicatedComponent>();
        m_world->parallel_for_each(image_ui_query, [&](UIComponent& uiComp, TransformComponent& transformComp, UIImageComponent& uiImageComp, RectTransformComponent& rectTfComp, DuplicatedComponent& dupComp)
            {
                auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);
                ui.SetText(false);
            });

    }

    void UISystem::UpdateExisting()
    {
        // Update UI
        static Ecs::Query ui_query = Ecs::make_query<UIComponent, TransformComponent, RectTransformComponent>();
        m_world->parallel_for_each(ui_query, [&](UIComponent& uiComp, TransformComponent& transformComp, RectTransformComponent& rectTfComp)
            {
                auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);

                if (transformComp.HasChangedThisFrame)
                {
                    ui.localToWorld = transformComp.GetGlobalMatrix();
                }

                if (rectTfComp.HasChanged)
                {
                    glm::vec3 Center = rectTfComp.BoundingVolume.Center;
                    
                    glm::vec2 HalfSize = rectTfComp.Size * 0.5f;

                    ui.format.box.min = glm::vec2{ Center } - HalfSize;
                    ui.format.box.max = glm::vec2{ Center } + HalfSize;
                }
                    
            });

        // Update Text UI
        static Ecs::Query text_ui_query = Ecs::make_query<UIComponent, TransformComponent, UITextComponent, RectTransformComponent>();
        m_world->parallel_for_each(text_ui_query, [&](UIComponent& uiComp, TransformComponent& transformComp, UITextComponent& uiTextComp, RectTransformComponent& rectTfComp)
            {
                auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);
                ui.textData = uiTextComp.Text;
                ui.colour = uiTextComp.TextColor;

                ui.format.fontSize = uiTextComp.FontSize;
                ui.format.alignment = static_cast<oGFX::FontAlignment>(uiTextComp.Alignment);
                ui.format.verticalLineSpace = uiTextComp.VerticalLineSpace;

                ui.fontAsset = uiTextComp.font;
            });
        
        // Update Image UI
        static Ecs::Query image_ui_query = Ecs::make_query<UIComponent, TransformComponent, UIImageComponent, RectTransformComponent>();
        m_world->parallel_for_each(image_ui_query, [&](UIComponent& uiComp, TransformComponent& transformComp, UIImageComponent& uiImageComp, RectTransformComponent& rectTfComp)
            {
                auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);
                ui.colour = uiImageComp.Tint;
                ui.bindlessGlobalTextureIndex_Albedo = uiImageComp.AlbedoID;
            });
    }


    void UISystem::EditorUpdate()
    {
        TRACY_PROFILE_SCOPE_NC(UISystem_EditorUpdate, tracy::Color::Cyan);
        OPTICK_EVENT();
        
            TRACY_PROFILE_SCOPE_NC(UISystem_UpdateJustCreated, tracy::Color::Cyan);
            UpdateJustCreated();
            TRACY_PROFILE_SCOPE_END(); 
        
            TRACY_PROFILE_SCOPE_NC(UISystem_UpdateDuplicated, tracy::Color::Cyan);
            UpdateDuplicated();
            TRACY_PROFILE_SCOPE_END(); 
        
            TRACY_PROFILE_SCOPE_NC(UISystem_UpdateExisting, tracy::Color::Cyan);
            UpdateExisting();
            TRACY_PROFILE_SCOPE_END();

            TRACY_PROFILE_SCOPE_NC(UISystem_UpdateRectTransform, tracy::Color::Cyan);
            UpdateRectTransformAll();
            TRACY_PROFILE_SCOPE_END();

            TRACY_PROFILE_SCOPE_NC(UISystem_DebugDrawUI, tracy::Color::Cyan);
            DebugDrawUI();
            TRACY_PROFILE_SCOPE_END();

        TRACY_PROFILE_SCOPE_END();
    }

    void UISystem::RuntimeUpdate()
    {
        TRACY_PROFILE_SCOPE_NC(UISystem_RuntimeUpdate, tracy::Color::DarkCyan);
        OPTICK_EVENT();

        TRACY_PROFILE_SCOPE_NC(UISystem_UpdateJustCreated, tracy::Color::DarkCyan);
        UpdateJustCreated();
        TRACY_PROFILE_SCOPE_END();

        TRACY_PROFILE_SCOPE_NC(UISystem_UpdateDuplicated, tracy::Color::DarkCyan);
        UpdateDuplicated();
        TRACY_PROFILE_SCOPE_END();

        TRACY_PROFILE_SCOPE_NC(UISystem_UpdateExisting, tracy::Color::DarkCyan);
        UpdateExisting();
        TRACY_PROFILE_SCOPE_END();

        TRACY_PROFILE_SCOPE_NC(UISystem_UpdateRectTransformAll, tracy::Color::DarkCyan);
        UpdateRectTransformAll();
        TRACY_PROFILE_SCOPE_END();

        if (Application::Get().GetWindow().IsFocused())
        {

            TRACY_PROFILE_SCOPE_NC(UISystem_UpdateButtonCallback, tracy::Color::DarkCyan);
            UpdateButtonCallbackAll();
            TRACY_PROFILE_SCOPE_END();

            TRACY_PROFILE_SCOPE_NC(UISystem_UpdateRectTransformAll_2, tracy::Color::DarkCyan);
            UpdateRectTransformAll();
            TRACY_PROFILE_SCOPE_END();
            
            TRACY_PROFILE_SCOPE_NC(UISystem_DebugDrawUI, tracy::Color::DarkCyan);
            DebugDrawUI();
            TRACY_PROFILE_SCOPE_END();
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
        // Remember order of update matters! (both order between calls and order internally!)

        // Update canvas here. order does matter here. Assumes all rect transform has been updated already.
        static Ecs::Query canvas_query = Ecs::make_query<GameObjectComponent, TransformComponent, UICanvasComponent, RectTransformComponent>();
        m_world->parallel_for_each(canvas_query, [&](GameObjectComponent& goc, TransformComponent& tf, UICanvasComponent& canvas, RectTransformComponent& rectTransform)
            {

                if (canvas.ScaleWithScreenSize)
                {
                    auto windowSize = Application::Get().GetWindow().GetSize();
                    rectTransform.Size = { windowSize.first, windowSize.second };
                }

                
                // retrieve canvas gameobject
                auto go = m_scene->FindWithInstanceID(goc.Id);
                bool CanvasIsWorldSpace = canvas.RenderingMode == UICanvasComponent::RenderMode::WorldSpace;
                //bool CanvasIsCanvasSpace = canvas.RenderingMode == UICanvasComponent::RenderMode::CanvasSpace;
                for (auto& child : go->GetChildren(true))
                {
                    // skip all children that does not have rect transform
                    if (child.HasComponent<RectTransformComponent>() == false)
                        continue;
                    
                    RectTransformComponent& childRect = child.GetComponent<RectTransformComponent>();
                    childRect.IsWorldSpace = CanvasIsWorldSpace;
                    
                    if (child.HasComponent<UIComponent>())
                    {
                        auto& uiComp = child.GetComponent<UIComponent>();
                        auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);
                        ui.SetScreenSpace(canvas.RenderOnTop);
                    }

                    auto parent = child.GetParent();
                    if (parent.HasComponent<RectTransformComponent>())
                    {
                        glm::vec2 parentSize = parent.GetComponent<RectTransformComponent>().Size;

                        if (glm::length2(parentSize) > 0.f)
                        {
                            glm::vec2 anchorMin = childRect.AnchorMin;
                            glm::vec2 anchorMax = childRect.AnchorMax;
                            glm::vec2 anchorDiff = anchorMax - anchorMin;
                            glm::vec2 anchor = anchorMin + (0.5f * anchorDiff) - glm::vec2{ 0.5f, 0.5f };
                            // set size based on anchors
                            if (fabsf(anchorDiff.x) > 0)
                            {
                                childRect.Size.x = anchorDiff.x * parentSize.x;
                            }
                            if (fabsf(anchorDiff.y) > 0)
                            {
                                childRect.Size.y = anchorDiff.y * parentSize.y;
                            }
                            // cache and set parent Offset used later in individual update
                            childRect.ParentOffset = glm::vec3{ anchor.x * parentSize.x, anchor.y * parentSize.y, 0 };
                        }
                    }
                }

            });

        // Update Individual Rect Transform here order of update between each other does not matter here.
        static Ecs::Query rect_transform_query = Ecs::make_query<TransformComponent, RectTransformComponent>();
        m_world->parallel_for_each(rect_transform_query, [&](TransformComponent& tf, RectTransformComponent& rectTransform)
            {
                if (rectTransform.IsDirty)
                {
                    UpdateIndividualRectTransform(&tf, &rectTransform);
                    // mark rectTransform as no longer dirty
                    rectTransform.IsDirty = false;
                    rectTransform.HasChanged = true;
                }

                rectTransform.BoundingVolume.Center       = tf.GetGlobalPosition();
                rectTransform.BoundingVolume.Orientation  = tf.GetGlobalRotationQuat();
                rectTransform.BoundingVolume.HalfExtents  = tf.GetGlobalScale()  * (glm::vec3{ rectTransform.Size, 1.0 } * 0.5f);
            });

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

    void UISystem::UpdateIndividualRectTransform(TransformComponent* tf, RectTransformComponent* rect)
    {
        if (rect->EditGlobal)
        {
            rect->AnchoredPosition = tf->GetPosition();
            rect->EulerAngles = glm::degrees(glm::eulerAngles(tf->GetRotationQuat().value));
            rect->Scale = tf->GetScale();
            rect->EditGlobal = false;
        }
        else
        {
            // assumes parent Offset is properly set prior.
            glm::vec3 pos = rect->AnchoredPosition + rect->ParentOffset;

            // update transform local data
            tf->SetPosition(pos);
            tf->SetRotation(rect->EulerAngles);
            tf->SetScale(rect->Scale);
        }
    }

    void UISystem::DebugDrawUI()
    {
        if (UIDebugDraw == false)
            return;

        static Ecs::Query rect_transform_query = Ecs::make_query<TransformComponent, RectTransformComponent>();
        m_world->for_each(rect_transform_query, [&](TransformComponent& tf, RectTransformComponent& rectTransform)
            {
                if (rectTransform.IsWorldSpace == false)
                    return;
                
                // Only draw World Space UI Debug Draws for now..

                glm::vec3 Center = rectTransform.BoundingVolume.Center; //tf.GetGlobalPosition();
                //glm::vec3 GlobalScale = tf.GetGlobalScale();
                glm::quat GlobalQuat = rectTransform.BoundingVolume.Orientation; // tf.GetGlobalRotationQuat().value;
                glm::vec3 HalfExtents = rectTransform.BoundingVolume.HalfExtents; //GlobalScale * (glm::vec3{rectTransform.Size, 0} * 0.5f);
                
                glm::vec3 HalfExtentX = glm::rotate(GlobalQuat, glm::vec3{HalfExtents.x, 0, 0});
                glm::vec3 HalfExtentY = glm::rotate(GlobalQuat, glm::vec3{0, HalfExtents.y, 0});

                glm::vec3 bottom_left = Center - HalfExtentX - HalfExtentY;
                glm::vec3 top_right = Center + HalfExtentX + HalfExtentY;
                glm::vec3 top_left = Center - HalfExtentX + HalfExtentY;
                glm::vec3 bottom_right = Center + HalfExtentX -HalfExtentY;
                
                oGFX::DebugDraw::AddLine(top_left, bottom_left);
                oGFX::DebugDraw::AddLine(bottom_left, bottom_right);
                oGFX::DebugDraw::AddLine(bottom_right, top_right);
                oGFX::DebugDraw::AddLine(top_right, top_left);
            });
    }

    void UISystem::UpdateButtonCallbackAll()
    {
        if (m_scene->IsValid(m_prevSelectedUI) && !m_prevSelectedUI.ActiveInHierarchy())
        {
            UpdateButtonCallback(m_prevSelectedUI.GetInstanceID(), &m_prevSelectedUI.GetComponent<UIRaycastComponent>(), false);
            m_prevSelectedUI = GameObject{}; //invalid gameobject
        }
        
        auto camera = m_scene->GetMainCameraObject();
        if (camera == nullptr)
            return;

        int32_t mouseX = oo::input::GetMouseX();
        int32_t mouseY = oo::input::GetMouseY();
#ifdef OO_EDITOR
        auto [windowSizeX, windowSizeY] = oo::Application::Get().GetWindow().GetSize();
        // we need to do extra conversion if we are in editor mode
        float intermediateX = mouseX - m_previewImgStartPos.x;
        intermediateX /= static_cast<float>(m_previewImgWidth);
        float intermediateY = mouseY - m_previewImgStartPos.y;
        intermediateY /= static_cast<float>(m_previewImgHeight);
        intermediateX *= windowSizeX;
        intermediateY *= windowSizeY;
        //LOG_CORE_INFO("actual mouse pos {0},{1}, mapped mouse pos {2},{3}", mouseX, mouseY, intermediateX, intermediateY);
        mouseX = static_cast<int32_t>(intermediateX);
        mouseY = static_cast<int32_t>(intermediateY);
#endif
        Ray mouseWorldRay = ScreenToWorld(m_scene->MainCamera(), &camera->GetComponent<TransformComponent>(), mouseX, mouseY);
        //LOG_TRACE("Ray From Camera P:{0},{1},{2} D:{3},{4},{5}", mouseWorldRay.Position.x, mouseWorldRay.Position.y, mouseWorldRay.Position.z, mouseWorldRay.Direction.x, mouseWorldRay.Direction.y, mouseWorldRay.Direction.z);
        if(UIDebugRaycast)
            oGFX::DebugDraw::AddLine(mouseWorldRay.Position, mouseWorldRay.Position + mouseWorldRay.Direction * 20.f);
        //Point2D mouseWorldPoint{ mousePos };

        /*SceneCamera* cam = SceneCamera::MainCamera();
        if (cam == nullptr)
            return;
        oom::vec2 mousePos = cam->MouseToGameScreen(Input::GetMouseX(), Input::GetMouseY());
        mousePos.x -= Application::Get().GetWindow().GetWidth() / 2.0f;
        mousePos.y -= Application::Get().GetWindow().GetHeight() / 2.0f;
        Point2D mouseScreenPoint{ mousePos };

        mousePos = cam->ScreenToWorld(Input::GetMouseX(), Input::GetMouseY());
        Point2D mouseWorldPoint{ mousePos };*/

        bool skip = false;

        static Ecs::Query canvas_with_raycaster_query = Ecs::make_query<GameObjectComponent, TransformComponent, UICanvasComponent, GraphicsRaycasterComponent>();
        m_world->for_each(canvas_with_raycaster_query, [&](GameObjectComponent& goc, TransformComponent& tf, UICanvasComponent& canvas, GraphicsRaycasterComponent& raycaster)
            {
                // retrieve canvas gameobject
                auto go = m_scene->FindWithInstanceID(goc.Id);

                // Iterate through and update all childs in REVERSE
                auto childs = go->GetChildren(true);
                for (auto iter = childs.rbegin(); iter != childs.rend(); ++iter)
                {
                    auto& child = *iter;
                    if (child.ActiveInHierarchy() == false)
                        continue;

                    // skip all children that does not have rect transform nor raycast component
                    if (child.HasComponent<RectTransformComponent>() == false && child.HasComponent<UIRaycastComponent>() == false)
                        continue;

                    bool mouseOutside = true;

                    switch (canvas.RenderingMode)
                    {
                    case UICanvasComponent::RenderMode::Overlay:
                        //mouseOutside = !Intersection2D::PointAABB(mouseScreenPoint, child.GetComponent<RectTransformComponent>().BoundingVolume);
                        break;
                    case UICanvasComponent::RenderMode::WorldSpace:
                    //case UICanvasComponent::RenderMode::CanvasSpace:
                        //Shoot a ray
                        auto obb = child.GetComponent<RectTransformComponent>().BoundingVolume;
                        mouseOutside = !intersection::RayOBB(mouseWorldRay, obb);
                        //mouseOutside = !Intersection2D::PointAABB(mouseWorldPoint, child.GetComponent<RectTransformComponent>().BoundingVolume);
                        break;
                    }

                    if (mouseOutside)
                        continue;

                    // Intersection detected!

                    // we find the highest object that's currently selected (because what we're clicking on now does not OWN a raycast component!
                    // [it'll either be a parent raycastComponent or be the canvas which means not found and terminate.]
                    GameObject currSelectedUI = child;
                    while (currSelectedUI.HasComponent<UICanvasComponent>() == false && currSelectedUI.HasComponent<UIRaycastComponent>() == false)
                    {
                        currSelectedUI = currSelectedUI.GetParent();
                    }

                    // on stay event [ selected = current, selectedButton = previous ]
                    if (currSelectedUI == m_prevSelectedUI)
                    {
                        UpdateButtonCallback(currSelectedUI.GetInstanceID(), &currSelectedUI.GetComponent<UIRaycastComponent>(), true);
                        skip = true;
                        return;
                    }

                    // the newly selected item is not what i previously selected, if i have previously selected something i deselct it first.
                    // On exit.
                    if (m_scene->IsValid(m_prevSelectedUI))
                    {
                        UpdateButtonCallback(m_prevSelectedUI.GetInstanceID(), &m_prevSelectedUI.GetComponent<UIRaycastComponent>(), false);
                        m_prevSelectedUI = GameObject{}; //invalid gameobject
                    }

                    // On Enter : we try because currently Selected might still be a canvas.
                    UIRaycastComponent* selectedButtonPointer = currSelectedUI.TryGetComponent<UIRaycastComponent>();
                    if (selectedButtonPointer != nullptr /*&& selectedButtonPointer->IsInteractable()*/)
                    {
                        auto& name = currSelectedUI.Name();
                        if (UIDebugPrint)
                            LOG_TRACE("newly selected UI {0}", name);

                        UpdateButtonCallback(currSelectedUI.GetInstanceID(), &currSelectedUI.GetComponent<UIRaycastComponent>(), true);
                        m_prevSelectedUI = currSelectedUI;
                    }
                    
                    //UpdateButtonCallback(currentlySelected.GetInstanceID(), &currentlySelected.GetComponent<UIRaycastComponent>(), true);
                    //m_previouslySelectedObject = currentlySelected;
                    skip = true;
                    return;
                }

            });

        // not hovering over any ui element
        // deselect what i'm previously selecting
        if (!skip && m_scene->IsValid(m_prevSelectedUI))
        {
            UpdateButtonCallback(m_prevSelectedUI.GetInstanceID(), &m_prevSelectedUI.GetComponent<UIRaycastComponent>(), false);
            m_prevSelectedUI = GameObject{};  // set to null.
        }
    }

    bool UISystem::UpdateButtonCallback(UUID buttonId, UIRaycastComponent* raycastComp, bool isInside)
    {
        // mouse was previously not in raycast volume, and also currently not in raycast volume
        if (!raycastComp->HasEntered && !isInside)
            return false;

        // we are sure to launch an UIButton event now
        UIButtonEvent e;
        e.buttonID = buttonId;

        // mouse was previously not in raycast volume, but is now in raycast volume
        if (!raycastComp->HasEntered && isInside)
        {
            raycastComp->HasEntered = true;
            e.Type = UIButtonEventType::ON_POINTER_ENTER;
            EventManager::Broadcast<UIButtonEvent>(&e);
            if (UIDebugPrint)
                LOG_TRACE("UI On pointer enter!");
            //raycastComp->InvokeButtonEvent("OnPointerEnter");
            //if (!raycastComp->IsInteractable()) // for if OnPointerEnter sets interactable false
            //    return false;
            return true;
        }
        // mouse was previously in raycast volume, but is not in raycast volume anymore
        if (raycastComp->HasEntered && !isInside)
        {
            if (raycastComp->IsPressed)
            {
                raycastComp->IsPressed = false;
                e.Type = UIButtonEventType::ON_RELEASE;
                EventManager::Broadcast<UIButtonEvent>(&e);
                if (UIDebugPrint)
                    LOG_TRACE("UI On Release!");
                //raycastComp->InvokeButtonEvent("OnRelease");
                //if (!raycastComp->IsInteractable()) // for if OnRelease sets interactable false
                //    return false;
            }
            raycastComp->HasEntered = false;
            e.Type = UIButtonEventType::ON_POINTER_EXIT;
            EventManager::Broadcast<UIButtonEvent>(&e);
            if (UIDebugPrint)
                LOG_TRACE("UI On Pointer Exit!");
            //raycastComp->InvokeButtonEvent("OnPointerExit");
            return false;
        }

        // mouse was previously in raycast volume, and is still in raycast volume

        if (oo::input::IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            raycastComp->IsPressed = true;
            e.Type = UIButtonEventType::ON_PRESS;
            EventManager::Broadcast<UIButtonEvent>(&e);
            if (UIDebugPrint)
                LOG_TRACE("UI On Press!");
            //raycastComp->InvokeButtonEvent("OnPress");
            //if (!raycastComp->IsInteractable()) // for if OnRelease sets interactable false
            //    return false;
        }
        if (raycastComp->IsPressed && oo::input::IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            e.Type = UIButtonEventType::ON_CLICK;
            EventManager::Broadcast<UIButtonEvent>(&e);
            if (UIDebugPrint)
                LOG_TRACE("UI On Click!");
            //raycastComp->InvokeButtonEvent("OnClick");
            // isInteractable check since OnClick could disable the button,
            //if so OnRelease already invoked, shouldn't be invoked again after OnInteractDisabled
            /*if (!raycastComp->IsInteractable())
                return false;*/

            raycastComp->IsPressed = false;
            e.Type = UIButtonEventType::ON_RELEASE;
            EventManager::Broadcast<UIButtonEvent>(&e);
            if (UIDebugPrint)
                LOG_TRACE("UI On Release!");
            //raycastComp->InvokeButtonEvent("OnRelease");
            //if (!raycastComp->IsInteractable()) // for if OnRelease sets interactable false
            //    return false;
        }

        return true;
    }

    Ray UISystem::ScreenToWorld(Camera camera, TransformComponent* cameraTf, int32_t mouse_x, int32_t mouse_y)
    {
        auto [winx, winy] = Application::Get().GetWindow().GetSize();
        int32_t viewport_x = static_cast<int32_t>(winx);
        int32_t viewport_y = static_cast<int32_t>(winy);

        // TODO : additional call to get viewport of window instead!
        //editorGameViewCallback(viewport_x, viewport_y, mouse_x, mouse_y);

        glm::vec3 screenPosition{ static_cast<float>(mouse_x),static_cast<float>(mouse_y), 0 };
        // convert to -1 to 1
        glm::vec3 clipPosition{ 0 };
        clipPosition.x = (screenPosition.x / viewport_x) * 2.0f - 1.0f;
        clipPosition.y = -(screenPosition.y / viewport_y) * 2.0f + 1.0f;

        auto& transform = cameraTf;
        // camera's Project Matrix
        const auto& proj = camera.matrices.perspective;
        const auto& pos = transform->GetGlobalPosition();
        
        // camera's transform in World Space
        //const auto& worldSpaceMatrix = transform->GetGlobalMatrix(); 
        //glm::vec3 point = worldSpaceMatrix * glm::inverse(proj) * glm::vec4{ worldpos, 1 };
        //// direction is calculated from camera global position to the newly found point
        //auto dir = point - cameraTf->GetGlobalPosition();
        //// we use the camera's Z as the starting point.
        //return { {point.x, point.y, cameraTf->GetGlobalPosition().z}, dir };

        glm::vec4 viewPosition = glm::inverse(proj) * glm::vec4{ clipPosition, 1 };
        viewPosition.z = -1.f;

        glm::vec4 world_pos = glm::inverse(camera.matrices.view) * viewPosition;
        glm::vec3 divided_pos = glm::vec3{ world_pos } / world_pos.w;

        //////TODO: store the inverse of the camera
        //float xProj = -1.0f / proj[0][0];
        //float yProj = 1.0f / proj[1][1];
        //float zProj = 1.0f / proj[2][2];
        ////oo::AABB2D extents{ {pos.x - xProj, pos.y - yProj },{pos.x + xProj , pos.y + yProj } };
        //worldpos.x = pos.x + worldpos.x * xProj;
        //worldpos.y = pos.y + worldpos.y * yProj;
        //worldpos.z = pos.z + worldpos.z * zProj;
        //
        //worldpos += transform->GlobalForward();

        return { pos, glm::normalize(divided_pos - pos)};
    }

    void UISystem::OnUIAssign(Ecs::ComponentEvent<UIComponent>* evnt)
    {
        assert(m_world != nullptr); // it should never be nullptr, was the Init funciton called?
        auto& uiComponent = evnt->component;
        auto& transform_component = m_world->get_component<TransformComponent>(evnt->entityID);
        auto& go_component = m_world->get_component<GameObjectComponent>(evnt->entityID);
        InitializeUI(uiComponent, transform_component, go_component);
    }
    
    void UISystem::OnUIRemove(Ecs::ComponentEvent<UIComponent>* evnt)
    {
        auto& comp = evnt->component;
        m_scene->RemovePickingID(comp.PickingID);
        m_graphicsWorld->DestroyUIInstance(comp.UI_ID);
    }

    void UISystem::InitializeUI(UIComponent& uiComp, TransformComponent& transformComp, GameObjectComponent& goComp)
    {
        uiComp.UI_ID = m_graphicsWorld->CreateUIInstance();
        uiComp.PickingID = m_scene->GeneratePickingID(goComp.Id);
        //update graphics world side to prevent wrong initial placement
        auto& ui = m_graphicsWorld->GetUIInstance(uiComp.UI_ID);
        ui.entityID = uiComp.PickingID;
        ui.localToWorld = transformComp.GetGlobalMatrix();
    }

    void UISystem::OnPreviewWindowImageResize(PreviewWindowImageResizeEvent* e)
    {
        m_previewImgStartPos = e->StartPosition;
        m_previewImgWidth = e->Width;
        m_previewImgHeight = e->Height;
    }

    /*void UISystem::OnTextAssign(Ecs::ComponentEvent<UITextComponent>* evnt)
    {
        ASSERT_MSG(m_world != nullptr, "it should never be nullptr, was the Init funciton called?");
        ASSERT_MSG(m_world->has_component<UIComponent>(evnt->entityID), " we should have the ui component by now!");
        auto& uiComponent = m_world->get_component<UIComponent>(evnt->entityID);
        uiComponent.SetText(true);
    }

    void UISystem::OnTextRemove(Ecs::ComponentEvent<UITextComponent>* evnt)
    {
    }

    void UISystem::InitializeText(UITextComponent& uiTextComp, TransformComponent& tfComp)
    {
    }

    void UISystem::OnImageAssign(Ecs::ComponentEvent<UIImageComponent>* evnt)
    {
    }

    void UISystem::OnImageRemove(Ecs::ComponentEvent<UIImageComponent>* evnt)
    {
    }

    void UISystem::InitializeImage(UIImageComponent& uiImgComp, TransformComponent& tfComp)
    {
    }*/

}