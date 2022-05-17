/************************************************************************************//*!
\file           Application.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 05, 2021
\brief          Core Application Loop and functionality. 
                Will be inherited by Sandbox project.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

//forward declare main
int main(int argc, char** argv);

namespace oo
{
    /********************************************************************************//*!
     @brief     Defines what makes up a command line argument.
    *//*********************************************************************************/
    struct CommandLineArgs
    {
        int Count = 0;
        char** Args = nullptr;

        /****************************************************************************//*!
         @brief     subscript operator overload for convinience when wanting to 
                    access arguments directly
         
         @note      make sure you do not use an invalid index that is larger then the
                    possible number of arguments.

         @param     the index you want to retrieve the command line argument from.
        *//*****************************************************************************/
        const char* operator[](int index) const
        {
            //ENGINE_ASSERT(index < Count);
            return Args[index];
        }
    };

    /********************************************************************************//*!
     @brief     Application entails the core loop and the where and how each system is 
                being used within.
    *//*********************************************************************************/
    class Application
    {
    public:
        /*-----------------------------------------------------------------------------*/
        /* Constructors and Destructors                                                */
        /*-----------------------------------------------------------------------------*/
        Application(const std::string& name = "Engine App", CommandLineArgs args = CommandLineArgs{});
        virtual ~Application();

        /*-----------------------------------------------------------------------------*/
        /* Getters                                                                     */
        /*-----------------------------------------------------------------------------*/
        /****************************************************************************//*!
         @brief     Retrieve the static instance of the create application.
         
         @note      Function will break if an Application is not yet created and 
                    this function gets called.

         @return    returns the static instance of the created application.
                    crashes if s_instance is null due to dereferencing.
        *//*****************************************************************************/
        //static Application& Get() { ENGINE_ASSERT(s_instance);  return *s_instance; }
        /****************************************************************************//*!
         @brief     Retrieve window that the application holds.

         @note      continue to call GetNativeWindow() to retrieve the actual window.

         @return    returns a generic window reference
        *//*****************************************************************************/
        //Window& GetWindow() const { ENGINE_ASSERT(m_window); return *m_window; }
        /****************************************************************************//*!
         @brief     Retrieve the command line arguments passed to the application.

         @return    returns the command line arguments struct held by the application.
        *//*****************************************************************************/
        CommandLineArgs GetCommandLineArgs() const { return m_commandLineArgs; }
        
        /*-----------------------------------------------------------------------------*/
        /* Functions                                                                   */
        /*-----------------------------------------------------------------------------*/
        /****************************************************************************//*!
         @brief     Describes the applications way of closing down
        *//*****************************************************************************/
        void Close();
        ///****************************************************************************//*!
        // @brief     Describes what happens when any type of event gets called
        //*//*****************************************************************************/
        //void OnEvent(Event& e);
        
        // Windows short-hand
        //static bool WindowIsFocused() { return Get().GetWindow().IsFocused(); }

    private:
        /****************************************************************************//*!
         @brief     Describes the applications core run loop
        *//*****************************************************************************/
        void Run();

        //bool OnWindowClose(WindowCloseEvent& e);

    private:
        bool m_running;
        CommandLineArgs m_commandLineArgs;
        
        //Order matters!
        //Window* m_window;
        //std::unique_ptr<Window> m_window;
        //GraphicsContext* m_renderer;
        
        static Application* s_instance;
        friend int ::main(int argc, char** argv);
    };

    /****************************************************************************//*!
     @brief     Function prototype, implementation to be defined in client
    *//*****************************************************************************/
    Application* CreateApplication(CommandLineArgs commandLineArgs);


}
