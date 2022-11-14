/************************************************************************************//*!
\file           VulkanContext.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Jun 17, 2022
\brief          Describes the Implementation of the Vulkan Backend Graphics Context
                and capabilities.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "Ouroboros/Vulkan/VulkanContext.h"
#include "Ouroboros/Core/Timer.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include <sdl2/SDL.h>
#include <sdl2/SDL_vulkan.h>

#include "OO_Vulkan/src/DefaultMeshCreator.h"

#include "Ouroboros/EventSystem/EventManager.h"
#include "Ouroboros/Core/Events/ApplicationEvent.h"
#include "Ouroboros/Core/WindowsWindow.h"
#include <App/Editor/Utility/ImGuiManager.h>

namespace oo
{

    VulkanRenderer* VulkanContext::vr{nullptr};
    GraphicsWorld VulkanContext::gw;
    Window VulkanContext::m_window;

    static constexpr int hardCodedLights = 6;
    static int32_t someLights[hardCodedLights];

    VulkanContext::VulkanContext(SDL_Window* window)
        : m_windowHandle(window)
    {
        ASSERT_MSG(m_windowHandle == nullptr, "Window handle is null!");

        //vkEngine.SetWindow(window);
    }

    VulkanContext::~VulkanContext()
    {
        vr->DestroyWorld(&gw);
        //vkEngine.cleanup();
        delete vr;

    }

    void VulkanContext::Init()
    {
        EventManager::Subscribe<VulkanContext, WindowResizeEvent>(this, &VulkanContext::OnWindowResize);
        EventManager::Subscribe<VulkanContext, WindowMinimizeEvent>(this, &VulkanContext::OnWindowMinimize);
        EventManager::Subscribe<VulkanContext, WindowRestoredEvent>(this, &VulkanContext::OnWindowRestored);

        
        // Setup Vulkan
        uint32_t extensions_count = 0;
        SDL_Vulkan_GetInstanceExtensions(m_windowHandle, &extensions_count, NULL);
        std::vector<const char*> extensions;
        extensions.resize(extensions_count);
        SDL_Vulkan_GetInstanceExtensions(m_windowHandle, &extensions_count, &extensions[0]);

        //// Setup Camera Internally, not used
        vr = VulkanRenderer::get();
        //auto& camera = vr->camera;
        ////m_runtimeCC.SetCamera(&camera);
        //camera.m_CameraMovementType = Camera::CameraMovementType::firstperson;
        //camera.movementSpeed = 5.0f;
        //camera.SetAspectRatio((float)m_window.m_width / (float)m_window.m_height);
        ////camera.SetRotation(glm::vec3(0.0f, 180.0f, 0.0f));
        ////camera.SetRotationSpeed(0.5f);
        ////camera.SetPosition(glm::vec3(0.0f, 2.0f, 4.0f));
        
        

        oGFX::SetupInfo si;
        si.debug = true;
        si.renderDoc = false;
        si.SurfaceFunctionPointer = std::function<void()>([&]() {
            return SDL_Vulkan_CreateSurface(m_windowHandle, vr->m_instance.instance, &vr->m_instance.surface);
            });
        si.extensions = extensions;
        //m_window.m_width = w;
        //m_window.m_height = h;
        m_window.m_type = Window::WindowType::SDL2;
        m_window.rawHandle = m_windowHandle;
        try
        {
            vr->Init(si, m_window);
        } 
        catch (std::runtime_error e)
        {
            std::cout << "VK_init: " << e.what() << std::endl;
        }

        for (size_t i = 0; i < hardCodedLights; i++)
        {
            someLights[i] = gw.CreateLightInstance();
        }

        // setup world..
        // TODO: move this out of here pls
        auto obj = gw.CreateObjectInstance();
        auto plane = gw.CreateObjectInstance();

        //int32_t white = 0x00404040;
        //auto tex = vr->CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&white));
        uint32_t tex = vr->whiteTextureID;

        DefaultMesh dm = CreateDefaultCubeMesh();
        cubeMesh.reset(vr->LoadMeshFromBuffers(dm.m_VertexBuffer, dm.m_IndexBuffer, nullptr));
        DefaultMesh pm = CreateDefaultPlaneXZMesh();
        planeMesh.reset(vr->LoadMeshFromBuffers(pm.m_VertexBuffer, pm.m_IndexBuffer, nullptr));

        /// This is an example of how to load in a full scene from FBX.. should work..
        //std::unique_ptr<ModelFileResource> loadedModel{vr->LoadModelFromFile("Model/Filename.fbx")};
        //std::function<void(ModelFileResource*,Node*)> EntityHelper = [&](ModelFileResource* model,Node* node) {
        //    if (node->meshRef != static_cast<uint32_t>(-1))
        //    {
        //        auto& ed = entities.emplace_back(EntityInfo{});
        //        ed.modelID = model->gfxMeshIndices[node->meshRef];
        //        ed.name = node->name;
        //        ed.entityID = FastRandomMagic();
        //        // this is trash just take the xform
        //        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(node->transform), glm::value_ptr(ed.position), glm::value_ptr(ed.rotVec), glm::value_ptr(ed.scale));
        //        
        //        ed.bindlessGlobalTextureIndex_Albedo = diffuseTexture3;
        //        ed.instanceData = 0;
        //    }
        //    for (auto& child : node->children)
        //    {
        //        EntityHelper(model,child);
        //    }            
        //
        //};
        //if (loadedModel)
        //{
        //    EntityHelper(loadedModel.get(),loadedModel->sceneInfo);
        //}

        {
            auto& myObj = gw.GetObjectInstance(obj); 
            myObj.modelID = cubeMesh->meshResource;
            myObj.scale = glm::vec3{ 2.1f,1.1f,1.1f };
            myObj.rotVec = glm::vec3{ 1.1f,1.1f,1.1f };
            myObj.rot = 35.0f;
            myObj.localToWorld = glm::mat4(1.0f);
            myObj.localToWorld = glm::translate(myObj.localToWorld, myObj.position);
            myObj.localToWorld = glm::rotate(myObj.localToWorld,glm::radians(myObj.rot), myObj.rotVec);
            myObj.localToWorld = glm::scale(myObj.localToWorld, myObj.scale);
            myObj.bindlessGlobalTextureIndex_Albedo = tex;
            myObj.submesh[0] = 1;
        }
       
        {
            auto& myPlane = gw.GetObjectInstance(plane);
            myPlane.modelID = planeMesh->meshResource;
            myPlane.position = { 0.0f,-1.0f,0.0f };
            myPlane.scale = { 15.0f,1.0f,15.0f };
            myPlane.localToWorld = glm::mat4(1.0f);
            myPlane.localToWorld = glm::translate(myPlane.localToWorld, myPlane.position);
            myPlane.localToWorld = glm::rotate(myPlane.localToWorld,glm::radians(myPlane.rot), myPlane.rotVec);
            myPlane.localToWorld = glm::scale(myPlane.localToWorld, myPlane.scale);
            myPlane.bindlessGlobalTextureIndex_Albedo = tex;
            myPlane.submesh[0] = 1;
        }        

        gw.numCameras = 1;
        // TODO: BAD fix this..
        vr->InitWorld(&gw);
        vr->SetWorld(&gw);
    }

    void VulkanContext::OnUpdateBegin()
    {
        //int w, h;
        //SDL_Vulkan_GetDrawableSize(m_windowHandle, &w, &h);
        //m_window.m_width = w;
        //m_window.m_height = h;

    }

    void VulkanContext::SwapBuffers()
    {
        if(!m_minimized)
            vr->Present();
    }

    void VulkanContext::InitImGui()
    {
        ImGui_ImplSDL2_InitForVulkan(m_windowHandle);
        vr->InitImGUI();

        ImGuiManager::InitAssetsAll();
    }

    void VulkanContext::ResetImguiInit()
    {
        ImGui_ImplSDL2_InitForVulkan(m_windowHandle);
        vr->RestartImgui();
    }

    void VulkanContext::OnImGuiBegin()
    {
        ImGui_ImplVulkan_NewFrame();
    }

    void VulkanContext::OnImGuiEnd()
    {
        // Vulkan will call internally

        // temporarily shift here for better structuring
        //m_runtimeCC.Update(oo::timer::dt());

        if (vr->PrepareFrame() == true)        
        {
            auto& obj = gw.GetObjectInstance(0);
            obj.rot += 0.25f;
            obj.localToWorld = glm::mat4(1.0f);
            obj.localToWorld = glm::translate(obj.localToWorld, obj.position);
            obj.localToWorld = glm::rotate(obj.localToWorld,glm::radians(obj.rot), obj.rotVec);
            obj.localToWorld = glm::scale(obj.localToWorld, obj.scale);

            {

                OmniLightInstance* lights[hardCodedLights];
                for (size_t i = 0; i < hardCodedLights; i++)
                {
                    lights[i] = &gw.GetLightInstance(someLights[i]);
                }


                static float lightTimer = 0.0f;
                lightTimer += oo::timer::dt() * 0.25f;


                lights[0]->position = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
                lights[0]->color = glm::vec4(1.5f);
                lights[0]->radius.x = 15.0f;
                // Red   
                lights[1]->position = glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f);
                lights[1]->color = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
                lights[1]->radius.x = 15.0f;
                // Blue  
                lights[2]->position = glm::vec4(2.0f, -1.0f, 0.0f, 0.0f);
                lights[2]->color = glm::vec4(0.0f, 0.0f, 2.5f, 0.0f);
                lights[2]->radius.x = 5.0f;
                // Yellow
                lights[3]->position = glm::vec4(0.0f, -0.9f, 0.5f, 0.0f);
                lights[3]->color = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
                lights[3]->radius.x = 2.0f;
                // Green 
                lights[4]->position = glm::vec4(0.0f, -0.5f, 0.0f, 0.0f);
                lights[4]->color = glm::vec4(0.0f, 1.0f, 0.2f, 0.0f);
                lights[4]->radius.x = 5.0f;
                // Yellow
                lights[5]->position = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
                lights[5]->color = glm::vec4(1.0f, 0.7f, 0.3f, 0.0f);
                lights[5]->radius.x = 25.0f;

                lights[0]->position.x = sin(glm::radians(360.0f * lightTimer)) * 5.0f;
                lights[0]->position.z = cos(glm::radians(360.0f * lightTimer)) * 5.0f;

                lights[1]->position.x = -4.0f + sin(glm::radians(360.0f * lightTimer) + 45.0f) * 2.0f;
                lights[1]->position.z = 0.0f + cos(glm::radians(360.0f * lightTimer) + 45.0f) * 2.0f;

                lights[2]->position.x = 4.0f + sin(glm::radians(360.0f * lightTimer)) * 2.0f;
                lights[2]->position.z = 0.0f + cos(glm::radians(360.0f * lightTimer)) * 2.0f;

                lights[4]->position.x = 0.0f + sin(glm::radians(360.0f * lightTimer + 90.0f)) * 5.0f;
                lights[4]->position.z = 0.0f - cos(glm::radians(360.0f * lightTimer + 45.0f)) * 5.0f;

                lights[5]->position.x = 0.0f + sin(glm::radians(-360.0f * lightTimer + 135.0f)) * 10.0f;
                lights[5]->position.z = 0.0f - cos(glm::radians(-360.0f * lightTimer - 45.0f)) * 10.0f;
            }

            // Upload CPU light data to GPU. Ideally this should only contain lights that intersects the camera frustum.
            vr->UploadLights();

            // Render the frame
            vr->RenderFrame();

      

        if(!m_minimized)
            vr->DrawGUI();

        } // if prepare frame is true

    }

    void VulkanContext::ResetImguiShutdown()
    {
        vr->ImguiSoftDestroy();
    }


    void VulkanContext::OnImGuiShutdown()
    {
        vr->DestroyImGUI();
    }

    bool VulkanContext::SetVSync(bool enable)
    {
        UNREFERENCED(enable);
        // vulkan does not currently support vsync yet
        return false;
    }

    VulkanRenderer* VulkanContext::getRenderer()
    {
        return VulkanRenderer::get();
    }

    void VulkanContext::OnWindowResize(WindowResizeEvent* e)
    {
        m_window.m_height = e->GetHeight();
        m_window.m_width = e->GetWidth();
    }

    void VulkanContext::OnWindowMinimize(WindowMinimizeEvent*)
    {
        m_minimized = true;
        m_window.m_height = 0;
        m_window.m_width = 0;
    }

    void VulkanContext::OnWindowRestored(WindowRestoredEvent*)
    {
        m_minimized = false;
        int w, h;
        SDL_Vulkan_GetDrawableSize(m_windowHandle, &w, &h);
        m_window.m_width = w;
        m_window.m_height = h;
    }

    void VulkanContext::SetWindowResized()
    {
        //vkEngine.SetWindowResized();
    }

}