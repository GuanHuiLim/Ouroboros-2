#pragma once
#include "Editor/UI/Tools/StyleEditor.h"
#include "Editor/UI/Tools/WarningMessage.h"
class Editor
{
public:
	Editor();
	~Editor();
	void Update();
	StyleEditor m_styleEditor;
	WarningMessage m_warningMessage;
private:
};

