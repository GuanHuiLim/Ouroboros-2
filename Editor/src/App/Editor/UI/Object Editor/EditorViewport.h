#pragma once
class EditorViewport
{
public:
	EditorViewport();
	~EditorViewport();
	void Show();
private:
	int m_gizmoOperation = 7;
};