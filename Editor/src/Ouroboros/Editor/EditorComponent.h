#pragma once
#include <Ouroboros/Vulkan/Color.h>
#include <rttr/type>
namespace oo
{
class EditorComponent
{
public:
	EditorComponent();
	~EditorComponent();

	oo::Color m_color = { 0.5f,0.5f,0.5f,1.0f };
	bool m_header = true;

	RTTR_ENABLE();
private:

};

}