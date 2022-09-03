/************************************************************************************//*!
\file           EventUtils.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552 | code contribution (100%)
\par            email: l.guanhui\@digipen.edu
\date           November 12, 2022
\brief          Utility header for events.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
namespace oo
{
	struct Event
	{
	protected:
		virtual ~Event() {};
	};

	/*struct TestEvent : public Event
	{

	};

	class TestType {};*/
}
