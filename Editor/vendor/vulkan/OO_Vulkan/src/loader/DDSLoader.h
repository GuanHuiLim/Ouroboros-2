/************************************************************************************//*!
\file           DDSLoader.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares DDS loader function

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <string>

namespace oGFX { struct FileImageData; };

namespace oGFX{

		void LoadDDS(const std::string& filename, oGFX::FileImageData& data);

 }// end namespace oGFX

