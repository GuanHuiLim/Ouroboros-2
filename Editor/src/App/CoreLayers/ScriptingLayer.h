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

#include "Ouroboros/Core/Layer.h"
#include "Scripting/Scripting.h"

// FOR TESTING
#include "Ouroboros/Core/Input.h"
#include <filesystem>

namespace oo
{
    class ScriptingLayer final : public oo::Layer
    {
    public:

        ScriptingLayer()
        {
        }

        ~ScriptingLayer()
        {
            ScriptEngine::Unload();
        }

        void OnAttach() override final
        {

        }

        void OnDetach() override final
        {

        }

        void OnUpdate() override final
        {
            if (input::IsKeyHeld(input::KeyCode::LSHIFT) && input::IsKeyPressed(input::KeyCode::TAB))
            {
                // TESTING
                std::filesystem::create_directory("TestProject");
                std::filesystem::create_directory("TestProject/Scripts");
                CreateProject("TestProject/", "Default");
            }
            if (input::IsKeyHeld(input::KeyCode::LSHIFT) && input::IsKeyPressed(input::KeyCode::ENTER))
            {
                ScriptDatabase::DeleteAll();
                try
                {
                    ScriptEngine::Compile("TestProject/Scripting.csproj", "TestProject/bin/Debug/warnings.log", "TestProject/bin/Debug/errors.log");
                }
                catch (std::exception const& e)
                {
                    LOG_ERROR(e.what());
                    return;
                }
                try
                {
                    ScriptEngine::Load("TestProject/bin/Debug/Scripting.dll");
                }
                catch (std::exception const& e)
                {
                    LOG_ERROR(e.what());
                    return;
                }
                std::vector<MonoClass*> classList = ScriptEngine::GetClassesByBaseClass("Scripting", ScriptEngine::GetClass("ScriptCore", "Ouroboros", "MonoBehaviour"));
                ScriptDatabase::Initialize(classList);
                LOG_TRACE("COMPILED AND LOADED");
                // MonoObject* obj = ScriptEngine::CreateObject(ScriptEngine::GetClass("Scripting", "", "TestClass"));
                MonoObject* obj = ScriptDatabase::Instantiate(1, "", "TestClass");
                LOG_TRACE("OBJECT CREATED");
                try
                {
                    ScriptEngine::InvokeFunction(obj, "Awake");
                }
                catch(std::exception const& e)
                {
                    LOG_ERROR(e.what());
                }
                LOG_TRACE("DONE SUCCESSFULLY");
            }
            ScriptDatabase::ForAll([](MonoObject* obj)
            {
                try
                {
                    ScriptEngine::InvokeFunction(obj, "Update");
                }
                catch (std::exception const& e)
                {
                    LOG_ERROR(e.what());
                }
            });
        }

    private:
        void CreateProject(std::string const& projectPath, std::string const& projectName)
        {
            // copy core scripting dll to project folder
            std::filesystem::create_directory(projectPath + "Library/");
            std::filesystem::copy_file("dlls/ScriptCore.dll", projectPath + "Library/ScriptCore.dll", std::filesystem::copy_options::skip_existing);

            // generate VS C# project
            std::ofstream vsProj(projectPath + "Scripting.csproj");
            if (!vsProj)
                return;
            vsProj << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
            vsProj << "<Project ToolsVersion=\"15.0\" DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << std::endl;
            vsProj << "  <Import Project=\"$(MSBuildExtensionsPath)\\$(MSBuildToolsVersion)\\Microsoft.Common.props\" Condition=\"Exists('$(MSBuildExtensionsPath)\\$(MSBuildToolsVersion)\\Microsoft.Common.props')\" />" << std::endl;
            vsProj << "  <PropertyGroup>" << std::endl;
            vsProj << "    <Configuration Condition=\" '$(Configuration)' == '' \">Debug OpenGL</Configuration>" << std::endl;
            vsProj << "    <Platform Condition=\" '$(Platform)' == '' \">x64</Platform>" << std::endl;
            vsProj << "    <ProjectGuid>{}</ProjectGuid>" << std::endl;
            vsProj << "    <OutputType>Library</OutputType>" << std::endl;
            vsProj << "    <AppDesignerFolder>Properties</AppDesignerFolder>" << std::endl;
            vsProj << "    <RootNamespace>Scripting</RootNamespace>" << std::endl;
            vsProj << "    <AssemblyName>Scripting</AssemblyName>" << std::endl;
            vsProj << "    <TargetFrameworkVersion>v4.7.2</TargetFrameworkVersion>" << std::endl;
            vsProj << "    <FileAlignment>512</FileAlignment>" << std::endl;
            vsProj << "    <AutoGenerateBindingRedirects>true</AutoGenerateBindingRedirects>" << std::endl;
            vsProj << "  </PropertyGroup>" << std::endl;
            vsProj << "    <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Debug OpenGL|x64' \">" << std::endl;
            vsProj << "    <PlatformTarget>x64</PlatformTarget>" << std::endl;
            vsProj << "    <DebugSymbols>true</DebugSymbols>" << std::endl;
            vsProj << "    <DebugType>pdbonly</DebugType>" << std::endl;
            vsProj << "    <Optimize>false</Optimize>" << std::endl;
            vsProj << "    <OutputPath>bin\\Debug\\</OutputPath>" << std::endl;
            vsProj << "    <DefineConstants></DefineConstants>" << std::endl;
            vsProj << "    <ErrorReport>prompt</ErrorReport>" << std::endl;
            vsProj << "    <WarningLevel>4</WarningLevel>" << std::endl;
            vsProj << "  </PropertyGroup>" << std::endl;
            vsProj << "  <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Production OpenGL|x64' \">" << std::endl;
            vsProj << "    <DebugType>pdbonly</DebugType>" << std::endl;
            vsProj << "    <Optimize>true</Optimize>" << std::endl;
            vsProj << "    <OutputPath>bin\\Production\\</OutputPath>" << std::endl;
            vsProj << "    <DefineConstants>TRACE</DefineConstants>" << std::endl;
            vsProj << "    <ErrorReport>prompt</ErrorReport>" << std::endl;
            vsProj << "    <WarningLevel>4</WarningLevel>" << std::endl;
            vsProj << "  </PropertyGroup>" << std::endl;
            vsProj << "  <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Release OpenGL|x64' \">" << std::endl;
            vsProj << "    <DebugType>pdbonly</DebugType>" << std::endl;
            vsProj << "    <Optimize>true</Optimize>" << std::endl;
            vsProj << "    <OutputPath>bin\\Release\\</OutputPath>" << std::endl;
            vsProj << "    <DefineConstants>TRACE</DefineConstants>" << std::endl;
            vsProj << "    <ErrorReport>prompt</ErrorReport>" << std::endl;
            vsProj << "    <WarningLevel>4</WarningLevel>" << std::endl;
            vsProj << "  </PropertyGroup>" << std::endl;
            vsProj << "  <ItemGroup>" << std::endl;
            vsProj << "    <Reference Include=\"System\" />" << std::endl;
            vsProj << "    <Reference Include=\"System.Numerics\" />" << std::endl;
            vsProj << "    <Reference Include=\"Library\\ScriptCore.dll\" />" << std::endl;
            vsProj << "  </ItemGroup>" << std::endl;
            vsProj << "  <ItemGroup>" << std::endl;
            vsProj << "    <Folder Include=\"Scripts\" />" << std::endl;
            vsProj << "    <Compile Include=\"Scripts\\**\\*.cs\" />" << std::endl;
            vsProj << "  </ItemGroup>" << std::endl;
            vsProj << "  <Import Project=\"$(MSBuildToolsPath)\\Microsoft.CSharp.targets\" />" << std::endl;
            vsProj << "</Project>" << std::endl;
            vsProj.close();

            // generate VS solution
            static std::string projectTypeID = "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC"; // DO NOT CHANGE, fixed type id that indicates the project is C#
            std::ofstream vsSln(projectPath + projectName + ".sln");
            if (!vsSln)
                return;
            vsSln << "Microsoft Visual Studio Solution File, Format Version 12.00" << std::endl;
            vsSln << "# Visual Studio Version 16" << std::endl;
            vsSln << "VisualStudioVersion = 16.0.30907.101" << std::endl;
            vsSln << "MinimumVisualStudioVersion = 10.0.40219.1" << std::endl;
            vsSln << "Project(\"{" << projectTypeID << "}\") = \"Scripting\", \"Scripting.csproj\", \"{}\"" << std::endl;
            vsSln << "EndProject" << std::endl;
            vsSln << "Global" << std::endl;
            vsSln << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution" << std::endl;
            vsSln << "\t\tDebug|OpenGL = Debug|OpenGL" << std::endl;
            vsSln << "\t\tProduction|OpenGL = Production|OpenGL" << std::endl;
            vsSln << "\t\tRelease|OpenGL = Release|OpenGL" << std::endl;
            vsSln << "\tEndGlobalSection" << std::endl;
            vsSln << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution" << std::endl;
            vsSln << "\t{}.Debug|OpenGL.ActiveCfg = Debug OpenGL|x64" << std::endl;
            vsSln << "\t{}.Debug|OpenGL.Build.0 = Debug OpenGL|x64" << std::endl;
            vsSln << "\t{}.Production|OpenGL.ActiveCfg = Production OpenGL|x64" << std::endl;
            vsSln << "\t{}.Production|OpenGL.Build.0 = Production OpenGL|x64" << std::endl;
            vsSln << "\t{}.Release|OpenGL.ActiveCfg = Release OpenGL|x64" << std::endl;
            vsSln << "\t{}.Release|OpenGL.Build.0 = Release OpenGL|x64" << std::endl;
            vsSln << "\tEndGlobalSection" << std::endl;
            vsSln << "\tGlobalSection(SolutionProperties) = preSolution" << std::endl;
            vsSln << "\tHideSolutionNode = FALSE" << std::endl;
            vsSln << "\tEndGlobalSection" << std::endl;
            vsSln << "\tGlobalSection(ExtensibilityGlobals) = postSolution" << std::endl;
            vsSln << "\tSolutionGuid = {}" << std::endl;
            vsSln << "\tEndGlobalSection" << std::endl;
            vsSln << "EndGlobal" << std::endl;
            vsSln.close();
        }
    };

}
