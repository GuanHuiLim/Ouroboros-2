/************************************************************************************//*!
\file          AssetBrowser.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Browse all assets and even interact with them. 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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
