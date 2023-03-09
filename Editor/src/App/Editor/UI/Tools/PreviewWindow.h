#pragma once

#include <glm/glm.hpp>

//fwd declaration
struct GetPreviewWindowSizeEvent;

class PreviewWindow
{
public:
	PreviewWindow();
	~PreviewWindow();
	void Show();
	void UpdateWhenNotShown();
private:
	void GetPreviewWindowSize(GetPreviewWindowSizeEvent* e);
	float m_viewportWidth = 0.f, m_viewportHeight = 0.f;
	glm::vec2 m_windowStartPosition;
};
