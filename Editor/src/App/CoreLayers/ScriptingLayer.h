/************************************************************************************//*!
\file           ScriptingLayer.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Aug 22, 2022
\brief          Defines a layer to handle the global scripting functionality

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <filesystem>

#include "Ouroboros/Core/Layer.h"
#include <Scripting/Scripting.h>
#include "Ouroboros/Scripting/ScriptManager.h"
#include "Ouroboros/Scripting/ScriptSystem.h"

#include "Ouroboros/Transform/TransformComponent.h"
#include "Ouroboros/Vulkan/MeshRendererComponent.h"
#include "Ouroboros/Vulkan/ParticleEmitterComponent.h"
#include "Ouroboros/Animation/AnimationComponent.h"
#include "Ouroboros/Audio/AudioSourceComponent.h"

#include "Ouroboros/Physics/RigidbodyComponent.h"
#include "Ouroboros/Physics/ColliderComponents.h"

#include "Ouroboros/EventSystem/EventSystem.h"
#include "Ouroboros/Core/Events/ApplicationEvent.h"
#include "App/Editor/Events/ToolbarButtonEvent.h"
#include "Ouroboros/Scene/EditorController.h"

#if OO_EDITOR
#include "App/Editor/UI/Tools/WarningMessage.h"
#endif // OO_EDITOR


namespace oo
{
    class ScriptingLayer final : public oo::Layer
    {
    public:

        ScriptingLayer(SceneManager& sceneManager)
        {
            ScriptManager::RegisterComponent<TransformComponent>("Ouroboros", "Transform");
            ScriptManager::RegisterComponent<MeshRendererComponent>("Ouroboros", "MeshRenderer");
            ScriptManager::RegisterComponent<ParticleEmitterComponent>("Ouroboros", "ParticleSystem");
            ScriptManager::RegisterComponent<AnimationComponent>("Ouroboros", "Animator");
            ScriptManager::RegisterComponent<AudioSourceComponent>("Ouroboros", "AudioSource");

            ScriptManager::RegisterComponent<RigidbodyComponent>("Ouroboros", "Rigidbody");
            ScriptManager::RegisterComponent<BoxColliderComponent>("Ouroboros", "BoxCollider");
            ScriptManager::RegisterComponent<SphereColliderComponent>("Ouroboros", "SphereCollider");
            ScriptManager::RegisterComponent<CapsuleColliderComponent>("Ouroboros", "CapsuleCollider");
            ScriptManager::RegisterComponent<ConvexColliderComponent>("Ouroboros", "ConvexCollider");

            ScriptManager::s_SceneManager = &sceneManager;

#if OO_EDITOR
            EventManager::Subscribe<ToolbarButtonEvent>([](ToolbarButtonEvent* e)
                {
                    if (e->m_buttonType != ToolbarButtonEvent::ToolbarButton::COMPILE)
                        return;
                    if (ScriptManager::Compile())
                    {
                        ScriptManager::Load();
                        ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<ScriptSystem>()->RefreshScriptInfoAll();
						WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Compiled Successfully");
					}
					else
					{
						WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_WARNING, "Failed to compile!");
					}
                });
            EventManager::Subscribe<EditorController::OnStopEvent>([](EditorController::OnStopEvent* e)
                {
                    ScriptManager::Load();
                });
            EventManager::Subscribe<WindowFocusEvent>([](WindowFocusEvent* e)
                {
                    std::shared_ptr<Scene> scene = ScriptManager::s_SceneManager->GetActiveScene<Scene>();
                    if (scene == nullptr)
                        return;
                    ScriptSystem* ss = scene->GetWorld().Get_System<ScriptSystem>();
                    if (ss->IsPlaying())
                        return;

                    if (std::filesystem::exists(Project::GetScriptBuildPath() / "Scripting.dll"))
                    {
                        bool outdated = false;
                        std::filesystem::file_time_type dll_time = std::filesystem::last_write_time(Project::GetScriptBuildPath() / "Scripting.dll");

                        if (std::filesystem::last_write_time(Project::GetProjectFolder() / "Scripts") > dll_time)
                        {
                            outdated = true;
                        }
                        else
                        {
                            for (std::filesystem::directory_entry const& dir : std::filesystem::recursive_directory_iterator(Project::GetProjectFolder() / "Scripts"))
                            {
                                if (!dir.is_directory())
                                    continue;
                                if (dir.last_write_time() > dll_time)
                                {
                                    outdated = true;
                                    break;
                                }
                            }
                        }

                        if (!outdated)
                            return;
                    }

                    if (ScriptManager::Compile())
                    {
                        ScriptManager::Load();
                        ss->RefreshScriptInfoAll();
                    }
                });
#endif
        }

        ~ScriptingLayer()
        {
            ScriptEngine::Shutdown();
        }

        void OnAttach() override final
        {

        }

        void OnDetach() override final
        {

        }

        void OnUpdate() override final
        {
        }
    };

}
