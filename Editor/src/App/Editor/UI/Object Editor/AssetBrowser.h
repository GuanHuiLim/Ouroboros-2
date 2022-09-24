#pragma once
#include <rttr/variant.h>

class AssetBrowser
{
public:
	static void AssetPickerUI(rttr::variant& data, bool& edited, int asset_type);
private:
	static void TextureUI(rttr::variant& data, bool& edited);
	static void FontUI(rttr::variant& data, bool& edited);
	static void AudioUI(rttr::variant& data, bool& edited);
	static void MeshUI(rttr::variant& data, bool& edited);
};
