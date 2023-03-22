/************************************************************************************//*!
\file          AnimatorController_Internal.cpp
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b | code contribution (100%)
\par           email: muhammadamirul.b\@digipen.edu
\date          September 22, 2022
\brief         File Contains the definition needed to create an Animator Controller View
               for the engine.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"
#include "AnimatorController_Internal.h"

namespace AnimControllerViewInternal {


    ed::Detail::EditorContext* GetNodeEditor()
    {
        return reinterpret_cast<ax::NodeEditor::Detail::EditorContext*>(ed::GetCurrentEditor());
    }


}