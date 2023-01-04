#include "pch.h"
#include "EditorComponent.h"
#include <rttr/registration>
namespace oo
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
	registration::class_<EditorComponent>("Editor Component")
		.property("Color",&EditorComponent::m_color)
		.property("Header", &EditorComponent::m_header);
	}
EditorComponent::EditorComponent()
{
}

EditorComponent::~EditorComponent()
{
}

}
