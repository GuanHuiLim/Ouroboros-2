/************************************************************************************//*!
\file          AnimatorController_Internal.h
\project       Editor
\author        
\par           
\date          September 22, 2022
\brief         File Contains some internal stuff.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

//#include <imgui/imgui_node_editor.h>
#include <imgui/imgui_node_editor_internal.h>

namespace ed = ax::NodeEditor;
struct ed::Detail::EditorContext;
namespace AnimControllerViewInternal {

	ed::Detail::EditorContext* GetNodeEditor();

}
//internal namespace for animation controller view logic
namespace ACI = AnimControllerViewInternal;