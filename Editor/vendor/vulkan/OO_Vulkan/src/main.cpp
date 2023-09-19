/************************************************************************************//*!
\file           main.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "gpuCommon.h"
#include "Vulkanrenderer.h"


#include <iostream>
#include <iomanip>
#include <chrono>
#include <cctype>
#include <thread>
#include <functional>
#include <random>

#include "window.h"
#include "input.h"

#include "Tests_Assignment1.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_win32.h>
#include "ImGuizmo.h"

#include "IcoSphereCreator.h"
#include "Tree.h"

#include <numeric>
//#include <algorithm>

#include "Profiling.h"

#include "BoudingVolume.h"
#include "DefaultMeshCreator.h"

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
    os << std::setprecision(4) << "[" << vec.x << "," << vec.y << "," << vec.z << "]";
    return os;
}

bool BoolQueryUser(const char * str)
{
    char response {0} ;
    std::cout<< str << " [y/n]"<< std::endl;
    while (! response ){
        std::cin>> response;
        response  = static_cast<char>(std::tolower(response));
        if (response != 'y' && response != 'n'){
            std::cout<< "Invalid input["<< response<< "]please try again"<< std::endl;
            response = 0;           
        }
    }
    return response == 'n' ? false : true;
}

oGFX::Color generateRandomColor()
{
    static std::default_random_engine rndEngine(3456);
    static std::uniform_real<float> uniformDist( 0.0f,1.0f);
   
    oGFX::Color col; 
    col.a = 1.0f;
    float sum;
    do
    {
        col.r = uniformDist(rndEngine);
        col.g = uniformDist(rndEngine);
        col.b = uniformDist(rndEngine);
         sum = (col.r + col.g + col.b);
    }while (sum < 2.0f && sum > 2.8f );
    return col;
}

void UpdateBV(Model* model, VulkanRenderer::EntityDetails& entity, int i = 0)
{
    std::vector<Point3D> vertPositions;
    vertPositions.resize(model->vertices.size());
    glm::mat4 xform(1.0f);
    xform = glm::translate(xform, entity.position);
    xform = glm::rotate(xform,glm::radians(entity.rot), entity.rotVec);
    xform = glm::scale(xform, entity.scale);
    std::transform(model->vertices.begin(), model->vertices.end(), vertPositions.begin(), [&](const oGFX::Vertex& v) {
        glm::vec4 pos{ v.pos,1.0f };
        pos = xform * pos;
        return pos; 
        }
    );
    switch (i)
    {
    case 0:
    oGFX::BV::RitterSphere(entity.sphere, vertPositions);
    break;
    case 1:
    oGFX::BV::LarsonSphere(entity.sphere, vertPositions, oGFX::BV::EPOS::_6);
    break;
    case 2:
    oGFX::BV::LarsonSphere(entity.sphere, vertPositions, oGFX::BV::EPOS::_14);
    break;
    case 3:
    oGFX::BV::LarsonSphere(entity.sphere, vertPositions, oGFX::BV::EPOS::_26);
    break;
    case 4:
    oGFX::BV::LarsonSphere(entity.sphere, vertPositions, oGFX::BV::EPOS::_98);
    break;
    case 5:
    oGFX::BV::EigenSphere(entity.sphere, vertPositions);
    break;
    default:
    oGFX::BV::RitterSphere(entity.sphere, vertPositions);
    break;
    }
    
    oGFX::BV::BoundingAABB(entity.aabb, vertPositions);
}

enum class AppWindowSizeTypes : int
{
    HD_720P_16_9,
    HD_900P_16_10
};

static glm::ivec2 gs_AppWindowSizes[] =
{
    glm::ivec2{ 1280, 720 },
    glm::ivec2{ 1440, 900 },
};

static float* gizmoHijack = nullptr; // TODO: Clean this up...

int main(int argc, char argv[])
{
    (void)argc;
    (void)argv;

    _CrtDumpMemoryLeaks();
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG );
    //_CrtSetBreakAlloc(228);

    //RunAllTests();

    AppWindowSizeTypes appWindowSizeType = AppWindowSizeTypes::HD_900P_16_10;
    const glm::ivec2 windowSize = gs_AppWindowSizes[(int)appWindowSizeType];
    Window mainWindow(windowSize.x, windowSize.y);
    mainWindow.Init();
    
    oGFX::SetupInfo setupSpec;

    //setupSpec.debug = BoolQueryUser("Do you want debugging?");
    //setupSpec.renderDoc = BoolQueryUser("Do you want renderdoc?");
    setupSpec.debug = true;
    setupSpec.renderDoc = true;
    setupSpec.useOwnImgui = true;

    VulkanRenderer* renderer = VulkanRenderer::get();
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    ImGui::StyleColorsDark();// Setup Dear ImGui style

    auto result = renderer->Init(setupSpec, mainWindow);
    if (result != oGFX::SUCCESS_VAL)
    {
        std::cout << "Cannot create vulkan instance! " << e.what() << std::endl;
        getchar();
        return 0;
    }
      
    renderer->InitImGUI();

    std::cout << "Created vulkan instance!"<< std::endl;
  

    std::vector<oGFX::Vertex>triVerts{
            oGFX::Vertex{ {-0.5,-0.5,0.0}, { 1.0f,0.0f,0.0f }, { 1.0f,0.0f,0.0f }, { 0.0f,0.0f } },
            oGFX::Vertex{ { 0.5,-0.5,0.0}, { 0.0f,1.0f,0.0f }, { 0.0f,1.0f,0.0f }, { 1.0f,0.0f } },
            oGFX::Vertex{ { 0.0, 0.5,0.0}, { 0.0f,0.0f,1.0f }, { 0.0f,0.0f,1.0f }, { 0.0f,1.0f } }
    };
    std::vector<uint32_t>triIndices{
        0,1,2
    };

    // triangle splitting test
    Triangle t;
    t.v0 = Point3D(-4.0f, -8.0f, 0.0f);
    t.v1 = Point3D(-2.0f,  6.0f, 0.0f);
    t.v2 = Point3D( 2.0f, -2.0f, 0.0f);
    Plane p;
    p.normal = vec4{ -1.0f,0.0f,0.0f, 1.0f };
    std::vector<Point3D> pV; std::vector<uint32_t> pI;
    std::vector<Point3D> nV; std::vector<uint32_t> nI;
    oGFX::BV::SliceTriangleAgainstPlane(t, p, pV, pI, nV, nI);

    auto defaultPlaneMesh = CreateDefaultPlaneXZMesh();
    auto defaultCubeMesh = CreateDefaultCubeMesh();

    std::unique_ptr<Model> icoSphere{};
    {
        auto [pos,triangleList] = icosahedron::make_icosphere(2);

        std::vector<oGFX::Vertex> vertices;
        vertices.reserve(pos.size());
        for (size_t i = 0; i < pos.size(); i++)
        {
            oGFX::Vertex v{};
            v.pos = pos[i];
            vertices.push_back(v);
        }
        std::vector<uint32_t>indices;
        indices.reserve(triangleList.size() * 3ull);
        for (size_t i = 0; i < triangleList.size(); i++)
        {           
            indices.push_back(triangleList[i].vertex[0]);
            indices.push_back(triangleList[i].vertex[1]);
            indices.push_back(triangleList[i].vertex[2]);

            glm::vec3 e1 = vertices[triangleList[i].vertex[0]].pos - vertices[triangleList[i].vertex[1]].pos;
            glm::vec3 e2 = vertices[triangleList[i].vertex[2]].pos - vertices[triangleList[i].vertex[1]].pos;
            glm::vec3 norm = glm::normalize(glm::cross(e1, e2));
            for (size_t j = 0; j < 3; j++)
            {
                vertices[triangleList[i].vertex[j]].norm = norm;
            }

        }
        renderer->g_MeshBuffers.VtxBuffer.reserve(100000*sizeof(oGFX::Vertex));
        renderer->g_MeshBuffers.IdxBuffer.reserve(100000*sizeof(oGFX::Vertex));
        icoSphere.reset( renderer->LoadMeshFromBuffers(vertices, indices, nullptr));
    }

    std::unique_ptr<Model> bunny{ renderer->LoadMeshFromFile("Models/bunny.obj") };
    std::vector<Point3D> vertPositions;
    Sphere ms;
    if (bunny)
    {
        vertPositions.resize(bunny->vertices.size());
        std::transform(bunny->vertices.begin(), bunny->vertices.end(), vertPositions.begin(), [](const oGFX::Vertex& v) { return v.pos; });
        oGFX::BV::RitterSphere(bunny->s, vertPositions);
        oGFX::BV::BoundingAABB(bunny->aabb, vertPositions);
        oGFX::BV::EigenSphere(ms, vertPositions);

        //quicky dirty scaling
        uint32_t bunnyTris = static_cast<uint32_t>(bunny->indices.size()) / 3;
        std::cout << "bunny model: " << bunnyTris << std::endl;
        std::for_each(vertPositions.begin(), vertPositions.end(), [](Point3D& v) { v *= 20.0f; });
    }

    vertPositions.resize(icoSphere->vertices.size());
    std::transform(icoSphere->vertices.begin(), icoSphere->vertices.end(), vertPositions.begin(), [](const oGFX::Vertex& v) { return v.pos; });
    oGFX::BV::RitterSphere(icoSphere->s, vertPositions);
    oGFX::BV::BoundingAABB(icoSphere->aabb, vertPositions);

    std::unique_ptr<Model> box{ renderer->LoadMeshFromBuffers(defaultCubeMesh.m_VertexBuffer, defaultCubeMesh.m_IndexBuffer, nullptr) };
    vertPositions.resize(box->vertices.size());
    std::transform(box->vertices.begin(), box->vertices.end(), vertPositions.begin(), [](const oGFX::Vertex& v) { return v.pos; });
    oGFX::BV::LarsonSphere(ms, vertPositions, oGFX::BV::EPOS::_98);
    oGFX::BV::BoundingAABB(box->aabb, vertPositions);

    std::unique_ptr<Model> lucy{ renderer->LoadMeshFromFile("Models/lucy_princeton.obj") };
    if (lucy)
    {
        vertPositions.resize(lucy->vertices.size());
        std::transform(lucy->vertices.begin(), lucy->vertices.end(), vertPositions.begin(), [](const oGFX::Vertex& v) { return v.pos; });
        oGFX::BV::RitterSphere(lucy->s, vertPositions);
        oGFX::BV::BoundingAABB(lucy->aabb, vertPositions);
    }

    std::unique_ptr<Model> starWars{ renderer->LoadMeshFromFile("Models/starwars1.obj") };
    if (starWars)
    {
        vertPositions.resize(starWars->vertices.size());
        std::transform(starWars->vertices.begin(), starWars->vertices.end(), vertPositions.begin(), [](const oGFX::Vertex& v) { return v.pos; });
        oGFX::BV::RitterSphere(starWars->s, vertPositions);
        oGFX::BV::BoundingAABB(starWars->aabb, vertPositions);
    }
    
    std::unique_ptr<Model> fourSphere{ renderer->LoadMeshFromFile("Models/4Sphere.obj") };
    if (fourSphere)
    {
        vertPositions.resize(fourSphere->vertices.size());
        std::transform(fourSphere->vertices.begin(), fourSphere->vertices.end(), vertPositions.begin(), [](const oGFX::Vertex& v) { return v.pos; });
        oGFX::BV::RitterSphere(fourSphere->s, vertPositions);
        oGFX::BV::BoundingAABB(fourSphere->aabb, vertPositions);
    }
   
    //std::unique_ptr<Model> ball;
    //ball.reset(renderer->LoadMeshFromFile("Models/sphere.obj"));
    
    //int o{};
    //std::vector<glm::vec3> positions(bunny->vertices.size());
    //std::transform(bunny->vertices.begin(), bunny->vertices.end(), positions.begin(), [](const oGFX::Vertex& v) { return v.pos; });
    //for (auto& v : bunny->vertices)
    //{
    //    //std::cout << v.pos << " ";
    //    //if ((++o % 5) == 0) std::cout << std::endl;
    //    //++o;
    //}
    //
    //
    //std::cout << std::endl;
    //std::cout << "vertices : " << bunny->vertices.size() << std::endl;
    //
    //
    //uint32_t triangle = renderer->LoadMeshFromBuffers(quadVerts, quadIndices, nullptr);
    std::unique_ptr<Model> plane{ renderer->LoadMeshFromBuffers(defaultPlaneMesh.m_VertexBuffer, defaultPlaneMesh.m_IndexBuffer, nullptr) };
    //delete bunny;
    //
    //oGFX::BV::RitterSphere(ms, positions);
    //std::cout << "Ritter Sphere " << ms.center << " , r = " << ms.radius << std::endl;
    //oGFX::BV::LarsonSphere(ms, positions, oGFX::BV::EPOS::_6);
    //std::cout << "Larson_06 Sphere " << ms.center << " , r = " << ms.radius << std::endl;
    //oGFX::BV::LarsonSphere(ms, positions, oGFX::BV::EPOS::_14);
    //std::cout << "Larson_14 Sphere " << ms.center << " , r = " << ms.radius << std::endl;
    //oGFX::BV::LarsonSphere(ms, positions, oGFX::BV::EPOS::_26);
    //std::cout << "Larson_26 Sphere " << ms.center << " , r = " << ms.radius << std::endl;
    //oGFX::BV::LarsonSphere(ms, positions, oGFX::BV::EPOS::_98);
    //std::cout << "Larson_98 Sphere " << ms.center << " , r = " << ms.radius << std::endl;
    //std::cout << "Eigen Sphere " << ms.center << " , r = " << ms.radius << std::endl;
    //AABB ab;
    ////positions.resize(ball->vertices.size());
    ////std::transform(ball->vertices.begin(), ball->vertices.end(), positions.begin(), [](const oGFX::Vertex& v) { return v.pos; });
    ////oGFX::BV::BoundingAABB(ab, positions);
    ////std::cout << "AABB " << ab.center << " , Extents = " << ab.halfExt << std::endl;
    //
    //glm::mat4 id(1.0f);

    uint32_t whiteTexture = 0xFFFFFFFF; // ABGR
    uint32_t blackTexture = 0xFF000000; // ABGR
    uint32_t normalTexture = 0xFFFF8080; // ABGR
    uint32_t pinkTexture = 0xFFA040A0; // ABGR
    renderer->CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&whiteTexture));
    renderer->CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&blackTexture));
    renderer->CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&normalTexture));
    renderer->CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&pinkTexture));

    uint32_t diffuseTexture0 = renderer->CreateTexture("Textures/7/d.png");
    uint32_t diffuseTexture1 = renderer->CreateTexture("Textures/8/d.png");
    uint32_t diffuseTexture2 = renderer->CreateTexture("Textures/13/d.png");
    uint32_t diffuseTexture3 = renderer->CreateTexture("Textures/23/d.png");

    uint32_t roughnessTexture0 = renderer->CreateTexture("Textures/7/r.png");
    uint32_t roughnessTexture1 = renderer->CreateTexture("Textures/8/r.png");
    uint32_t roughnessTexture2 = renderer->CreateTexture("Textures/13/r.png");
    uint32_t roughnessTexture3 = renderer->CreateTexture("Textures/23/r.png");

    std::array<uint32_t, 4> diffuseBindlessTextureIndexes =
    {
        diffuseTexture0, diffuseTexture1, diffuseTexture2, diffuseTexture3
    };

    std::array<uint32_t, 4> roughnessBindlessTextureIndexes =
    {
        roughnessTexture0, roughnessTexture1, roughnessTexture2, roughnessTexture3
    };

    // Temporary solution to assign random numbers... Shamelessly stolen from Intel.
    auto FastRandomMagic = []() -> uint32_t
    {
        static uint32_t seed = 0xDEADBEEF;
        seed = (214013 * seed + 2531011);
        return (seed >> 16) & 0x7FFF;
    };

    int iter = 0;
    
    {
        VulkanRenderer::EntityDetails ed;
        ed.name = "Plane";
        ed.entityID = FastRandomMagic();
        ed.modelID = plane->gfxIndex;
        ed.position = { 0.0f,0.0f,0.0f };
        ed.scale = { 15.0f,1.0f,15.0f };
        renderer->entities.push_back(ed);
    }

    {
        VulkanRenderer::EntityDetails ed;
        ed.name = "IcoSphere";
        ed.entityID = FastRandomMagic();
        ed.modelID = icoSphere->gfxIndex;
        ed.position = { -2.0f,2.0f,-2.0f };
        ed.scale = { 1.0f,1.0f,1.0f };
        renderer->entities.push_back(ed);
    }
   
    {
        VulkanRenderer::EntityDetails ed;
        ed.modelID = box->gfxIndex;
        ed.name = "Box";
        ed.entityID = FastRandomMagic();
        ed.position = { 2.0f,3.0f,2.0f };
        ed.scale = { 2.0f,3.0f,1.0f };
        ed.rot = { 45.0f };
        renderer->entities.push_back(ed);
    }

    // Create 8 more surrounding planes
    {
        for (int i = 0; i < 8; ++i)
        {
            constexpr float offset = 16.0f;
            static std::array<glm::vec3, 8> positions =
            {
                glm::vec3{  offset, 0.0f,   0.0f },
                glm::vec3{ -offset, 0.0f,   0.0f },
                glm::vec3{    0.0f, 0.0f,  offset },
                glm::vec3{    0.0f, 0.0f, -offset },
                glm::vec3{  offset, 0.0f,  offset },
                glm::vec3{ -offset, 0.0f,  offset },
                glm::vec3{  offset, 0.0f, -offset },
                glm::vec3{ -offset, 0.0f, -offset },
            };

            VulkanRenderer::EntityDetails ed;
            ed.name = "Plane_" + std::to_string(i);
            ed.entityID = FastRandomMagic();
            ed.modelID = plane->gfxIndex;
            ed.position = positions[i];
            ed.scale = { 15.0f,1.0f,15.0f };
            ed.bindlessGlobalTextureIndex_Albedo = diffuseBindlessTextureIndexes[i / 2];
            ed.bindlessGlobalTextureIndex_Roughness = roughnessBindlessTextureIndexes[i / 2];
            renderer->entities.push_back(ed);
        }
    }

    std::unique_ptr<Model> diona{ renderer->LoadMeshFromFile("Models/diona.fbx") };
    if (diona)
    {
        VulkanRenderer::EntityDetails ed;
        ed.modelID = diona->gfxIndex;
        ed.name = "diona";
        ed.entityID = FastRandomMagic();
        ed.position = { 0.0f,0.0f,0.0f };
        ed.scale = { 1.0f,1.0f,1.0f };
        renderer->entities.push_back(ed);
    }
    
    std::unique_ptr<Model> qiqi{ renderer->LoadMeshFromFile("Models/qiqi.fbx") };
    if (qiqi)
    {
        VulkanRenderer::EntityDetails ed;
        ed.modelID = qiqi->gfxIndex;
        ed.name = "qiqi";
        ed.entityID = FastRandomMagic();
        ed.position = { 1.0f,0.0f,0.0f };
        ed.scale = { 1.0f,1.0f,1.0f };
        renderer->entities.push_back(ed);
    }

    if (bunny)
    {
        VulkanRenderer::EntityDetails ed;
        ed.modelID = bunny->gfxIndex;
        ed.name = "Bunny";
        ed.entityID = FastRandomMagic();
        ed.position = { -3.0f,2.0f,-3.0f };
        ed.scale = { 5.0f,5.0f,5.0f };
        renderer->entities.push_back(ed);
    }

    if (lucy)
    {
        VulkanRenderer::EntityDetails ed;
        ed.modelID = lucy->gfxIndex;
        ed.name = "lucy";
        ed.entityID = FastRandomMagic();
        ed.position = { -1.0f,1.0f,2.0f };
        ed.scale = { 0.002f,0.002f,0.002f };
        ed.rotVec = { 1.0f,1.0f,0.0f };
        ed.rot = { 0.0f };
        renderer->entities.push_back(ed);
    }

    if (starWars)
    {
        VulkanRenderer::EntityDetails ed;
        ed.modelID = starWars->gfxIndex;
        ed.name = "Starwars1";
        ed.entityID = FastRandomMagic();
        ed.position = { 3.0f,-2.0f,-5.0f };
        ed.scale = { 0.001f,0.001f,0.001f };
        renderer->entities.push_back(ed);
    }

    if (fourSphere)
    {
        VulkanRenderer::EntityDetails ed;
        ed.modelID = fourSphere->gfxIndex;
        ed.name = "fourSphere";
        ed.entityID = FastRandomMagic();
        ed.scale = { 0.001f,0.001f,0.001f };
        ed.position = { 1.0f, 2.0f,5.0f };
        renderer->entities.push_back(ed);
    }

    std::vector<Point3D> sceneVertices;
    std::vector<uint32_t> sceneIndices;
    for (size_t i = 0; i < renderer->entities.size(); i++)
    {
        auto& ent = renderer->entities[i];
        auto& model = renderer->models[renderer->entities[i].modelID];
        auto& meshInfo = *model.cpuModel;
        
        auto chachedPos = sceneVertices.size();

        glm::mat4 xform(1.0f);
        xform = glm::translate(xform, ent.position);
        xform = glm::rotate(xform,glm::radians(ent.rot), ent.rotVec);
        xform = glm::scale(xform, ent.scale);
        std::transform(meshInfo.vertices.begin(), meshInfo.vertices.end(), std::back_inserter(sceneVertices), [&](const oGFX::Vertex& v)
            {                
                return xform * vec4{ v.pos ,1.0f};
            });
        for (auto ind: meshInfo.indices)
        {
            sceneIndices.push_back((uint32_t)(chachedPos + ind));
        }
    }

    std::cout << "Total triangles : " << sceneIndices.size() / 3 << std::endl;
    
    GraphicsWorld Gworld;
    std::vector<int32_t> gWorldIds;

    for (auto& e: renderer->entities)
    {
        AABB ab;
        auto& model = renderer->models[e.modelID];
        
        UpdateBV(renderer->models[e.modelID].cpuModel, e);

        glm::mat4 xform(1.0f);
        xform = glm::translate(xform, e.position);
        xform = glm::rotate(xform,glm::radians(e.rot), e.rotVec);
        xform = glm::scale(xform, e.scale);
        auto id = Gworld.CreateObjectInstance(ObjectInstance{
            e.name,
            e.position,
            e.scale,
            e.rot,
            e.rotVec,
            e.bindlessGlobalTextureIndex_Albedo,
            e.bindlessGlobalTextureIndex_Normal,
            e.bindlessGlobalTextureIndex_Roughness,
            e.bindlessGlobalTextureIndex_Metallic,
            xform,
            e. modelID,
            e. entityID 
            }
        );
        gWorldIds.push_back(id);
    }

    // for now..
    // renderer->SetWorld(Gworld);

    //create a hundred random textures because why not
    std::default_random_engine rndEngine(123456);
    std::uniform_int_distribution<uint32_t> uniformDist(0xFF000000, 0xFFFFFFFF);
    std::vector<oGFX::InstanceData> instanceData;
    constexpr size_t numTex = 5;
    constexpr size_t dims = 2;
    std::vector<uint32_t> bitmap(dims* dims);
    for (size_t i = 0; i < numTex; i++)
    {
        for (size_t x = 0; x < bitmap.size(); x++)
        {
            uint32_t colour = uniformDist(rndEngine); // ABGR
            bitmap[x] = colour;
        }
        renderer->CreateTexture(dims, dims, reinterpret_cast<unsigned char*>(bitmap.data()));
    }

    auto lastTime = std::chrono::high_resolution_clock::now();

    renderer->camera.type = Camera::CameraType::lookat;
    renderer->camera.target = glm::vec3(0.01f, 0.0f, 0.0f);
    renderer->camera.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
    renderer->camera.SetRotationSpeed(0.5f);
    renderer->camera.SetPosition(glm::vec3(0.1f, 10.0f, 10.5f));
    renderer->camera.movementSpeed = 5.0f;
    renderer->camera.SetPerspective(60.0f, (float)mainWindow.m_width / (float)mainWindow.m_height, 0.1f, 10000.0f);
    renderer->camera.Rotate(glm::vec3(1 * renderer->camera.rotationSpeed, 1 * renderer->camera.rotationSpeed, 0.0f));
    renderer->camera.type = Camera::CameraType::firstperson;

    static bool freezeLight = false;

    //renderer->UpdateDebugBuffers();

    int currSphereType{ 0 };
    bool geomChanged = false;
    bool warningMsg = false;

    // handling winOS messages
    // This will handle inputs and pass it to our input callback
    while( mainWindow.windowShouldClose == false )  // infinite loop
    {
        PROFILE_FRAME("MainThread");
        
        //reset keys
        Input::Begin();
        while(Window::PollEvents());

        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>( now - lastTime).count();
        lastTime = now;

        renderer->camera.keys.left =     Input::GetKeyHeld(KEY_A)? true : false;
        renderer->camera.keys.right =    Input::GetKeyHeld(KEY_D)? true : false;
        renderer->camera.keys.down =     Input::GetKeyHeld(KEY_S)? true : false;
        renderer->camera.keys.up =       Input::GetKeyHeld(KEY_W)? true : false;
        renderer->camera.Update(deltaTime);

        if (mainWindow.m_width != 0 && mainWindow.m_height != 0)
        {
            renderer->camera.SetPerspective(60.0f, (float)mainWindow.m_width / (float)mainWindow.m_height, 0.1f, 10000.0f);
        }

        auto mousedel = Input::GetMouseDelta();
        float wheelDelta = Input::GetMouseWheel();
        if (Input::GetMouseHeld(MOUSE_RIGHT)) 
        {
            renderer->camera.Rotate(glm::vec3(-mousedel.y * renderer->camera.rotationSpeed, mousedel.x * renderer->camera.rotationSpeed, 0.0f));
        }
        if (renderer->camera.type == Camera::CameraType::lookat)
        {
            renderer->camera.ChangeDistance(wheelDelta * -0.001f);
        }
        
        if (Input::GetKeyTriggered(KEY_SPACE))
        {
            freezeLight = !freezeLight;
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //std::cout<<  renderer->camera.position << '\n';

        if (geomChanged)
        {
            // update bv
            for (auto& e: renderer->entities)
            {
                AABB ab;
                auto& model = renderer->models[e.modelID];
                ab.center = e.position;
                ab.halfExt = e.scale * 0.5f;
                //UpdateBV(renderer->models[e.modelID].cpuModel, e, currSphereType);
                
            }

            geomChanged = false;
        }

        if (renderer->gpuTransformBuffer.MustUpdate())
        {
            auto dbi = renderer->gpuTransformBuffer.GetDescriptorBufferInfo();
            //VkWriteDescriptorSet write = oGFX::vk::inits::writeDescriptorSet(renderer->g0_descriptors, 
            //    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
            //    3,
            //    &dbi);
            //vkUpdateDescriptorSets(renderer->m_device.logicalDevice, 1, &write, 0, 0);
            DescriptorBuilder::Begin(&DescLayoutCache, &descAllocs[0])
                .BindBuffer(3, gpuTransformBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .BindBuffer(4, boneMatrixBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .BindBuffer(5, objectInformationBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .Build(descriptorSet_gpuscene,SetLayoutDB::gpuscene);
            renderer->gpuTransformBuffer.Updated();
        }

        if (renderer->PrepareFrame() == true)
        {
            PROFILE_SCOPED("renderer->PrepareFrame() == true");

            renderer->timer += deltaTime;
            if (freezeLight == false)
            {
                // TODO: turn into proper entities
                renderer->UpdateLights(deltaTime);
            }

            // Upload CPU light data to GPU. Ideally this should only contain lights that intersects the camera frustum.
            renderer->UploadLights();

            // Render the frame
            renderer->RenderFrame();

            // Create a dockspace over the mainviewport so that we can dock stuff
            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), 
                ImGuiDockNodeFlags_PassthruCentralNode // make the dockspace transparent
                | ImGuiDockNodeFlags_NoDockingInCentralNode // dont allow docking in the central area
            );
            
            //ImGui::ShowDemoWindow();
            
            // ImGuizmo
            if (gizmoHijack)
            {
                ImGuizmo::BeginFrame();
                ImGuizmo::Enable(true);
                ImGuizmo::AllowAxisFlip(false);
                ImGuizmo::SetGizmoSizeClipSpace(0.1f);

                static bool prevFrameUsingGizmo = false;
                static bool currFrameUsingGizmo = false;

                // Save the state of the gizmos in the previous frame.
                prevFrameUsingGizmo = currFrameUsingGizmo;
                currFrameUsingGizmo = ImGuizmo::IsUsing();
                const bool firstUseGizmo = currFrameUsingGizmo && !prevFrameUsingGizmo;
                const bool justReleaseGizmo = prevFrameUsingGizmo && !currFrameUsingGizmo;

                ImGuiIO& io = ImGui::GetIO();
                ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
                const auto& viewMatrix = renderer->camera.matrices.view;
                const auto& projMatrix = renderer->camera.matrices.perspective;
                
                static glm::mat4x4 localToWorld{ 1.0f };
                float* matrixPtr = glm::value_ptr(localToWorld);
                
                //if (gizmoHijack) // Fix me!
                {
                    glm::vec3 position = { gizmoHijack[0], gizmoHijack[1], gizmoHijack[2] };
                    localToWorld = glm::translate(glm::mat4{ 1.0f }, position);
                    matrixPtr = glm::value_ptr(localToWorld);
                }

                const ImGuizmo::OPERATION gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
                const ImGuizmo::MODE gizmoMode = ImGuizmo::MODE::WORLD;
                const bool gizmoMoved = ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projMatrix), gizmoOperation, gizmoMode, matrixPtr);

                if (firstUseGizmo)
                {
                    // TODO: What can you do here?
                    // - Save the state of the transform? For undo/redo feature?
                }
                else if (justReleaseGizmo)
                {
                    // TODO: What can you do here?
                    // - Save the state of the transform? For undo/redo feature?
                }

                if (currFrameUsingGizmo)
                {
                    if (gizmoMoved)
                    {
                        // TODO: What can you do here?
                        // - Update transform because the gizmo have moved.

                        glm::vec3 position, euler_deg, scale; // World Space.
                        ImGuizmo::DecomposeMatrixToComponents(matrixPtr, glm::value_ptr(position), glm::value_ptr(euler_deg), glm::value_ptr(scale));
                        glm::quat q = glm::quat(glm::radians(euler_deg));

                        // Hacky and unsafe...
                        gizmoHijack[0] = position[0];
                        gizmoHijack[1] = position[1];
                        gizmoHijack[2] = position[2];
                    }
                }
            }
            bool renderGraphicsWorld = renderer->currWorld;
            // Display ImGui Window
            {
                PROFILE_SCOPED("ImGuiSceneHelper");
                if (ImGui::Begin("Scene Helper"))
                {
                    if (ImGui::Checkbox("RenderGraphicsWorld", &renderGraphicsWorld))
                    {
                        if (renderGraphicsWorld)
                        {
                            renderer->SetWorld(&Gworld);
                        }
                        else
                        {
                            renderer->SetWorld(nullptr);
                        }
                    }
                    if (ImGui::BeginTabBar("SceneHelperTabBar"))
                    {
                        if (ImGui::BeginTabItem("Entity"))
                        {
                            if (ImGui::SmallButton("Create Cube"))
                            {
                                VulkanRenderer::EntityDetails entity;
                                entity.position = { 2.0f,2.0f,2.0f };
                                entity.scale = { 1.0f,1.0f,1.0f };
                                entity.modelID = box->gfxIndex;
                                entity.entityID = FastRandomMagic();
                                renderer->entities.emplace_back(entity);
                            }

                            int addRandomEntityCount = 0;
                            ImGui::Text("Add Random Entity: ");
                            ImGui::SameLine();
                            if (ImGui::SmallButton("+10")) { addRandomEntityCount = 10; }
                            ImGui::SameLine();
                            if (ImGui::SmallButton("+50")) { addRandomEntityCount = 50; }
                            ImGui::SameLine();
                            if (ImGui::SmallButton("+100")) { addRandomEntityCount = 100; }
                            ImGui::SameLine();
                            if (ImGui::SmallButton("+250")) { addRandomEntityCount = 250; }

                            if (addRandomEntityCount)
                            {
                                for (int i = 0; i < addRandomEntityCount; ++i)
                                {
                                    const glm::vec3 pos = glm::sphericalRand(20.0f);

                                    VulkanRenderer::EntityDetails entity;
                                    entity.position = { pos.x, glm::abs(pos.y), pos.z };
                                    entity.scale = { 1.0f,1.0f,1.0f };
                                    entity.modelID = box->gfxIndex;
                                    entity.entityID = FastRandomMagic();
                                    renderer->entities.emplace_back(entity);
                                }
                            }

                            ImGui::Text("Total Entities: %u", renderer->entities.size());

                            if (ImGui::TreeNode("Entity List"))
                            {
                                for (auto& entity : renderer->entities)
                                {
                                    ImGui::PushID(entity.entityID);

                                    ImGui::BulletText("[ID:%u] ", entity.entityID);
                                    ImGui::SameLine();
                                    ImGui::Text(entity.name.c_str());
                                    geomChanged |= ImGui::DragFloat3("Position", glm::value_ptr(entity.position), 0.01f);
                                    {
                                        if (ImGui::BeginPopupContextItem("Gizmo hijacker"))
                                        {
                                            if (ImGui::Selectable("Set ptr Gizmo"))
                                            {
                                                // Shamelessly point to this property (very unsafe, but quick to test shit and speed up iteration time)
                                                gizmoHijack = glm::value_ptr(entity.position);
                                            }
                                            ImGui::EndPopup();
                                        }
                                    }

                                    geomChanged |= ImGui::DragFloat3("Scale", glm::value_ptr(entity.scale), 0.01f);
                                    geomChanged |= ImGui::DragFloat3("Rotation Axis", glm::value_ptr(entity.rotVec));
                                    geomChanged |= ImGui::DragFloat("Theta", &entity.rot);
                                    // TODO: We should be using quaternions.........

                                    ImGui::PopID();
                                }
                                
                                ImGui::TreePop();
                            }//ImGui::TreeNode
                            
                            ImGui::EndTabItem();
                        }//ImGui::BeginTabItem
                        
                        if (ImGui::BeginTabItem("Light"))
                        {
                            ImGui::BeginDisabled(); // TODO remove once implemented
                            if (ImGui::SmallButton("Create PointLight")) {} // TODO Implement me!
                            ImGui::EndDisabled(); // TODO remove once implemented
                            
                            static bool debugDrawPosition = false;
                            ImGui::Checkbox("Freeze Lights", &freezeLight);
                            ImGui::Checkbox("Debug Draw Position", &debugDrawPosition);
                            ImGui::DragFloat3("ViewPos", glm::value_ptr(renderer->lightUBO.viewPos));
                            ImGui::Separator();
                            for (int i = 0; i < 6; ++i)
                            {
                                ImGui::PushID(i);
                                auto& light = renderer->m_HardcodedOmniLights[i];
                                ImGui::DragFloat3("Position", glm::value_ptr(light.position), 0.01f);
                                {
                                    if (ImGui::BeginPopupContextItem("Gizmo hijacker"))
                                    {
                                        if (ImGui::Selectable("Set ptr Gizmo"))
                                        {
                                            // Shamelessly point to this property (very unsafe, but quick to test shit and speed up iteration time)
                                            gizmoHijack = glm::value_ptr(light.position);
                                        }
                                        ImGui::EndPopup();
                                    }
                                }

                                ImGui::DragFloat3("Color", glm::value_ptr(light.color));
                                ImGui::DragFloat("Radius", &light.radius.x);
                                ImGui::PopID();
                            }

                            // Shamelessly hijack ImGui for debugging tools
                            if (debugDrawPosition)
                            {
                                if (ImDrawList* bgDrawList = ImGui::GetBackgroundDrawList())
                                {
                                    auto WorldToScreen = [&](const glm::vec3& worldPosition) -> ImVec2
                                    {
                                        const int screenWidth = (int)windowSize.x;
                                        const int screenHeight = (int)windowSize.y;
                                        const glm::mat4& viewMatrix = renderer->camera.matrices.view;
                                        const glm::mat4& projectionMatrix = renderer->camera.matrices.perspective;
                                        // World Space to NDC Space
                                        glm::vec4 ndcPosition = projectionMatrix * viewMatrix * glm::vec4{ worldPosition, 1.0f };
                                        // Perspective Division
                                        ndcPosition /= ndcPosition.w;
                                        // NDC Space to Viewport Space
                                        const float winX = glm::round(((ndcPosition.x + 1.0f) / 2.0f) * (float)screenWidth);
                                        const float winY = glm::round(((1.0f - ndcPosition.y) / 2.0f) * (float)screenHeight);
                                        return ImVec2{ winX, winY };
                                    };

                                    for (int i = 0; i < 6; ++i)
                                    {
                                        auto& light = renderer->m_HardcodedOmniLights[i];
                                        auto& pos = light.position;
                                        const auto screenPosition = WorldToScreen(pos);
                                        constexpr float circleSize = 10.0f;
                                        bgDrawList->AddCircle(ImVec2(screenPosition.x - 0.5f * circleSize, screenPosition.y - 0.5f * circleSize), circleSize, IM_COL32(light.color.r * 0xFF, light.color.g * 0xFF, light.color.b * 0xFF, 0xFF), 0, 2.0f);
                                    }
                                }
                            }

                            ImGui::EndTabItem();
                        }//ImGui::BeginTabItem

                        if (ImGui::BeginTabItem("Settings"))
                        {
                            // TODO?
                            ImGui::EndTabItem();
                        }//ImGui::BeginTabItem

                        if (ImGui::BeginTabItem("Settings"))
                        {
                            // TODO?
                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }//ImGui::BeginTabBar
                }//ImGui::Begin
                
                ImGui::End();
            }

            renderer->DebugGUIcalls();
            //
            ImGui::Render();  // Rendering UI
            renderer->DrawGUI();

            renderer->Present();
        }

        //finish for all windows
        ImGui::EndFrame();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {            
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

    }   

    renderer->DestroyImGUI();
    ImGui::DestroyContext(ImGui::GetCurrentContext());
    delete renderer;
    

    std::cout << "Exiting application..."<< std::endl;

}
