#pragma once

//fwd declaration
struct GetPreviewWindowSizeEvent;

class PreviewWindow
{
public:
	PreviewWindow();
	~PreviewWindow();
	void Show();

private:
	void GetPreviewWindowSize(GetPreviewWindowSizeEvent* e);
	float m_viewportWidth = 0.f, m_viewportHeight = 0.f;
};
