/************************************************************************************//*!
\file           EventUtils.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552 | code contribution (100%)
\par            email: l.guanhui\@digipen.edu
\date           November 12, 2021
\brief          Utility header for events.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
namespace oo
{
	namespace event
	{
		struct Event
		{
		protected:
			virtual ~Event() {};
		};
	}
	
}
