///************************************************************************************//*!
//\file           GraphicsContext.h
//\project        Ouroboros
//\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
//\par            email: c.tecklee\@digipen.edu
//\date           Jun 17, 2022
//\brief          Describes a generic abstraction of a graphical context with no knowledge
//                of its backend implementation
//
//Copyright (C) 2022 DigiPen Institute of Technology.
//Reproduction or disclosure of this file or its contents
//without the prior written consent of DigiPen Institute of
//Technology is prohibited.
//*//*************************************************************************************/
//#pragma once
//
//namespace oo
//{
//    class GraphicsContext
//    {
//    public:
//        virtual ~GraphicsContext() = default;
//
//        virtual void Init() = 0;
//        virtual void OnUpdateBegin() = 0;
//        virtual void SwapBuffers() = 0;
//
//        virtual void InitImGui() = 0;
//        virtual void OnImGuiBegin() = 0;
//        virtual void OnImGuiEnd() = 0;
//        virtual void OnImGuiShutdown() = 0;
//
//        virtual void SetWindowResized() {};
//        virtual bool SetVSync(bool enable) = 0;
//    };
//}