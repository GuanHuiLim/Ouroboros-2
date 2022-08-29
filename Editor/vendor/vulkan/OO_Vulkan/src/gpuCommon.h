#pragma once
#include <functional>

namespace oGFX
{

struct SetupInfo
{
	bool debug = false;
	bool renderDoc = false;
	bool useOwnImgui = false;
	std::function<void()> SurfaceFunctionPointer{ nullptr };
	std::vector<const char*> extensions;
};

}