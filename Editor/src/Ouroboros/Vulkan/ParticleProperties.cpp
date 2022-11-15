/************************************************************************************//*!
\file          ParticleProperties.cpp
\project       Ouroboros
\author        Jamie Kong, j.kong , 390004720 | code contribution (100%)
\par           email: j.kong\@digipen.edu
\date          December 1, 2021
\brief         File contains particle properties data getters and setters.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "ParticleProperties.h"
#include "Ouroboros/Core/Assert.h"
#include "OO_Vulkan/src/GfxTypes.h"

namespace oo
{
	ParticleProperties::ParticleProperties() : 
		startSpeed{ 0.0f }, 
		p_colours{ {0.0f,1.0f} },
		colours{ {oGFX::Color(1.0f,1.0f,1.0f,1.0f),oGFX::Color(1.0f,1.0f,1.0f,1.0f)} },
		p_speeds{ {0.0f,1.0f} },
		speeds{ {1.0f,1.0f} },
		p_rotations{ {0.0f,1.0f} },
		rotations{ {0.0f,0.0f} },
		p_directions{ {0.0f,1.0f} },
		directions{ {glm::vec3(0.0f,1.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f)} },
		p_sizes{ {0.0f,1.0f} },
		sizes{ {glm::vec3(1.0f),glm::vec3(1.0f)} }
	{
	}

	bool ParticleProperties::GetLocalSpace() const
	{
		return localSpace;
	}

	void ParticleProperties::SetLocalSpace(bool b)
	{
		localSpace = b;
	}

} // end namespace oo
